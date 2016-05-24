/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */
 
// STL includes
#include <vector> 
#include <map>
 
// Boost includes
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/thread/mutex.hpp> 
#include <boost/thread/condition_variable.hpp>
 
// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>

// Application includes
#include <Application/StatusBar/StatusBar.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerUndoBufferItem.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/LayerFilterLock.h>
#include <Application/Filters/LayerFilterNotifier.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/UndoBuffer/Actions/ActionUndo.h>
 
namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class LayerFilterPrivate
//////////////////////////////////////////////////////////////////////////

class LayerFilterPrivate : public Core::ConnectionHandler
{
  // -- constructor --
public:
  LayerFilterPrivate() :
    done_( false ),
    abort_( false ),
    key_( Layer::GenerateFilterKey() ),
    id_count_( LayerManager::GetLayerIdCount() ),
    sandbox_( -1 )
  {
  }

public:
  // Keep track of errors
  std::string error_;
  
  // Message that is relayed in case there is no error
  std::string success_;
    
  // Keep track of which layers were locked for use (no processing).
  std::vector< LayerHandle > locked_for_use_layers_;

  // Keep track of which layers were locked for processing.
  std::vector< LayerHandle > locked_for_processing_layers_;

  // Keep track of which layers were locked for deletion.
  std::vector< LayerHandle > locked_for_deletion_layers_;
  
  // Keep track of which layers were created.
  std::vector< LayerHandle > created_layers_;
  
  // Keep track of which layers to create volume checkpoints for
  std::vector< LayerHandle > volume_check_point_layers_;

  // Keep track of which layers will be deleted
  std::vector< LayerHandle > deleted_layers_;

  // Keep track of whether the filter has finished
  bool done_;
  
  // Keep track of abort status
  bool abort_;
  
  // Keep track of stop status
  bool stop_;
  
  // Mutex protecting abort status
  boost::mutex mutex_;
  
  // Condition Variable signaling when filter is done
  boost::condition_variable filter_done_;

  // Key used for this filter
  Layer::filter_key_type key_;

  // ID counts for layer and group, used for undo mechanism
  LayerManager::id_count_type id_count_;
  
  // Provenance IDs
  typedef std::map< Layer*, ProvenanceID > provenance_map_type;
  provenance_map_type provenance_ids_;
  
  // ProvenanceStep, the provenance step record that is associated with this filter
  ProvenanceStepHandle provenance_step_;

  // Record ID of the provenance step.
  std::vector< ProvenanceStepID > prov_step_ids_;

  // A weak handle to the LayerUndoBufferItem. It's used to rollback any changes
  // that might have already happened when the filter is aborted.
  LayerUndoBufferItemWeakHandle undo_item_weak_handle_;

  // Pointer to the filter.
  LayerFilter* filter_;

  // Sandbox ID
  SandboxID sandbox_;

  // Filter done notifier
  LayerFilterNotifierHandle notifier_;
  
  // -- internal functions --
public:
  // FINALIZE:
  // Clean up all the filter components and release the locks on the layers and
  // delete layers if the filter did not finish.
  void finalize();

  // DELETE_LAYER:
  // Delete layer
  bool delete_layer( LayerHandle layer );

  // INTERNAL_ABORT:
  // Abort the filter.
  void internal_abort();

  // CREATE_UNDO_REDO_RECORD:
  // Create an undo record and add it to the undo stack
  void create_undo_redo_record( Core::ActionContextHandle context, Core::ActionHandle action );

  // CREATE_PROVENANCE_RECORD:
  // Create a provenance record and add it to the provenance database
  void create_provenance_record( Core::ActionContextHandle context, 
    Core::ActionHandle action, bool split_prov );
};

