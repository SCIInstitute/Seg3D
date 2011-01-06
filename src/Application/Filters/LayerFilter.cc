/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
 
// Boost includes
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/thread/mutex.hpp> 
#include <boost/thread/condition_variable.hpp> 
 
// Core includes
#include <Core/Utils/Log.h>

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/LayerFilterLock.h>
#include <Application/StatusBar/StatusBar.h>
 
namespace Seg3D
{

class LayerFilterPrivate : public Core::ConnectionHandler
{
  // -- constructor --
public:
  LayerFilterPrivate() :
    done_( false ),
    abort_( false ),
    key_( Layer::GenerateFilterKey() ),
    id_count_( LayerManager::GetLayerIdCount() )
  {
  }


public:
  // Keep track of errors
  std::string error_;
  
  // Message that is relayed in case there is no error
  std::string success_;
    
  // Keep track of which layers were locked.
  std::vector< LayerHandle > locked_layers_;
  
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
  
  // -- internal functions --
public:
  // FINALIZE:
  // Clean up all the filter components and release the locks on the layers and
  // delete layers if the filter did not finish.
  void finalize();
};

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

  // Clean 
  for ( size_t j = 0; j < this->locked_layers_.size(); j++ )
  {
    LayerManager::DispatchUnlockLayer( this->locked_layers_[ j ], this->key_ );
  }

  if ( this->abort_ )
  {
    for ( size_t j = 0; j < this->created_layers_.size(); j++ )
    {
      LayerManager::DispatchDeleteLayer( this->created_layers_[ j ], this->key_ );
    }
  }
  else
  {
    for ( size_t j = 0; j < this->created_layers_.size(); j++ )
    {
      LayerManager::DispatchUnlockOrDeleteLayer( this->created_layers_[ j ], this->key_ );
    }
  }
  
  if ( this->error_.size() )
  {
    CORE_LOG_ERROR( this->error_ ); 
  }
  else if ( this->success_.size() )
  {
    CORE_LOG_SUCCESS( this->success_ ); 
  }

  this->locked_layers_.clear();
  this->created_layers_.clear();
  this->volume_check_point_layers_.clear();
}



LayerFilter::LayerFilter() :
  private_( new LayerFilterPrivate )
{
}

LayerFilter::~LayerFilter()
{
  this->private_->finalize();
}


void LayerFilter::raise_abort()
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  this->private_->abort_ = true;
  this->report_error( "Processing was aborted." );
  this->handle_abort();
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
  this->raise_abort();
  
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
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  this->private_->add_connection( layer->abort_signal_.connect( boost::bind(
    &LayerFilter::raise_abort, this ) ) );
}

void LayerFilter::connect_stop( const  LayerHandle& layer )
{
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
  layer = LayerManager::Instance()->get_layer_by_id( layer_id );
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
  this->private_->locked_layers_.push_back( layer );
  return true;
}

bool LayerFilter::lock_for_processing( LayerHandle layer, bool check_point_volume )
{
  // Check whether there has been another key still present and if we are the only
  // layer.
  
  if ( !( LayerManager::LockForProcessing( layer, this->private_->key_ ) ) )
  {
    this->report_error( "Could not lock '" + layer->get_layer_name() + "'." );
    return false;
  }
  
  // As we are processing with this filter. The filter pointer is added to the layer, to ensure
  // we can kill the filter in case the layer needs to be undone
  layer->set_filter_handle( this->shared_from_this() );
  
  this->private_->locked_layers_.push_back( layer );
  if ( check_point_volume ) 
  {
    this->private_->volume_check_point_layers_.push_back( layer );
  }
  else
  {
    this->private_->deleted_layers_.push_back( layer );
  }
  
  // Hook up the abort signal from the layer
  this->connect_abort( layer );
  
  return true;
}

bool LayerFilter::create_and_lock_data_layer_from_layer( LayerHandle src_layer, 
  LayerHandle& dst_layer )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + src_layer->get_layer_name();

  // Create the layer in creating mode
  if ( !( LayerManager::CreateAndLockDataLayer( src_layer->get_grid_transform(),
    name, dst_layer, this->private_->key_ ) ) )
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
    this->private_->key_ ) ) )
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

  // Success
  return true;
}

bool LayerFilter::create_and_lock_mask_layer_from_layer( LayerHandle src_layer, LayerHandle& dst_layer )
{
  // Generate a new name for the filter
  std::string name = this->get_layer_prefix() + "_" + src_layer->get_layer_name();

  // Create the layer in creating mode
  if ( !( LayerManager::CreateAndLockMaskLayer( src_layer->get_grid_transform(),
    name, dst_layer, this->private_->key_ ) ) )
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
    this->private_->key_ ) ) )
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

  // Success
  return true;
}