bool LayerFilterPrivate::delete_layer( LayerHandle layer )
{
  // Check whether the locked layer is still in the list of layers that this filter locked
  std::vector<LayerHandle>::iterator it;
  
  it = std::find( this->locked_for_use_layers_.begin(), 
    this->locked_for_use_layers_.end(), layer );

  if ( it != this->locked_for_use_layers_.end() )
  {
    // Take the layer out of the list
    ( *it ).reset();
    return true;
  }

  it = std::find( this->locked_for_processing_layers_.begin(), 
    this->locked_for_processing_layers_.end(), layer );

  if ( it != this->locked_for_processing_layers_.end() )
  {
    // Take the layer out of the list
    ( *it ).reset();
    return true;
  }

  it = std::find( this->locked_for_deletion_layers_.begin(),
    this->locked_for_deletion_layers_.end(), layer );
  if ( it != this->locked_for_deletion_layers_.end() )
  {
    ( *it ).reset();
    return true;
  }
  
  // Check whether the locked layer is still in the list of layers that this filter created
  it = std::find( this->created_layers_.begin(), this->created_layers_.end(), layer );

  if ( it != this->created_layers_.end() )
  {
    // Take the layer out of the list
    ( *it ).reset();
    return true;
  }

  return false;
}

void LayerFilterPrivate::finalize()
{
  bool abort = false;
  {
    boost::mutex::scoped_lock lock( this->mutex_ );
    abort = this->abort_;
  }
    
  // Disconnect all the connections with the layer signals, i.e. the abort signal from target
  // layers.
  this->disconnect_all();

  for ( size_t j = 0; j < this->locked_for_use_layers_.size(); j++ )
  {
    // Only unlock it if the handle still exists
    if ( this->locked_for_use_layers_[ j ] )
    {
      LayerManager::DispatchUnlockLayer( this->locked_for_use_layers_[ j ], 
        this->key_, this->sandbox_ );
      this->locked_for_use_layers_[ j ].reset();
    }
  }

  for ( size_t j = 0; j < this->locked_for_processing_layers_.size(); j++ )
  {
    // Only unlock it if the handle still exists
    if ( this->locked_for_processing_layers_[ j ] )
    {
      LayerManager::DispatchUnlockLayer( this->locked_for_processing_layers_[ j ], 
        this->key_, this->sandbox_ );
      this->locked_for_processing_layers_[ j ].reset();
    }
  }

  for ( size_t j = 0; j < this->locked_for_deletion_layers_.size(); ++j )
  {
    if ( this->locked_for_deletion_layers_[ j ] )
    {
      LayerManager::DispatchUnlockLayer( this->locked_for_deletion_layers_[ j ], 
        this->key_, this->sandbox_ );
      this->locked_for_deletion_layers_[ j ].reset();
    } 
  }

  if ( abort )
  {
    for ( size_t j = 0; j < this->created_layers_.size(); j++ )
    {
      if ( this->created_layers_[ j ] )
      {
        LayerManager::DispatchDeleteLayer( this->created_layers_[ j ], this->key_, this->sandbox_ );
        this->created_layers_[ j ].reset();
      }
    }
  }
  else
  {
    for ( size_t j = 0; j < this->created_layers_.size(); j++ )
    {
      if ( this->created_layers_[ j ] )
      {
        LayerManager::DispatchUnlockOrDeleteLayer( this->created_layers_[ j ], 
          this->key_, this->sandbox_ );
        this->created_layers_[ j ].reset();
      }
    }
  }
  
  if ( this->error_.size() )
  {
    // This is unnecessary since the error was already logged in report_error.
    //CORE_LOG_ERROR( this->error_ ); 
  }
  else if ( this->success_.size() )
  {
    CORE_LOG_SUCCESS( this->success_ ); 
  }

  // Notify that the filter is done
  this->notifier_->trigger();
}

void LayerFilterPrivate::internal_abort()
{
  boost::mutex::scoped_lock lock( this->mutex_ );
  if ( !this->abort_ )
  {
    this->abort_ = true;
    this->filter_->report_error( "Processing was aborted." );
    this->filter_->handle_abort();
  }
}

void LayerFilterPrivate::create_undo_redo_record( Core::ActionContextHandle context, 
                         Core::ActionHandle action )
{
  // Create a new undo/redo record
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( this->filter_->get_filter_name() ) );
  this->undo_item_weak_handle_ = item;

  // Keep track of the filter
  item->add_filter_to_abort( this->filter_->shared_from_this() );

  std::vector<LayerHandle>::iterator it, it_end;
  // Figure out which layers need to be deleted 
  it = this->created_layers_.begin();
  it_end = this->created_layers_.end();

  while ( it != it_end )
  {
    item->add_layer_to_delete( *it );
    ++it;
  }

  // Figure out which layers need to be restore using a check point
  it = this->volume_check_point_layers_.begin();
  it_end = this->volume_check_point_layers_.end();

  while ( it != it_end )
  {
    item->add_layer_to_restore( *it, LayerCheckPointHandle( new LayerCheckPoint( *it ) ) );
    ++it;
  }

  // Figure out which layers need to be added
  it = this->deleted_layers_.begin();
  it_end = this->deleted_layers_.end();

  while ( it != it_end )
  {
    item->add_layer_to_add( *it );
    ++it;
  }

  // Add the redo action
  item->set_redo_action( action );

  // Add id count to restore 
  item->add_id_count_to_restore( this->id_count_ );

  // Set the provenance step ID 
  item->set_provenance_step_ids( this->prov_step_ids_ );

  // Insert the record into the undo buffer
  UndoBuffer::Instance()->insert_undo_item( context, item );

  this->volume_check_point_layers_.clear();
  this->deleted_layers_.clear();
}

void LayerFilterPrivate::create_provenance_record( Core::ActionContextHandle context, 
                          Core::ActionHandle action, bool split_prov )
{
  // Provenance is only recorded for LayerActions
  LayerAction* layer_action = dynamic_cast< LayerAction* >( action.get() ); 
  if ( ! layer_action ) 
  {
    CORE_THROW_LOGICERROR( "LayerFilter should only be used with LayerAction objects." );
  }

  // Figure out what provenance numbers need to be send down to the layers
  // when finalizing this filter 
  size_t output_layer_count = 0;
  ProvenanceIDList output_provenance_ids;
  ProvenanceIDList deleted_provenance_ids;

  for ( size_t j = 0; j < this->created_layers_.size(); j++ )
  {
    if ( this->created_layers_[ j ] ) 
    {
      ProvenanceID prov_id = layer_action->get_output_provenance_id( output_layer_count++ );
      this->provenance_ids_[ this->created_layers_[ j ].get() ] = prov_id;
      output_provenance_ids.push_back( prov_id );
    }
  }

  for ( size_t j = 0; j < this->locked_for_processing_layers_.size(); j++ )
  {
    if ( this->locked_for_processing_layers_[ j ] ) 
    {
      ProvenanceID prov_id = layer_action->get_output_provenance_id( output_layer_count++ ); 
      this->provenance_ids_[ this->locked_for_processing_layers_[ j ].get() ] = prov_id;
      output_provenance_ids.push_back( prov_id );
      deleted_provenance_ids.push_back( this->locked_for_processing_layers_[ j ]->
        provenance_id_state_->get() );
    }
  }

  for ( size_t j = 0; j < this->locked_for_deletion_layers_.size(); ++j )
  {
    if ( this->locked_for_deletion_layers_[ j ] )
    {
      deleted_provenance_ids.push_back( this->locked_for_deletion_layers_[ j ]->
        provenance_id_state_->get() );
    }
  }

  ProvenanceIDList input_provenance_ids = layer_action->get_input_provenance_ids();

  if ( split_prov )
  {
    size_t num_inputs = input_provenance_ids.size();
    if ( num_inputs != output_provenance_ids.size() )
    {
      CORE_THROW_LOGICERROR( "Can't split provenance because the number of inputs"
        " doesn't match the number of outputs." );
    }

    if ( deleted_provenance_ids.size() > 0 && deleted_provenance_ids.size() != num_inputs )
    {
      CORE_THROW_LOGICERROR( "Can't split provenance because the number of replaced IDs"
        " doesn't match the number of inputs." );
    }
    
    this->prov_step_ids_.resize( num_inputs );
    for ( size_t i = 0; i < num_inputs; ++i )
    {
      this->provenance_step_.reset( new ProvenanceStep );

      this->provenance_step_->set_input_provenance_ids( ProvenanceIDList( 1, input_provenance_ids[ i ] ) );
      this->provenance_step_->set_output_provenance_ids( ProvenanceIDList( 1, output_provenance_ids[ i ] ) );
      if ( deleted_provenance_ids.size() > 0 )
      {
        this->provenance_step_->set_replaced_provenance_ids( ProvenanceIDList( 1, deleted_provenance_ids[ i ] ) );
      }
      // Get the input command of what needs t be rerun
      this->provenance_step_->set_action_name( layer_action->get_type() );
      this->provenance_step_->set_action_params( 
        layer_action->export_params_to_provenance_string( true ) );

      this->prov_step_ids_[ i ] = ProjectManager::Instance()->get_current_project()->
        add_provenance_record( this->provenance_step_ );
    }
  }
  else
  {
    // Create a provenance record
    this->provenance_step_ = ProvenanceStepHandle( new ProvenanceStep );
    // Get the input provenance ids from the translate step
    this->provenance_step_->set_input_provenance_ids( 
      layer_action->get_input_provenance_ids() );
    // Get the output and replace provenance ids from the analysis above
    this->provenance_step_->set_output_provenance_ids( output_provenance_ids );
    this->provenance_step_->set_replaced_provenance_ids( deleted_provenance_ids );

    // Get the input command of what needs t be rerun
    this->provenance_step_->set_action_name( layer_action->get_type() );
    this->provenance_step_->set_action_params( 
      layer_action->export_params_to_provenance_string() );

    this->prov_step_ids_.push_back( ProjectManager::Instance()->get_current_project()->
      add_provenance_record( this->provenance_step_ ) );
  }
}