bool LayerFilter::dispatch_unlock_layer( LayerHandle layer )
{
  bool found_layer = false;
  // Check whether the locked layer is still in the list of layers that this filter locked
  std::vector<LayerHandle>::iterator it;
  
  it = std::find( this->private_->locked_layers_.begin(), 
    this->private_->locked_layers_.end(), layer );

  if ( it != this->private_->locked_layers_.end() )
  {
    // Take the layer out of the list
    this->private_->locked_layers_.erase( it );
    found_layer = true;
  }
  
  // Check whether the locked layer is still in the list of layers that this filter created
  it = std::find( this->private_->created_layers_.begin(), 
    this->private_->created_layers_.end(), layer );

  if ( it != this->private_->created_layers_.end() )
  {
    // Take the layer out of the list
    this->private_->created_layers_.erase( it );
    found_layer = true;
  }

  // Check whether the locked layer is still in the list of layers that this filter created
  it = std::find( this->private_->volume_check_point_layers_.begin(), 
    this->private_->volume_check_point_layers_.end(), layer );

  if ( it != this->private_->volume_check_point_layers_.end() )
  {
    // Take the layer out of the list
    this->private_->volume_check_point_layers_.erase( it );
  }

  // If we did not find the layer return false, as we did not lock it in the first place.
  if ( ! found_layer ) 
  {
    this->report_error( "Internal error in filter logic." );
    return false;
  }

  // Send a request to the layer manager to unlock the layer.
  LayerManager::DispatchUnlockLayer( layer, this->private_->key_ );

  // Done
  return true;
}

bool LayerFilter::dispatch_delete_layer( LayerHandle layer )
{
  bool found_layer = false;
  
  // Check whether the locked layer is still in the list of layers that this filter created
  std::vector<LayerHandle>::iterator it = std::find( 
    this->private_->created_layers_.begin(), this->private_->created_layers_.end(), layer );

  if ( it != this->private_->created_layers_.end() )
  {
    // Take the layer out of the list
    this->private_->created_layers_.erase( it );
    found_layer = true;
  }

  it = std::find( this->private_->locked_layers_.begin(), 
    this->private_->locked_layers_.end(), layer );
  if ( it != this->private_->locked_layers_.end() )
  {
    this->private_->locked_layers_.erase( it );
    found_layer = true;
  }

  it = std::find( this->private_->deleted_layers_.begin(),
    this->private_->deleted_layers_.end(), layer );
  if ( it != this->private_->deleted_layers_.end() )
  {
    this->private_->deleted_layers_.erase( it );
    found_layer = true;
  }
  
  it = std::find( this->private_->volume_check_point_layers_.begin(), 
    this->private_->volume_check_point_layers_.end(), layer );
  if ( it != this->private_->volume_check_point_layers_.end() )
  {
    this->private_->volume_check_point_layers_.erase( it );
  }
  
  // If we did not find the layer return false, as we did not lock it in the first place.
  if ( ! found_layer ) 
  {
    this->report_error( "Internal error in filter logic." );
    return false;
  }

  // Send a request to the layer manager to unlock the layer.
  LayerManager::DispatchDeleteLayer( layer, this->private_->key_ );

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
  
  // Ensure that the application thread will process this update.
  LayerManager::DispatchInsertDataVolumeIntoLayer( data_layer, data,
     this->private_->key_ );
  return true;
}


bool LayerFilter::dispatch_insert_mask_volume_into_layer( LayerHandle layer, 
  Core::MaskVolumeHandle mask )
{ 
  // Check whether the layer is of the right type
  MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
  if ( ! mask_layer ) return false;

  // Ensure that the application thread will process this update.
  LayerManager::DispatchInsertMaskVolumeIntoLayer( mask_layer, mask,
    this->private_->key_ );
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
  
  // Notify if application thread if it is waiting for this to succeed in which case
  // it will immediately finalize the filter.
  boost::mutex::scoped_lock lock( this->private_->mutex_ ); 
  this->private_->done_ = true;
  this->private_->filter_done_.notify_all();
  
}

Layer::filter_key_type LayerFilter::get_key() const
{
  return this->private_->key_;
}

bool LayerFilter::create_undo_redo_record( Core::ActionContextHandle context, Core::ActionHandle redo_action )
{
  // Create a new undo/redo record
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( get_filter_name() ) );

  // Keep track of the filter
  item->add_filter_to_abort( this->shared_from_this() );
  
  // Figure out which layers need to be deleted
  std::vector<LayerHandle>::iterator it = this->private_->created_layers_.begin();
  std::vector<LayerHandle>::iterator it_end = this->private_->created_layers_.end();
  
  while ( it != it_end )
  {
    item->add_layer_to_delete( *it );
    ++it;
  }

  // Figure out which layers need to be restore using a check point
  it = this->private_->volume_check_point_layers_.begin();
  it_end = this->private_->volume_check_point_layers_.end();
  
  while ( it != it_end )
  {
    item->add_layer_to_restore( *it, LayerCheckPointHandle( new LayerCheckPoint( *it ) ) );
    ++it;
  }

  // Figure out which layers need to be added
  std::for_each( this->private_->deleted_layers_.begin(), this->private_->deleted_layers_.end(),
    boost::lambda::bind( &LayerUndoBufferItem::add_layer_to_add, item.get(), boost::lambda::_1 ) );
  
  // Add the redo action
  item->set_redo_action( redo_action );

  // Add id count to restore 
  item->add_id_count_to_restore( this->private_->id_count_ );
  
  // Insert the record into the undo buffer
  UndoBuffer::Instance()->insert_undo_item( context, item );

  return true;
}

} // end namespace Seg3D