//////////////////////////////////////////////////////////////////////////
// Class LayerFilter
//////////////////////////////////////////////////////////////////////////

LayerFilter::LayerFilter() :
  private_( new LayerFilterPrivate )
{
  this->private_->filter_ = this;
  this->private_->notifier_.reset( new LayerFilterNotifier( "Layer Filter" ) );
}

LayerFilter::~LayerFilter()
{
  this->private_->finalize();
}

void LayerFilter::raise_abort()
{
  {
    boost::mutex::scoped_lock lock( this->private_->mutex_ );
    if ( this->private_->abort_ ) return;
  }

  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &LayerFilter::raise_abort, 
      boost::dynamic_pointer_cast< LayerFilter >( this->shared_from_this() ) ) );
    return;
  }

  LayerUndoBufferItemHandle undo_item = this->private_->undo_item_weak_handle_.lock();
  // If there exists an undo item, use it to roll back any changes. The actual abort will happen
  // during the rollback.
  // NOTE: This won't delete the undo record from undo buffer, as there might be some
  // extra cleanups required to fully undo the action (such as restoring layer ID count).
  if ( undo_item )
  {
    undo_item->rollback_layer_changes();
  }
  // Otherwise, abort immediately.
  else
  {
    this->private_->internal_abort();
  }
}

void LayerFilter::raise_stop()
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  this->private_->stop_ = true;
  this->handle_stop();
}

bool LayerFilter::check_abort()
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  return this->private_->abort_;
}

bool LayerFilter::check_stop()
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  return this->private_->stop_;
}

void LayerFilter::abort_and_wait()
{
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "abort_and_wait can only be called from the"
      " application thread." ); 
  }
  
  // Raise the abort in case it wasn't raised.
  this->private_->internal_abort();
  
  {
    boost::mutex::scoped_lock lock( this->private_->mutex_ ); 
    while( this->private_->done_ == false )
    {
      this->private_->filter_done_.wait( lock );
    }
  }
  
  this->private_->finalize();
}


void LayerFilter::connect_abort( const  LayerHandle& layer )
{
  // TODO: connect to a free function instead of a member function
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  this->private_->add_connection( layer->abort_signal_.connect( boost::bind(
    &LayerFilter::raise_abort, this ) ) );
}

void LayerFilter::connect_stop( const  LayerHandle& layer )
{
  // TODO: connect to a free function instead of a member function
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  this->private_->add_connection( layer->stop_signal_.connect( boost::bind(
    &LayerFilter::raise_stop, this ) ) );
}

void LayerFilter::handle_abort()
{
}

void LayerFilter::handle_stop()
{
}

void LayerFilter::report_error( const std::string& error )
{
  std::string filter_error = this->get_filter_name() +": " + error;
  CORE_LOG_ERROR( filter_error );
  this->private_->error_ = filter_error;
}

bool LayerFilter::find_layer( const std::string& layer_id, LayerHandle& layer )
{
  layer = LayerManager::FindLayer( layer_id, this->private_->sandbox_ );
  if ( layer )
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool LayerFilter::lock_for_use( LayerHandle layer )
{
  // Lock the layer as an input layer and add the filter key to keep track of which filter
  // locked the layer
  if ( !( LayerManager::LockForUse( layer, this->private_->key_ ) ) ) 
  {
    this->report_error( "Could not lock '" + layer->get_layer_name() + "'." );
    return false;
  }
  
  // Add the layer to the internal data base so they can be released after the filter is done
  this->private_->locked_for_use_layers_.push_back( layer );
  return true;
}

bool LayerFilter::lock_for_processing( LayerHandle layer )
{
  // Check whether there has been another key still present and if we are the only
  // layer that will filter this layer.
  if ( !( LayerManager::LockForProcessing( layer, this->private_->key_ ) ) )
  {
    this->report_error( "Could not lock '" + layer->get_layer_name() + "'." );
    return false;
  }
  
  // As we are processing with this filter. The filter pointer is added to the layer, to ensure
  // we can kill the filter in case the layer needs to be undone.
  layer->set_filter_handle( this->shared_from_this() );
  
  // Add the layer to the list so it can be unlocked when the filter is done
  this->private_->locked_for_processing_layers_.push_back( layer );
  
  // Add the volume to be check pointed. 
  this->private_->volume_check_point_layers_.push_back( layer );
  
  // Hook up the abort signal from the layer
  this->connect_abort( layer );
  this->connect_stop( layer );
  
  return true;
}

bool LayerFilter::lock_for_deletion( LayerHandle layer )
{
  // Check whether there has been another key still present and if we are the only
  // layer that will filter this layer.
  if ( !( LayerManager::LockForProcessing( layer, this->private_->key_ ) ) )
  {
    this->report_error( "Could not lock '" + layer->get_layer_name() + "'." );
    return false;
  }
  
  // As we are processing with this filter. The filter pointer is added to the layer, to ensure
  // we can kill the filter in case the layer needs to be undone.
  layer->set_filter_handle( this->shared_from_this() );
  
  // Add the layer to the list so it can be unlocked when the filter is done
  this->private_->locked_for_deletion_layers_.push_back( layer );
  
  // Add the layer to be deleted at the end
  this->private_->deleted_layers_.push_back( layer );
  
  // Hook up the abort signal from the layer
  this->connect_abort( layer );
  this->connect_stop( layer );
  
  return true;
}


bool LayerFilter::create_and_lock_data_layer_from_layer( LayerHandle src_layer, 
  LayerHandle& dst_layer )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + src_layer->get_layer_name();

  // Create the layer in creating mode
  if ( !( LayerManager::CreateAndLockDataLayer( src_layer->get_grid_transform(),
    name, dst_layer, src_layer->get_meta_data(), this->private_->key_, this->private_->sandbox_ ) ) )
  {
    dst_layer.reset();
    this->report_error( "Could not allocate enough memory." );
    return false;
  }
  
  if( src_layer->get_type() == Core::VolumeType::DATA_E )
  {
    DataLayerHandle temp_source = boost::dynamic_pointer_cast< DataLayer >( src_layer );
    DataLayerHandle temp_destination = boost::dynamic_pointer_cast< DataLayer >( dst_layer );
    
    temp_destination->contrast_state_->set( temp_source->contrast_state_->get() );
    temp_destination->brightness_state_->set( temp_source->brightness_state_->get() );
  }


  // Record that the layer is locked
  this->private_->created_layers_.push_back( dst_layer );

  dst_layer->set_filter_handle( this->shared_from_this() );

  // Hook up the abort signal from the layer
  this->connect_abort( dst_layer );
  this->connect_stop( dst_layer );

  // Success
  return true;
}

bool LayerFilter::create_and_lock_data_layer( const Core::GridTransform& grid_trans, 
                      LayerHandle src_layer, LayerHandle& dst_layer )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + src_layer->get_layer_name();

  // Create the layer in creating mode
  if ( !( LayerManager::CreateAndLockDataLayer( grid_trans, name, dst_layer,
     src_layer->get_meta_data(), this->private_->key_, this->private_->sandbox_ ) ) )
  {
    dst_layer.reset();
    this->report_error( "Could not allocate enough memory." ); 
    return false;
  }

  // Record that the layer is locked
  this->private_->created_layers_.push_back( dst_layer );

  dst_layer->set_filter_handle( this->shared_from_this() );

  // Hook up the abort signal from the layer
  this->connect_abort( dst_layer );
  this->connect_stop( dst_layer );

  // Success
  return true;
}

bool LayerFilter::create_cropped_large_volume_layer( const Core::GridTransform& crop_trans,
  LayerHandle src_layer, LayerHandle& dst_layer )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + src_layer->get_layer_name();

  LargeVolumeLayerHandle lv_layer = boost::dynamic_pointer_cast<LargeVolumeLayer>( src_layer );
  if (!lv_layer) return false;

  // Create the layer in creating mode
  LayerManager::CreateCroppedLargeVolumeLayer( lv_layer->get_schema(), crop_trans,
    name, dst_layer, src_layer->get_meta_data(), this->private_->sandbox_ );

  // Record that the layer is locked
  this->private_->created_layers_.push_back( dst_layer );

  // Success
  return true;
}

bool LayerFilter::create_and_lock_mask_layer_from_layer( LayerHandle src_layer, LayerHandle& dst_layer )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + src_layer->get_layer_name();

  // Create the layer in creating mode
  if ( !( LayerManager::CreateAndLockMaskLayer( src_layer->get_grid_transform(),
    name, dst_layer, src_layer->get_meta_data(), this->private_->key_, this->private_->sandbox_ ) ) )
  {
    dst_layer.reset();
    this->report_error( "Could not allocate enough memory." );
    return false;
  }
  
  // Record that the layer is locked
  this->private_->created_layers_.push_back( dst_layer );

  dst_layer->set_filter_handle( this->shared_from_this() );

  // Hook up the abort signal from the layer
  this->connect_abort( dst_layer );
  this->connect_stop( dst_layer );

  // Success
  return true;
}

bool LayerFilter::create_and_lock_mask_layer_from_layer( LayerHandle src_layer, 
                            LayerHandle& dst_layer, std::string dst_layer_name )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + dst_layer_name;

  // Create the layer in creating mode
  if ( !( LayerManager::CreateAndLockMaskLayer( src_layer->get_grid_transform(),
    name, dst_layer, src_layer->get_meta_data(), this->private_->key_, this->private_->sandbox_ ) ) )
  {
    dst_layer.reset();
    this->report_error( "Could not allocate enough memory." );
    return false;
  }

  // Record that the layer is locked
  this->private_->created_layers_.push_back( dst_layer );

  dst_layer->set_filter_handle( this->shared_from_this() );

  // Hook up the abort signal from the layer
  this->connect_abort( dst_layer );
  this->connect_stop( dst_layer );

  // Success
  return true;
}

bool LayerFilter::create_and_lock_mask_layer( const Core::GridTransform& grid_trans, 
                      LayerHandle src_layer, LayerHandle& dst_layer )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + src_layer->get_layer_name();

  // Create the layer in creating mode
  if ( !( LayerManager::CreateAndLockMaskLayer( grid_trans, name, dst_layer,
     src_layer->get_meta_data(), this->private_->key_, this->private_->sandbox_ ) ) )
  {
    dst_layer.reset();
    this->report_error( "Could not allocate enough memory." );
    return false;
  }

  // Record that the layer is locked
  this->private_->created_layers_.push_back( dst_layer );

  dst_layer->set_filter_handle( this->shared_from_this() );

  // Hook up the abort signal from the layer
  this->connect_abort( dst_layer );
  this->connect_stop( dst_layer );

  // Success
  return true;
}

bool LayerFilter::dispatch_unlock_layer( LayerHandle layer )
{
  if ( !( this->private_->delete_layer( layer ) ) )
  {
    this->report_error( "Internal error in filter logic." );
    return false;
  }

  // Send a request to the layer manager to unlock the layer.
  LayerManager::DispatchUnlockLayer( layer, this->private_->key_, this->private_->sandbox_ );

  // Done
  return true;
}

bool LayerFilter::dispatch_delete_layer( LayerHandle layer )
{
  if ( !( this->private_->delete_layer( layer ) ) )
  {
    this->report_error( "Internal error in filter logic." );
    return false;
  }

  // Send a request to the layer manager to unlock the layer.
  LayerManager::DispatchDeleteLayer( layer, this->private_->key_, this->private_->sandbox_ );

  // Done
  return true;
}

bool LayerFilter::dispatch_insert_data_volume_into_layer( LayerHandle layer, 
  Core::DataVolumeHandle data, bool update_histogram )
{
  // Check whether the layer is of the right type
  DataLayerHandle data_layer = boost::dynamic_pointer_cast<DataLayer>( layer );
  if ( ! data_layer ) return false;

  // Update the data volume if needed.
  // NOTE: We assume that this data volume is not shared by any other thread yet
  // Hence we can update the histogram on the calling thread.
  if ( update_histogram ) data->get_data_block()->update_histogram();
  
  // Find the provenance id that this layer will use
  ProvenanceID prov_id = -1;
  
  LayerFilterPrivate::provenance_map_type::iterator it = 
    this->private_->provenance_ids_.find( layer.get() );
    
  if ( it != this->private_->provenance_ids_.end() )
  {
    prov_id = ( *it ).second;
  }
  
  // Ensure that the application thread will process this update.
  LayerManager::DispatchInsertDataVolumeIntoLayer( data_layer, data, prov_id,
     this->private_->key_, this->private_->sandbox_ );
  return true;
}

bool LayerFilter::dispatch_insert_mask_volume_into_layer( LayerHandle layer, 
  Core::MaskVolumeHandle mask )
{ 
  // Check whether the layer is of the right type
  MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
  if ( ! mask_layer ) return false;

  // Ensure that the application thread will process this update.

  ProvenanceID prov_id = -1;
  
  LayerFilterPrivate::provenance_map_type::iterator it = 
    this->private_->provenance_ids_.find( layer.get() );
    
  if ( it != this->private_->provenance_ids_.end() )
  {
    prov_id = ( *it ).second;
  }

  LayerManager::DispatchInsertMaskVolumeIntoLayer( mask_layer, mask, prov_id,
    this->private_->key_, this->private_->sandbox_ );

  return true;
}

void LayerFilter::run()
{
  // NOTE: Running too many filters in parallel can cause a huge surge in memory
  // hence we restrict the maximum number of filters can run simultaneously.

  // If more filters are running wait until one of them finished computing
  LayerFilterLock::Instance()->lock();
  
  try
  {
    this->run_filter();
  }
  catch( ... )
  {
  }
  
  // Release the lock so another filter can start
  LayerFilterLock::Instance()->unlock();

  // Generate a message indicating that filter was terminated
  this->private_->success_ = this->get_filter_name() + " finished processing";
  
  // NOTE: Notify the application thread if it is waiting for this to succeed in which case
  // it will immediately finalize the filter.
  boost::mutex::scoped_lock lock( this->private_->mutex_ ); 
  this->private_->done_ = true;
  this->private_->filter_done_.notify_all();
}


Layer::filter_key_type LayerFilter::get_key() const
{
  return this->private_->key_;
}


void LayerFilter::create_undo_redo_and_provenance_record( 
  Core::ActionContextHandle context, Core::ActionHandle action, bool split_prov )
{
  // NOTE: Create the provenance record first, as the provenance step ID
  // is needed by the undo/redo record.
  // Only create the records when not in a sandbox
  if ( this->private_->sandbox_ == -1 )
  {
    this->private_->create_provenance_record( context, action, split_prov );
    if ( PreferencesManager::Instance()->enable_undo_state_->get() )
    {
      this->private_->create_undo_redo_record( context, action );
    }
  }
}

bool LayerFilter::update_provenance_action_string( Core::ActionHandle action )
{
  // Provenance is only recorded for LayerActions
  LayerAction* layer_action = dynamic_cast<LayerAction*>( action.get() ); 
  if ( ! layer_action ) return false;

  if ( this->private_->prov_step_ids_.size() > 1 )
  {
    CORE_THROW_LOGICERROR( "Can not split the provenance of an action that needs"
      " dynamic parameter update." );
  }
  
  if ( this->private_->sandbox_ == -1 )
  {
    this->private_->provenance_step_->set_action_params( 
      layer_action->export_params_to_provenance_string() );
    ProjectManager::Instance()->get_current_project()->update_provenance_record( 
      this->private_->prov_step_ids_[ 0 ], this->private_->provenance_step_ );
  }
  
  return true;
}

void LayerFilter::set_sandbox( SandboxID sandbox )
{
  this->private_->sandbox_ = sandbox;
}

SandboxID LayerFilter::get_sandbox()
{
  return this->private_->sandbox_;
}

Core::NotifierHandle LayerFilter::get_notifier()
{
  return this->private_->notifier_;
}

} // end namespace Seg3D
