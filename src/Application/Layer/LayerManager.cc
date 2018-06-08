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

// Core includes
#include <Core/Application/Application.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/DataBlockManager.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Utils/Exception.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/LayerScene.h>
#include <Application/Layer/LayerAvailabilityNotifier.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

// Boost includes
#include <boost/foreach.hpp>
#include <boost/smart_ptr.hpp>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class LayerManagerPrivate
//////////////////////////////////////////////////////////////////////////
  
typedef std::list < LayerGroupHandle > GroupList;

typedef std::map< std::string, LayerHandle > LayerSandbox;
typedef boost::shared_ptr< LayerSandbox > LayerSandboxHandle;
typedef std::map< SandboxID, LayerSandboxHandle > LayerSandboxMap;

class LayerManagerPrivate
{
public:
  // UPDATE_LAYER_LIST:
  // Update the option list of active_layer_state_.
  void update_layer_list();

  // HANDLE_ACTIVE_LAYER_STATE_CHANGED:
  // This function is connected to active_layer_state_::value_changed_signal_.
  void handle_active_layer_state_changed( std::string layer_id );

  // HANDLE_ACTIVE_LAYER_CHANGED:
  // Called by LayerManager::set_active_layer function.
  void handle_active_layer_changed();

  // RESET:
  // Reset the layer manager, invalidate all layers.
  void reset();

  // HANDLE_LAYER_NAME_CHANGED:
  // This function is connected to the value_changed_signal_ of all the layers and relays
  // it to the layer_name_changed_signal_ of LayerManager.
  void handle_layer_name_changed( std::string layer_id, std::string name );

  // HANDLE_LAYER_DATA_CHANGED:
  // This function keeps track off when layer data is changed.
  void handle_layer_data_changed( LayerWeakHandle layer );

  // FIND_FREE_COLOR:
  // Find a color that has not yet been used.
  int find_free_color();

  // FIND_SANDBOX:
  // Find the specified sandbox.
  LayerSandboxHandle find_sandbox( SandboxID sandbox );

  // An internal counter for temporarily blocking certain signals from being processed.
  size_t signal_block_count_;
  // A list of layer groups
  GroupList group_list_;
  // The active layer
  LayerHandle active_layer_;
  // Pointer to the layer manager instance
  LayerManager* layer_manager_;
  // A map of sandboxes for processing in the background
  LayerSandboxMap sandboxes_;
  // Sandbox counter
  SandboxID sandbox_count_;
};

void LayerManagerPrivate::update_layer_list()
{
  std::vector< LayerIDNamePair > layers;
  this->layer_manager_->get_layer_names( layers );

  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->layer_manager_->active_layer_state_->set_option_list( layers );
    if ( this->active_layer_ )
    {
      this->layer_manager_->active_layer_state_->set( this->active_layer_->get_layer_id() );
    }
  }
}

void LayerManagerPrivate::handle_active_layer_state_changed( std::string layer_id )
{
  if ( this->signal_block_count_ > 0 || layer_id == "" )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );

  this->layer_manager_->set_active_layer( this->layer_manager_->find_layer_by_id( layer_id ) );
  // Set the state again because the requested active layer might not be valid
  if ( this->active_layer_ )
  {
    this->layer_manager_->active_layer_state_->set( this->active_layer_->get_layer_id() );
  }
}

void LayerManagerPrivate::handle_active_layer_changed()
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );
  if ( this->active_layer_ )
  {
    this->layer_manager_->active_layer_state_->set( this->active_layer_->get_layer_id() );
  }
}

void LayerManagerPrivate::reset()
{
  ASSERT_IS_APPLICATION_THREAD();

  // Clean up all the data blocks.
  Core::MaskDataBlockManager::Instance()->clear();
  Core::DataBlockManager::Instance()->clear();

  // Cycle through all the groups and delete all the layers
  GroupList::iterator group_iterator = this->group_list_.begin();
  while ( group_iterator != this->group_list_.end() )
  {
    ( *group_iterator )->clear();
    ( *group_iterator )->invalidate();
    ++group_iterator;
  }
  this->active_layer_.reset();
  this->group_list_.clear();
}

void LayerManagerPrivate::handle_layer_name_changed( std::string layer_id, std::string name )
{
  this->layer_manager_->layer_name_changed_signal_( layer_id, name );
}

void LayerManagerPrivate::handle_layer_data_changed( LayerWeakHandle layer_weak_handle )
{
  LayerHandle layer = layer_weak_handle.lock();
  if ( layer )
  {
    this->layer_manager_->layer_data_changed_signal_( layer );
  }
}

int LayerManagerPrivate::find_free_color()
{
  std::set< int > used_colors;
  for( GroupList::iterator i = this->group_list_.begin(); 
    i != this->group_list_.end(); ++i )
  {
    LayerList& layer_list = ( *i )->get_layer_list();
    for( LayerList::iterator j = layer_list.begin(); 
      j != layer_list.end(); ++j )
    {
      if( ( *j )->get_type() == Core::VolumeType::MASK_E )
      {
        used_colors.insert( boost::dynamic_pointer_cast< MaskLayer >( ( *j ) )->color_state_->get() );
      }
    }
  }
  for( int i = 0; i < 12; ++i )
  {
    if( used_colors.find( i ) == used_colors.end() )
    {
      return i;
    }
  }
  return -1;
}

LayerSandboxHandle LayerManagerPrivate::find_sandbox( SandboxID sandbox )
{
  LayerSandboxMap::iterator it = this->sandboxes_.find( sandbox );
  if ( it != this->sandboxes_.end() )
  {
    return it->second;
  }
  return LayerSandboxHandle();
}

//////////////////////////////////////////////////////////////////////////
// Class LayerManager
//////////////////////////////////////////////////////////////////////////

CORE_SINGLETON_IMPLEMENTATION( LayerManager );

LayerManager::LayerManager() :
  StateHandler( "layermanager", false ),
  private_( new LayerManagerPrivate )
{ 
  this->mark_as_project_data();

  this->add_state( "active_layer", this->active_layer_state_, "", "" );
  this->private_->signal_block_count_ = 0;
  this->private_->layer_manager_ = this;
  this->private_->sandbox_count_ = 1;

  this->add_connection( this->layers_changed_signal_.connect( boost::bind( 
    &LayerManagerPrivate::update_layer_list, this->private_ ) ) );
  this->add_connection( this->layer_name_changed_signal_.connect( boost::bind(
    &LayerManagerPrivate::update_layer_list, this->private_ ) ) );
  this->add_connection( this->active_layer_state_->value_changed_signal_.connect( boost::bind( 
    &LayerManagerPrivate::handle_active_layer_state_changed, this->private_, _2 ) ) );
  this->add_connection( Core::Application::Instance()->reset_signal_.connect( boost::bind(
    &LayerManagerPrivate::reset, this->private_ ) ) );
}

LayerManager::~LayerManager()
{
  this->disconnect_all();
}
  
bool LayerManager::insert_layer( LayerHandle layer, SandboxID sandbox )
{
  ASSERT_IS_APPLICATION_THREAD();

  bool active_layer_changed = false;
  bool new_group = false;
  LayerGroupHandle group_handle;
  
  {
    lock_type lock( this->get_mutex() );

    // If a sandbox is given, add the layer to the sandbox
    if ( sandbox >= 0 )
    {
      LayerSandboxHandle layer_sandbox = this->private_->find_sandbox( sandbox );
      if ( layer_sandbox )
      {
        layer_sandbox->operator []( layer->get_layer_id() ) = layer;
        return true;
      }
      return false;
    }
    
    CORE_LOG_MESSAGE( std::string("Insert New Layer: ") + layer->get_layer_id());
    
    // if its a mask, lets reset its color to one that doesn't exist if possible
    if( layer->get_type() == Core::VolumeType::MASK_E ) 
    {
      int mask_color = this->private_->find_free_color();
      if( mask_color != -1 )
      { 
        boost::dynamic_pointer_cast< MaskLayer >( layer )->color_state_->set( mask_color );
      } 
    }
    
    // Find an existing group that has the same grid transform as the layer
    Core::GridTransform layer_grid_transform = layer->get_grid_transform();
    for ( GroupList::iterator it = this->private_->group_list_.begin(); 
       it != this->private_->group_list_.end(); ++it )
    {
      if ( layer_grid_transform == ( *it )->get_grid_transform() ) 
      {
        group_handle = *it;
        break;
      }
    }

    // No match, create a new group
    if ( !group_handle )  
    {
      new_group = true;
      group_handle.reset( new LayerGroup(  layer->get_grid_transform(),
        layer->provenance_id_state_->get(), layer->get_meta_data() ) );
      this->private_->group_list_.push_front( group_handle );
      
      // Make the new layer active since it's the first one of a new group
      if ( layer->has_valid_data() )
      {
        CORE_LOG_DEBUG( std::string( "Set Active Layer: " ) + layer->get_layer_id());
        this->private_->active_layer_ = layer;
        active_layer_changed = true;
      } 
    }
    
    group_handle->insert_layer( layer );
      
    // Connect to the value_changed_signal of layer name
    // NOTE: LayerManager will always out-live layers, so it's safe to not disconnect.
    layer->name_state_->value_changed_signal_.connect( boost::bind(
      &LayerManagerPrivate::handle_layer_name_changed, this->private_, layer->get_layer_id(), _2 ) );
      
    // NOTE: Add a connection here to check when layer data state changes
    // This is need to switch on/off menu options in the interface
    layer->data_state_->state_changed_signal_.connect( boost::bind(
      &LayerManagerPrivate::handle_layer_data_changed, this->private_, LayerWeakHandle( layer ) ) );
        
  } // unlocked from here

  CORE_LOG_DEBUG( std::string( "Signalling that new layer was inserted" ) );
  CORE_LOG_DEBUG( std::string( "--- triggering signals ---" ) );

  this->layer_inserted_signal_( layer, new_group );
  this->layers_changed_signal_();
  
  if( active_layer_changed )
  {
    active_layer_changed_signal_( layer );
  }
  
  return true;
}

bool LayerManager::move_group( LayerGroupHandle src_group, LayerGroupHandle dst_group )
{
  ASSERT_IS_APPLICATION_THREAD();

  // Find the src_group in the list
  GroupList::iterator src_it = std::find( this->private_->group_list_.begin(),
    this->private_->group_list_.end(), src_group );
  assert( src_it != this->private_->group_list_.end() );

  // Make sure that src_group is not already in the desired position
  GroupList::iterator tmp_it = src_it;
  ++tmp_it;
  if ( tmp_it != this->private_->group_list_.end() &&
    *tmp_it == dst_group )
  {
    return true; // Consider it successful
  }

  {
    // Acquire the lock
    lock_type lock( this->get_mutex() );

    // Remove the src_group from the list
    this->private_->group_list_.erase( src_it );

    // Find the dst_group in the list
    GroupList::iterator dst_it = std::find( this->private_->group_list_.begin(),
      this->private_->group_list_.end(), dst_group );
    assert( dst_it != this->private_->group_list_.end() );

    // Insert src_group to the new position
    this->private_->group_list_.insert( dst_it, src_group );
  }

  this->groups_reordered_signal_();
  this->layers_changed_signal_();

  return true;
}

bool LayerManager::move_layer( LayerHandle src_layer, LayerHandle dst_layer /*= LayerHandle() */ )
{
  ASSERT_IS_APPLICATION_THREAD();

  LayerGroupHandle layer_group = src_layer->get_layer_group();

  assert( !dst_layer || layer_group == dst_layer->get_layer_group() );

  if ( layer_group->move_layer( src_layer, dst_layer ) )
  {
    // NOTE: Only trigger signals if a change was made
    this->layers_reordered_signal_( layer_group->get_group_id() );
    this->layers_changed_signal_();
  }

  return true;
}

void LayerManager::set_active_layer( LayerHandle layer )
{
  ASSERT_IS_APPLICATION_THREAD();

  {
    lock_type lock( this->get_mutex() );    
    
    // Do nothing if this layer is already the active one
    if ( this->private_->active_layer_ == layer || !layer->has_valid_data() )
    {
      return;
    }
    
    CORE_LOG_DEBUG( std::string("Set Active Layer: ") + layer->get_layer_id());
    
    this->private_->active_layer_ = layer;
        
  } // We release the lock  here.

  this->private_->handle_active_layer_changed();
  this->active_layer_changed_signal_( layer );  
}

void LayerManager::shift_active_layer( bool downward /*= false */ )
{
  ASSERT_IS_APPLICATION_THREAD();

  LayerVector layers;
  this->get_layers( layers );

  // Return if no layer exists.
  if ( layers.empty() ) return;
  
  // Find the active layer in the vector
  LayerVector::iterator it = std::find( layers.begin(), layers.end(), this->private_->active_layer_ );
  assert( it != layers.end() );
  LayerVector::iterator::difference_type index = it - layers.begin();
  size_t num_of_layers = layers.size();

  // Search for the next possible active layer
  if ( downward )
  {
    for ( size_t i = static_cast< size_t >( index + 1 ); i < num_of_layers; ++i )
    {
      if ( layers[ i ]->get_layer_group()->group_widget_expanded_state_->get() &&
        layers[ i ]->has_valid_data() )
      {
        this->set_active_layer( layers[ i ] );
        return;
      }
    }
  }
  else
  {
    for ( int i = static_cast< int >( index - 1 ); i >= 0; --i )
    {
      if ( layers[ i ]->get_layer_group()->group_widget_expanded_state_->get() &&
        layers[ i ]->has_valid_data() )
      {
        this->set_active_layer( layers[ i ] );
        return;
      }
    }   
  }
}

LayerGroupHandle LayerManager::find_group( const std::string& group_id )
{
  lock_type lock( this->get_mutex() );
  
  for( GroupList::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    if (( *i )->get_group_id() == group_id ) 
    {
      return ( *i );
    }
  }
  return LayerGroupHandle();
}

LayerGroupHandle LayerManager::find_group( ProvenanceID prov_id )
{
  lock_type lock( this->get_mutex() );
  
  for( GroupList::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    if (( *i )->provenance_id_state_->get() == prov_id ) 
    {
      return ( *i );
    }
  }
  return LayerGroupHandle();
}


LayerHandle LayerManager::find_layer_by_id( const std::string& layer_id, SandboxID sandbox )
{
  lock_type lock( this->get_mutex() );

  // If a sandbox number is specified, look it up in the sandbox
  if ( sandbox >= 0 )
  {
    // Look for the sandbox
    LayerSandboxHandle layer_sandbox = this->private_->find_sandbox( sandbox );
    if ( layer_sandbox )
    {
      // Look for the layer with the ID in the sandbox
      LayerSandbox::iterator layer_it = layer_sandbox->find( layer_id );
      if ( layer_it != layer_sandbox->end() )
      {
        return layer_it->second;
      }
    }
    // Not found, return an empty handle
    return LayerHandle();
  }
  
  for( GroupList::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    LayerList& layer_list = ( *i )->get_layer_list();
    for( LayerList::iterator j = layer_list.begin(); j != layer_list.end(); ++j )
    {
      if( ( *j )->get_layer_id() == layer_id )
      {
        return ( *j );
      }
    }
  }
  return LayerHandle();
}

LayerHandle LayerManager::find_layer_by_name( const std::string& layer_name, SandboxID sandbox )
{
  lock_type lock( this->get_mutex() );

  // If a sandbox number is specified, look it up in the sandbox
  if ( sandbox >= 0 )
  {
    // Look for the sandbox
    LayerSandboxHandle layer_sandbox = this->private_->find_sandbox( sandbox );
    if ( layer_sandbox )
    {
      // Look for the layer with the name in the sandbox
      LayerSandbox::iterator layer_it = layer_sandbox->begin();
      for ( ; layer_it != layer_sandbox->end(); ++layer_it )
      {
        if ( layer_it->second->get_layer_name() == layer_name )
        {
          return layer_it->second;
        }
      }
    }
    // Not found, return an empty handle
    return LayerHandle();
  }

  for( GroupList::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    LayerList& layer_list = ( *i )->get_layer_list();
    for( LayerList::iterator j = layer_list.begin(); j != layer_list.end(); ++j )
    {
      if( ( *j )->get_layer_name() == layer_name )
      {
        return ( *j );
      }
    }
  }
  return LayerHandle();
}

LayerHandle LayerManager::find_layer_by_provenance_id( ProvenanceID provenance_id, SandboxID sandbox )
{
  lock_type lock( this->get_mutex() );

  // If a sandbox number is specified, look it up in the sandbox
  if ( sandbox >= 0 )
  {
    // Look for the sandbox
    LayerSandboxHandle layer_sandbox = this->private_->find_sandbox( sandbox );
    if ( layer_sandbox )
    {
      // Look for the layer with the name in the sandbox
      LayerSandbox::iterator layer_it = layer_sandbox->begin();
      for ( ; layer_it != layer_sandbox->end(); ++layer_it )
      {
        if ( layer_it->second->provenance_id_state_->get() == provenance_id )
        {
          return layer_it->second;
        }
      }
    }
    // Not found, return an empty handle
    return LayerHandle();
  }

  for( GroupList::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    LayerList& layer_list = ( *i )->get_layer_list();
    for( LayerList::iterator j = layer_list.begin(); j != layer_list.end(); ++j )
    {
      if( ( *j )->provenance_id_state_->get() == provenance_id )
      {
        return ( *j );
      }
    }
  }
  return LayerHandle();
}


DataLayerHandle LayerManager::find_data_layer_by_id( const std::string& layer_id, SandboxID sandbox )
{
  return boost::dynamic_pointer_cast<DataLayer>( find_layer_by_id( layer_id, sandbox ) );
}

MaskLayerHandle LayerManager::find_mask_layer_by_id( const std::string& layer_id, SandboxID sandbox )
{
  return boost::dynamic_pointer_cast<MaskLayer>( find_layer_by_id( layer_id, sandbox ) );
}

void LayerManager::get_groups( std::vector< LayerGroupHandle >& groups )
{
  lock_type lock( this->get_mutex() );
  groups.insert( groups.begin(), this->private_->group_list_.begin(), 
    this->private_->group_list_.end() );
}

void LayerManager::get_layers( std::vector< LayerHandle >& layers )
{
  lock_type lock( this->get_mutex() );
  
  for( GroupList::const_iterator i = 
    this->private_->group_list_.begin(); i != this->private_->group_list_.end(); ++i )
  {
    LayerList& layer_list = ( *i )->get_layer_list();
    layers.insert( layers.end(), layer_list.begin(), layer_list.end() );
  }
}

void LayerManager::delete_layers( const std::vector< LayerHandle >& layers, SandboxID sandbox )
{
  ASSERT_IS_APPLICATION_THREAD();

  if ( layers.empty() ) return;
  
  bool active_layer_changed = false;
  bool group_deleted = false;
  
  std::vector< std::string > layer_ids;
  std::set< std::string > group_id_set;
  
  { // start the lock scope
    lock_type lock( this->get_mutex() );  

    if ( sandbox >= 0 )
    {
      LayerSandboxHandle layer_sandbox = this->private_->find_sandbox( sandbox );
      if ( layer_sandbox )
      {
        BOOST_FOREACH( LayerHandle layer, layers )
        {
          layer_sandbox->erase( layer->get_layer_id() );
        }
      }
      return;
    }
    
    for( size_t i = 0; i < layers.size(); ++i )
    {
      if( !layers[ i ] ) continue;
      
      CORE_LOG_MESSAGE( std::string( "Deleting Layer: " ) + layers[ i ]->get_layer_id() );

      // NOTE: Layer invalidation has been moved to the LayerUndoBufferItem.
      // A layer will only be invalidated when the corresponding undo buffer item has been deleted.
      // This also applies to layer groups.

      LayerGroupHandle group = layers[ i ]->get_layer_group();
      group->delete_layer( layers[ i ] );
      layer_ids.push_back( layers[ i ]->get_layer_id() );
      group_id_set.insert( group->get_group_id() );
      
      if( group->is_empty() )
      {   
        this->private_->group_list_.remove( group );
        group_deleted = true;     
      }

      if ( this->private_->active_layer_ == layers[ i ] )
      {
        this->private_->active_layer_.reset();
        active_layer_changed = true;
      }
    }

    if ( active_layer_changed && this->private_->group_list_.size() > 0 )
    {
      this->private_->active_layer_ = this->private_->group_list_.front()->top_layer();
    }
  } 
  
  std::vector< std::string > group_ids;
  group_ids.insert( group_ids.begin(), group_id_set.begin(), group_id_set.end() );

  this->layers_deleted_signal_( layer_ids, group_ids, group_deleted );
  this->layers_changed_signal_();
  
  if ( active_layer_changed && this->private_->active_layer_ )
  {
    this->active_layer_changed_signal_( this->private_->active_layer_ );
  }
}

void LayerManager::delete_layer( LayerHandle layer, SandboxID sandbox /*= -1 */ )
{
  std::vector< LayerHandle > layers( 1, layer );
  this->delete_layers( layers, sandbox );
}

LayerHandle LayerManager::get_active_layer()
{
  // NOTE: This locks the state engine.  DO NOT call this if RenderResources is locked or a 
  // deadlock will occur.
  lock_type lock( this->get_mutex() );  
  return this->private_->active_layer_;
}

LayerManager::mutex_type& LayerManager::get_mutex()
{
  return Core::StateEngine::GetMutex();
}
  
LayerSceneHandle LayerManager::compose_layer_scene( size_t viewer_id )
{
  // NOTE: This functions is called from the Rendering Thread
  // Lock the LayerManager
  lock_type lock( this->get_mutex() );

  LayerSceneHandle layer_scene( new LayerScene );

  // For each layer group
  GroupList::reverse_iterator group_iterator = 
    this->private_->group_list_.rbegin();
  for ( ; group_iterator != this->private_->group_list_.rend(); group_iterator++)
  {
    
    const LayerList& layer_list = ( *group_iterator )->get_layer_list();

    LayerList::const_reverse_iterator layer_iterator = layer_list.rbegin();
    // For each layer in the group
    for ( ; layer_iterator != layer_list.rend(); layer_iterator++ )
    {
      LayerHandle layer = *layer_iterator;
      
      // Skip processing this layer if it's not visible or that is not valid.
      // NOTE: Layers that are not valid include the layers that are currently
      // under construction.
      if ( !layer->is_visible( viewer_id ) || !layer->has_valid_data() )
      {
        continue;
      }

      LayerSceneItemHandle layer_scene_item;

      switch( layer->get_type() )
      {
      case Core::VolumeType::DATA_E:
        {
          DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
          DataLayerSceneItem* data_layer_scene_item = new DataLayerSceneItem;
          layer_scene_item = LayerSceneItemHandle( data_layer_scene_item );
          data_layer_scene_item->data_min_ = data_layer->min_value_state_->get();
          data_layer_scene_item->data_max_ = data_layer->max_value_state_->get();
          data_layer_scene_item->display_min_ = data_layer->display_min_value_state_->get();
          data_layer_scene_item->display_max_ = data_layer->display_max_value_state_->get();
		  data_layer_scene_item->colormap_ = data_layer->colormap_state_->index();
          data_layer_scene_item->color_ = data_layer->color_state_->get();
          if ( data_layer_scene_item->display_min_ > data_layer_scene_item->display_max_ )
          {
            std::swap( data_layer_scene_item->display_min_, data_layer_scene_item->display_max_ );
          }
          data_layer_scene_item->volume_rendered_ = data_layer->
            volume_rendered_state_->get();
        }
        break;
      case Core::VolumeType::MASK_E:
        {
          MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
          MaskLayerSceneItem* mask_layer_scene_item = new MaskLayerSceneItem;
          layer_scene_item = LayerSceneItemHandle( mask_layer_scene_item );
          mask_layer_scene_item->color_ = mask_layer->color_state_->get();
          mask_layer_scene_item->border_ = mask_layer->border_state_->index();
          mask_layer_scene_item->fill_ = mask_layer->fill_state_->index();
          mask_layer_scene_item->show_isosurface_ = mask_layer->
            show_isosurface_state_->get();
        }
        break;
      case Core::VolumeType::LARGE_DATA_E:
        {
          LargeVolumeLayer* lv_layer = dynamic_cast<LargeVolumeLayer*>(layer.get());
          LargeVolumeLayerSceneItem* lv_layer_scene_item = new LargeVolumeLayerSceneItem;
          layer_scene_item = LayerSceneItemHandle(lv_layer_scene_item);
          lv_layer_scene_item->data_min_ = lv_layer->min_value_state_->get();
          lv_layer_scene_item->data_max_ = lv_layer->max_value_state_->get();
          lv_layer_scene_item->display_min_ = lv_layer->display_min_value_state_->get();
          lv_layer_scene_item->display_max_ = lv_layer->display_max_value_state_->get();
          lv_layer_scene_item->color_ = lv_layer->color_state_->get();
		  lv_layer_scene_item->colormap_ = lv_layer->colormap_state_->index();
          if (lv_layer_scene_item->display_min_ > lv_layer_scene_item->display_max_)
          {
            std::swap(lv_layer_scene_item->display_min_, lv_layer_scene_item->display_max_);
          }
        }
        break;
      default:
        CORE_THROW_LOGICERROR("Unknow layer type");
        break;
      } // end switch

      layer_scene_item->layer_id_ = layer->get_layer_id();
      layer_scene_item->layer_ = layer;
      layer_scene_item->opacity_ = layer->opacity_state_->get();
      layer_scene_item->grid_transform_ = layer->get_grid_transform();

      layer_scene->push_back( layer_scene_item );
    } // end for each layer

  } // end for each group

  return layer_scene;
}

Core::BBox LayerManager::get_layers_bbox()
{
  // NOTE: This functions is called from the Rendering Thread
  // Lock the LayerManager
  lock_type lock( this->get_mutex() );

  Core::BBox bbox;
  GroupList::iterator group_iterator = this->private_->group_list_.begin();
  for ( ; group_iterator != this->private_->group_list_.end(); group_iterator++)
  {
    LayerGroupHandle group = *group_iterator;
    const Core::GridTransform& grid_trans = group->get_grid_transform();
    Core::Point pt( -0.5, -0.5, -0.5 );
    bbox.extend( grid_trans * pt );
    pt = Core::Point( static_cast< double >( grid_trans.get_nx() - 0.5 ), 
      static_cast< double >( grid_trans.get_ny() - 0.5 ), 
      static_cast< double >( grid_trans.get_nz() - 0.5 ) );
    bbox.extend( grid_trans * pt );
  }

  return bbox;
}

void LayerManager::get_layer_names( std::vector< LayerIDNamePair >& layer_names, int type )
{
  lock_type lock( this->get_mutex() );

  std::vector< LayerHandle > layers;
  LayerManager::Instance()->get_layers( layers );
  size_t num_of_layers = layers.size();
  for ( size_t i = 0; i < num_of_layers; i++ )
  {
    // NOTE: Only if a layer is valid do we save it in a session. An example of an invalid
    // layer is for instance a layer that was just created, but hasn't finished processing
    // its data.
    if ( layers[ i ]->has_valid_data() && ( layers[ i ]->get_type() & type ) )
    {
      layer_names.push_back( std::make_pair( layers[ i ]->get_layer_id(), 
        layers[ i ]->get_layer_name() ) );
    }
  }
}

size_t LayerManager::get_group_position( LayerGroupHandle group )
{
  ASSERT_IS_APPLICATION_THREAD();

  GroupList::const_iterator it =  this->private_->group_list_.begin();
  GroupList::const_iterator it_end = this->private_->group_list_.end();
  size_t position = 0;
  while ( it != it_end && ( *it ) != group )
  {
    ++it;
    ++position;
  }

  if ( it == it_end )
  {
    assert( false );
    CORE_THROW_LOGICERROR( "Group no longer exists in LayerManager" );
  }

  return position;
}

void LayerManager::undelete_layers( const std::vector< LayerHandle >& layers, 
                   const std::vector< size_t >& group_pos, 
                   const std::vector< size_t >& layer_pos )
{
  ASSERT_IS_APPLICATION_THREAD();
  assert( layers.size() == group_pos.size() && layers.size() == layer_pos.size() );

  bool new_active_layer = false;
  std::set< LayerGroupHandle > new_groups;
  std::set< LayerGroupHandle > changed_groups;

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    for ( size_t i = 0; i < layers.size(); ++i )
    {
      LayerHandle layer = layers[ i ];
      LayerGroupHandle layer_group = layer->get_layer_group();
      GroupList::iterator group_it = std::find( 
        this->private_->group_list_.begin(),
        this->private_->group_list_.end(), layer_group );
      if ( group_it == this->private_->group_list_.end() )
      {
        if ( this->private_->group_list_.size() > group_pos[ i ] )
        {
          group_it = this->private_->group_list_.begin();
          std::advance( group_it, group_pos[ i ] );
        }
        this->private_->group_list_.insert( group_it, layer_group );
        new_groups.insert( layer_group );
      }
      else if ( new_groups.find( layer_group ) == new_groups.end() )
      {
        changed_groups.insert( layer_group );
      }

      layer_group->insert_layer( layer, layer_pos[ i ] );
    }

    if ( !this->private_->active_layer_ )
    {
      this->private_->active_layer_ = layers[ 0 ];
      new_active_layer = true;
    }
  }

  std::set< LayerHandle > layer_set;
  layer_set.insert( layers.begin(), layers.end() );

  // For each group that was added
  BOOST_FOREACH( const LayerGroupHandle& layer_group, new_groups )
  {
    // For the first layer in the group, the layer_inserted_signal_
    // needs to be triggered with the second parameter set to true
    LayerList& layer_list = layer_group->get_layer_list();
    layer_set.erase( layer_list.front() );
    this->layer_inserted_signal_( layer_list.front(), true );
  }

  // For the rest of the layers, trigger layer_inserted_signal_ with the second
  // parameter set to false
  BOOST_FOREACH( const LayerHandle& layer, layer_set )
  {
    this->layer_inserted_signal_( layer, false );
  }

  this->layers_changed_signal_();

  // Signal the new active layer
  if ( new_active_layer )
  {
    this->active_layer_changed_signal_( layers[ 0 ] );
  }
}

SandboxID LayerManager::create_sandbox()
{
  ASSERT_IS_APPLICATION_THREAD();
  lock_type lock( this->get_mutex() );
  SandboxID sandbox_id = this->private_->sandbox_count_++;
  this->private_->sandboxes_[ sandbox_id ] = LayerSandboxHandle( new LayerSandbox );
  lock.unlock();
  this->sandbox_created_signal_( sandbox_id );
  return sandbox_id;
}

bool LayerManager::create_sandbox( SandboxID sandbox )
{
  ASSERT_IS_APPLICATION_THREAD();
  lock_type lock( this->get_mutex() );

  // Sandbox ID must be non-negative, and should not exist yet
  if ( sandbox < 0 || this->private_->sandboxes_.count( sandbox ) != 0 )
  {
    return false;
  }
  
  this->private_->sandboxes_[ sandbox ] = LayerSandboxHandle( new LayerSandbox );
  lock.unlock();
  this->sandbox_created_signal_( sandbox );
  return true;
}

bool LayerManager::delete_sandbox( SandboxID sandbox )
{
  ASSERT_IS_APPLICATION_THREAD();
  lock_type lock( this->get_mutex() );
  // Invalidate all the sandbox layers
  // TODO: Ideally, at this point, the sandbox should be the only one that still has
  // handles to those layers, and thus the invalidation shouldn't be necessary.
  // However, some of the layers are still being referenced from somewhere else,
  // causing the layers to persist after the sandbox is destroyed.
  // We need to fix this.
  LayerSandboxHandle layer_sandbox = this->private_->find_sandbox( sandbox );
  if ( layer_sandbox )
  {
    for ( LayerSandbox::iterator it = layer_sandbox->begin();
      it != layer_sandbox->end(); ++it )
    {
      // Abort any filters that are still running on the layer
      it->second->abort_signal_();
      // Invalidate the layer
      it->second->invalidate();
    }
  }
  
  if ( this->private_->sandboxes_.erase( sandbox ) == 1 )
  {
    lock.unlock();
    this->sandbox_deleted_signal_( sandbox );
    return true;
  }
  
  return  false;
}

bool LayerManager::is_sandbox( SandboxID sandbox_id )
{
  ASSERT_IS_APPLICATION_THREAD();
  return ( sandbox_id == -1 ||
    this->private_->sandboxes_.count( sandbox_id ) == 1 );
}

bool LayerManager::pre_save_states( Core::StateIO& state_io )
{
  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
  if ( current_project && current_project->get_need_anonymize() )
  {
    LayerMetaData empty_metadata;

    for( GroupList::const_iterator git = 
      this->private_->group_list_.begin(); git != this->private_->group_list_.end(); ++git )
    {
      (*git)->set_meta_data( empty_metadata );
    
      LayerList& layer_list = ( *git )->get_layer_list();
      LayerList::iterator it = layer_list.begin();
      LayerList::iterator it_end = layer_list.end();
      
      while ( it != it_end )
      {
        (*it)->set_meta_data( empty_metadata );
        ++it;
      }
    }
  }

  current_project->set_need_anonymize( false );
  
  return true;
}

bool LayerManager::post_save_states( Core::StateIO& state_io )
{
  TiXmlElement* lm_element = state_io.get_current_element();
  assert( this->get_statehandler_id() == lm_element->Value() );
  
  TiXmlElement* groups_element = new TiXmlElement( "groups" );
  lm_element->LinkEndChild( groups_element );
  
  state_io.push_current_element();
  state_io.set_current_element( groups_element );
  
  bool succeeded = true;
  for( GroupList::reverse_iterator i = 
    this->private_->group_list_.rbegin(); i != this->private_->group_list_.rend(); ++i )
  {
    if( ( *i )->has_a_valid_layer() )
    {
      succeeded = succeeded && ( *i )->save_states( state_io );
    }
  }

  state_io.pop_current_element();

  return succeeded;
} 
  
bool LayerManager::pre_load_states( const Core::StateIO& state_io )
{
  // Make sure that the group list is empty.
  // NOTE: The application should have been properly reset before loading a session.
  assert( this->private_->group_list_.empty() );

  // Load the layers from session
  const TiXmlElement* groups_element = state_io.get_current_element()->
    FirstChildElement( "groups" );
  if ( groups_element == 0 )
  {
    return false;
  }

  state_io.push_current_element();
  state_io.set_current_element( groups_element );

  bool succeeded = true;
  const TiXmlElement* group_element = groups_element->FirstChildElement();
  while ( group_element != 0 )
  {
    std::string group_id( group_element->Value() );
    LayerGroupHandle group( new LayerGroup( group_id ) );

    if ( group->load_states( state_io ) )
    {
      this->private_->group_list_.push_front( group );

      const LayerList& layer_list = group->get_layer_list();
      LayerList::const_iterator it = layer_list.begin();
      bool first = true;
      for ( ; it != layer_list.end(); it++ )
      {
        // Connect to the value_changed_signal of layer name
        // NOTE: LayerManager will always out-live layers, so it's safe to not disconnect.
        ( *it )->name_state_->value_changed_signal_.connect( boost::bind(
          &LayerManagerPrivate::handle_layer_name_changed, this->private_, ( *it )->get_layer_id(), _2 ) );
          
        // NOTE: Add a connection here to check when layer data state changes
        // This is need to switch on/off menu options in the interface
        ( *it )->data_state_->state_changed_signal_.connect( boost::bind(
          &LayerManagerPrivate::handle_layer_data_changed, this->private_, LayerWeakHandle( *it ) ) );

        this->layer_inserted_signal_( ( *it ), first );
        first = false;
      }
    }
    else
    {
      succeeded = false;
    }

    group_element = group_element->NextSiblingElement();
  }

  state_io.pop_current_element();

  this->layers_changed_signal_();

  return succeeded;
}

bool LayerManager::post_load_states( const Core::StateIO& state_io )
{ 
  // If there are layers loaded, restore the active layer state
  if ( this->private_->group_list_.size() > 0 )
  {
    LayerHandle active_layer = this->find_layer_by_id( this->active_layer_state_->get() );
    if ( !active_layer )
    {
      CORE_LOG_WARNING( "Incorrect active layer state loaded from session" );
      active_layer = this->private_->group_list_.front()->top_layer();  
    }
    Core::ScopedCounter signal_block( this->private_->signal_block_count_ );
    this->set_active_layer( active_layer );
  }
  
  return true;
}


// == static functions ==

bool LayerManager::CheckSandboxExistence( SandboxID sandbox, Core::ActionContextHandle context )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckSandboxExistence can only be called from the"
      " application thread." );
  }

  // Check whether the sandbox exists
  if ( !LayerManager::Instance()->is_sandbox( sandbox ) )
  {
    context->report_error( Core::ExportToString( sandbox ) + " is not a valid sandbox ID." );
    return false;
  }
  
  return true;
}

bool LayerManager::CheckLayerExistence( const std::string& layer_id, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistence can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  if ( !( LayerManager::Instance()->find_layer_by_id( layer_id, sandbox ) ) )
  {
    return false;
  }

  return true;
}

bool LayerManager::CheckLayerExistence( const std::string& layer_id, 
  Core::ActionContextHandle context, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistence can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  if ( !( LayerManager::Instance()->find_layer_by_id( layer_id, sandbox ) ) )
  {
    context->report_error( std::string( "Incorrect layerid: layer '") + layer_id + 
      "' does not exist." );
    return false;
  }

  return true;
}


bool LayerManager::CheckGroupExistence( const std::string& group_id, 
  std::string& error )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckGroupExistence can only be called from the"
      " application thread." );
  }
  
  // Clear error string
  error = "";

  // Check whether layer exists
  if ( !( LayerManager::Instance()->find_group( group_id ) ) )
  {
    error = std::string( "Incorrect groupid: group '") + group_id + "' does not exist.";
    return false;
  }

  return true;
}


bool LayerManager::CheckLayerExistenceAndType( const std::string& layer_id, Core::VolumeType type, 
    Core::ActionContextHandle context, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistenceAndType can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id, sandbox );
  if ( !layer )
  {
    context->report_error( std::string( "Incorrect layerid: layer '") + layer_id + 
      "' does not exist." );
    return false;
  }

  // Check whether the type of the layer is correct
  if ( layer->get_type() != type )
  {
    if ( type == Core::VolumeType::DATA_E )
    {
      context->report_error( std::string( "Layer '") + layer_id + 
        "' is not a data layer." );
    }
    else if ( type == Core::VolumeType::MASK_E )
    {
      context->report_error( std::string( "Layer '") + layer_id + 
        "' is not a mask layer." );
    }
    else if ( type == Core::VolumeType::LABEL_E )
    {
      context->report_error( std::string( "Layer '") + layer_id + 
        "' is not a label layer." );
    }
    else
    {
      context->report_error( std::string( "Layer '") + layer_id + 
        "' is of an incorrect type." );
    }
    return false;
  }
  
  return true;
}

bool LayerManager::CheckLayerSize( const std::string& layer_id1, const std::string& layer_id2,
    Core::ActionContextHandle context, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerSize can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  LayerHandle layer1 = LayerManager::Instance()->find_layer_by_id( layer_id1, sandbox );
  if ( !layer1 )
  {
    context->report_error( std::string( "Incorrect layerid: layer '") + layer_id1 + 
      "' does not exist." );
    return false;
  }

  LayerHandle layer2 = LayerManager::Instance()->find_layer_by_id( layer_id2, sandbox );
  if ( !layer2 )
  {
    context->report_error( std::string( "Incorrect layerid: layer '") + layer_id2 + 
      "' does not exist." );
    return false;
  }
  
  if ( layer1->get_grid_transform() != layer2->get_grid_transform() )
  {
    context->report_error( std::string( "Layer '" ) + layer_id1 + "' and layer '" + layer_id2 + 
      "' are not of the same size and origin." );
    return false;
  }
  
  return true;
}

bool LayerManager::CheckLayerAvailabilityForProcessing( const std::string& layer_id, 
    Core::ActionContextHandle context, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerAvailabilityForProcessing can only be called from the"
      " application thread." );
  }
  
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id, sandbox );
  // Check whether layer exists
  if ( !layer )
  {
    CORE_THROW_LOGICERROR( "Layer does not exist, please check existence "
      "before availability" );
  }

  std::string layer_state = layer->data_state_->get();
  bool lock_state = layer->locked_state_->get();
  if ( layer_state == Layer::AVAILABLE_C && lock_state == false )
  {
    return true;
  }
  else
  {
    // This notifier will inform the calling process when the layer will be available again.
    Core::NotifierHandle notifier = Core::NotifierHandle( 
      new LayerAvailabilityNotifier( layer ) );
    context->report_need_resource( notifier );

    return false; 
  }
}

bool LayerManager::CheckLayerAvailabilityForUse( const std::string& layer_id, 
    Core::ActionContextHandle context, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerAvailabilityForUse can only be called from the"
      " application thread." );
  }
  
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id, sandbox );
  // Check whether layer exists
  if ( !layer )
  {
    CORE_THROW_LOGICERROR( "Layer does not exist, please check existence "
      "before availability" );
  }

  std::string layer_state = layer->data_state_->get();
  bool lock_state = layer->locked_state_->get();

  if ( ( layer_state == Layer::AVAILABLE_C || layer_state == Layer::IN_USE_C ) 
    && ( lock_state == false ) )
  {
    return true;
  }
  else
  {
    // This notifier will inform the calling process when the layer will be available again.
    Core::NotifierHandle notifier = Core::NotifierHandle( 
      new LayerAvailabilityNotifier( layer ) );
    context->report_need_resource( notifier );

    return false; 
  }
}

bool LayerManager::CheckLayerAvailability( const std::string& layer_id, bool replace,
    Core::ActionContextHandle context, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerAvailability can only be called from the"
      " application thread." );
  }

  if ( replace )
  {
    return LayerManager::CheckLayerAvailabilityForProcessing( layer_id, context, sandbox );
  }
  else
  {
    return LayerManager::CheckLayerAvailabilityForUse( layer_id, context, sandbox );
  }
}

LayerHandle LayerManager::FindLayer( const std::string& layer_id, SandboxID sandbox )
{
  return LayerManager::Instance()->find_layer_by_id( layer_id, sandbox );
}

LayerHandle LayerManager::FindLayer( ProvenanceID prov_id, SandboxID sandbox )
{
  return LayerManager::Instance()->find_layer_by_provenance_id( prov_id, sandbox );
}

LayerGroupHandle LayerManager::FindGroup( const std::string& group_id )
{
  return LayerManager::Instance()->find_group( group_id );
}

LayerGroupHandle LayerManager::FindGroup( ProvenanceID prov_id )
{
  return LayerManager::Instance()->find_group( prov_id );
}


MaskLayerHandle LayerManager::FindMaskLayer( const std::string& layer_id, SandboxID sandbox )
{
  return boost::dynamic_pointer_cast<MaskLayer>(  
    LayerManager::Instance()->find_layer_by_id( layer_id, sandbox ) );
}

DataLayerHandle LayerManager::FindDataLayer( const std::string& layer_id, SandboxID sandbox )
{
  return boost::dynamic_pointer_cast<DataLayer>(  
    LayerManager::Instance()->find_layer_by_id( layer_id, sandbox ) );
}

bool LayerManager::LockForUse( LayerHandle layer, filter_key_type key )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "LockForUse can only be called from the"
      " application thread." );
  }
  
  // Check whether the layer is really available
  if ( layer->data_state_->get() != Layer::AVAILABLE_C &&
     layer->data_state_->get() != Layer::IN_USE_C ) return false;

  // If it is available set it to processing and list the key used by the filter
  if ( layer->data_state_->get() == Layer::AVAILABLE_C )
  {
    layer->data_state_->set( Layer::IN_USE_C );
  }
  
  // Add the key so we can could all the keys to see when the layer needs to be unlocked
  layer->add_filter_key( key );

  return true;
}

bool LayerManager::LockForProcessing( LayerHandle layer, filter_key_type key )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "LockForProcessing can only be called from the"
      " application thread." );
  }

  // Check whether the layer is really available
  if ( layer->data_state_->get() != Layer::AVAILABLE_C ) return false;

  // If it is available set it to processing and list the key used by the filter
  layer->data_state_->set( Layer::PROCESSING_C );
  layer->add_filter_key( key );

  return true;
}

bool LayerManager::CreateAndLockMaskLayer( Core::GridTransform transform, const std::string& name, 
    LayerHandle& layer, const LayerMetaData& meta_data, filter_key_type key, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane.
  // Only the Application Thread guarantees that nothing is changed in the program.
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CreateAndLockMaskLayer can only be called from the"
      " application thread." );
  }

  // NOTE: We create a mask without data associated with it. Only a skeleton of a 
  // layer needs to be created.
  Core::MaskVolumeHandle invalid_mask;
  if ( ! ( Core::MaskVolume::CreateInvalidMask( transform, invalid_mask ) ) )
  {
    return false;
  }

  // Wrap the Layer structure around the mask data.
  layer = LayerHandle( new MaskLayer( name, invalid_mask ) );

  // Insert the key used to keep track of which process is using this layer
  layer->add_filter_key( key );

  // Meta data id
  layer->set_meta_data( meta_data );
  
  // Insert the layer into the layer manager.
  LayerManager::Instance()->insert_layer( layer, sandbox );
  
  return true;
}

bool LayerManager::CreateAndLockDataLayer( Core::GridTransform transform, const std::string& name, 
  LayerHandle& layer, const LayerMetaData& meta_data, filter_key_type key, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CreateAndLockDataLayer can only be called from the"
      " application thread." );
  }

  // NOTE: We create a datavolume without data associated with it. Only a skeleton of a 
  // layer needs to be created.
  Core::DataVolumeHandle invalid_data;
  if ( ! ( Core::DataVolume::CreateInvalidData( transform, invalid_data ) ) )
  {
    return false;
  }
  
  // Wrap the Layer structure around the volume data.
  layer = LayerHandle( new DataLayer( name, invalid_data ) );

  // Insert the key used to keep track of which process is using this layer
  layer->add_filter_key( key );

  // Meta data id
  layer->set_meta_data( meta_data );

  // Insert the layer into the layer manager.
  LayerManager::Instance()->insert_layer( layer, sandbox );
  
  return true;
}

bool LayerManager::CreateCroppedLargeVolumeLayer( Core::LargeVolumeSchemaHandle schema,
  const Core::GridTransform& crop_trans, const std::string& name,
  LayerHandle& layer, const LayerMetaData& meta_data, SandboxID sandbox )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !Core::Application::IsApplicationThread() )
  {
    CORE_THROW_LOGICERROR( "CreateAndLockDataLayer can only be called from the"
      " application thread." );
  }

  layer.reset( new LargeVolumeLayer( name, schema, crop_trans ) );
  layer->set_meta_data( meta_data );

  LayerManager::Instance()->insert_layer( layer, sandbox );

  return true;
}

void LayerManager::DispatchDeleteLayer( LayerHandle layer, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchDeleteLayer, 
      layer, key, sandbox ) );
    return;
  }

  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    // The filter should not have access to this one any more
    layer->remove_filter_key( key );
    
    if ( layer->num_filter_keys() == 0 )
    {
      layer->reset_filter_handle();
      layer->reset_allow_stop();
      // Unlock the layer before deleting it, so when it's undeleted the data state is correct
      layer->data_state_->set( Layer::AVAILABLE_C );
      // Delete the layer from the layer manager.
      LayerManager::Instance()->delete_layer( layer, sandbox );
    }
    else
    {
      CORE_THROW_LOGICERROR( "Could not delete layer that is connected to another filter" );
    }
  }
}

void LayerManager::DispatchUnlockLayer( LayerHandle layer, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchUnlockLayer, 
      layer, key, sandbox ) );
    return;
  }

  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    // The filter should not have access to this one any more
    layer->remove_filter_key( key );
    
    if ( layer->num_filter_keys() == 0 )
    {
      layer->data_state_->set( Layer::AVAILABLE_C );
      layer->reset_filter_handle();
      layer->reset_allow_stop();
    }
  }
}

void LayerManager::DispatchUnlockOrDeleteLayer( LayerHandle layer, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchUnlockOrDeleteLayer, 
      layer, key, sandbox ) );
    return;
  }

  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    // The filter should not have access to this one any more
    layer->remove_filter_key( key );
      
    // NOTE: We can only determine whether the layer is valid on the application thread.
    // Other instructions to insert data into the layer may still be in the application thread
    // queue, hence we need to process this at the end of the queue.
    if( layer->has_valid_data() )
    {
      if ( layer->num_filter_keys() == 0 )
      {
        layer->data_state_->set( Layer::AVAILABLE_C );
        layer->reset_filter_handle();
        layer->reset_allow_stop();
      }
    }
    else
    {
      if ( layer->num_filter_keys() != 0 )
      {
        CORE_THROW_LOGICERROR( "Could not delete layer as it is connected to another filter" );
      }
      // NOTE: Unlock the layer before deleting it so any thing waiting for it can be waken up.
      // This is crucial for scripts running in a sandbox because deleting layers from a sandbox
      // won't trigger the layers_deleted_signal_.
      layer->data_state_->set( Layer::AVAILABLE_C );
      layer->reset_filter_handle();
      layer->reset_allow_stop();

      LayerManager::Instance()->delete_layer( layer, sandbox );
    }
  }
}

void LayerManager::DispatchInsertDataVolumeIntoLayer( DataLayerHandle layer, 
  Core::DataVolumeHandle data, ProvenanceID prov_id, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchInsertDataVolumeIntoLayer,
      layer, data, prov_id, key, sandbox ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    if ( layer->set_data_volume( data ) )
    {
      layer->provenance_id_state_->set( prov_id );
      // Only trigger signals if not in a sandbox
      if ( sandbox == -1 )
      {
        LayerManager::Instance()->layer_volume_changed_signal_( layer );
        LayerManager::Instance()->layers_changed_signal_();
      }
    }
  }
}

void LayerManager::DispatchInsertMaskVolumeIntoLayer( MaskLayerHandle layer, 
  Core::MaskVolumeHandle mask, ProvenanceID prov_id, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertMaskVolumeIntoLayer, layer, mask, prov_id, key, sandbox ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    if ( layer->set_mask_volume( mask ) )
    {
      layer->provenance_id_state_->set( prov_id );
      // Only trigger signals if not in a sandbox
      if ( sandbox == -1 )
      {
        LayerManager::Instance()->layer_volume_changed_signal_( layer );
        LayerManager::Instance()->layers_changed_signal_();
      }   
    }
  }
}

void LayerManager::DispatchInsertVolumeIntoLayer( LayerHandle layer, 
  Core::VolumeHandle volume, ProvenanceID prov_id, filter_key_type key, SandboxID sandbox )
{
  if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    MaskLayerHandle mask = boost::dynamic_pointer_cast<MaskLayer>( layer );
    Core::MaskVolumeHandle mask_volume = boost::dynamic_pointer_cast<Core::MaskVolume>( volume );
    if ( mask && mask_volume )
    {
      DispatchInsertMaskVolumeIntoLayer( mask, mask_volume, prov_id, key, sandbox );
    }
  }
  else if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    DataLayerHandle data = boost::dynamic_pointer_cast<DataLayer>( layer );
    Core::DataVolumeHandle data_volume = boost::dynamic_pointer_cast<Core::DataVolume>( volume );
    if ( data && data_volume )
    {
      DispatchInsertDataVolumeIntoLayer( data, data_volume, prov_id, key, sandbox );
    }
  }
}

void LayerManager::DispatchInsertDataSliceIntoLayer( DataLayerHandle layer,
    Core::DataSliceHandle data,  ProvenanceID prov_id, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertDataSliceIntoLayer, layer, data, prov_id, key, sandbox ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    Core::DataVolumeHandle data_volume = layer->get_data_volume();
    if ( !data_volume ) return;

    if ( data_volume->insert_slice( data ) )
    {
      layer->provenance_id_state_->set( prov_id );
      // Only trigger signals if not in a sandbox
      if ( sandbox == -1 )
      {
        LayerManager::Instance()->layer_volume_changed_signal_( layer );
        LayerManager::Instance()->layers_changed_signal_();
      }
    }
  }
}

void LayerManager::DispatchInsertDataSlicesIntoLayer( DataLayerHandle layer,
    std::vector<Core::DataSliceHandle> data, ProvenanceID prov_id, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertDataSlicesIntoLayer, layer, data, prov_id, key, sandbox ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    Core::DataVolumeHandle data_volume = layer->get_data_volume();
    if ( !data_volume ) return;

    std::vector<Core::DataSliceHandle>::iterator it = data.begin();
    std::vector<Core::DataSliceHandle>::iterator it_end = data.end();
  
    while( it != it_end )
    {
      data_volume->insert_slice( (*it) );
      ++it; 
    }
  
    layer->provenance_id_state_->set( prov_id );
    if ( sandbox == -1 )
    {
      LayerManager::Instance()->layer_volume_changed_signal_( layer );
      LayerManager::Instance()->layers_changed_signal_();
    }
  }
}


void LayerManager::DispatchInsertMaskSliceIntoLayer(MaskLayerHandle layer,
    Core::MaskDataSliceHandle mask, ProvenanceID prov_id, filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertMaskSliceIntoLayer, layer, mask, prov_id, key, sandbox ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    Core::MaskVolumeHandle mask_volume = layer->get_mask_volume();
    if ( !mask_volume ) return;
  
    if ( mask_volume->insert_slice( mask ) )
    {
      layer->provenance_id_state_->set( prov_id );
      if ( sandbox == -1 )
      {
        LayerManager::Instance()->layer_volume_changed_signal_( layer );
        LayerManager::Instance()->layers_changed_signal_();
      }   
    }
  }
}

void LayerManager::DispatchInsertMaskSlicesIntoLayer( MaskLayerHandle layer,
    std::vector<Core::MaskDataSliceHandle> mask, ProvenanceID prov_id, 
    filter_key_type key, SandboxID sandbox )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertMaskSlicesIntoLayer, layer, mask, prov_id, key, sandbox ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    Core::MaskVolumeHandle mask_volume = layer->get_mask_volume();
    if ( !mask_volume ) return;
    
    std::vector<Core::MaskDataSliceHandle>::iterator it = mask.begin();
    std::vector<Core::MaskDataSliceHandle>::iterator it_end = mask.end();
  
    while( it != it_end )
    {
      mask_volume->insert_slice( (*it) );
      ++it; 
    }
  
    layer->provenance_id_state_->set( prov_id );
    if ( sandbox == -1 )
    {
      LayerManager::Instance()->layer_volume_changed_signal_( layer );
      LayerManager::Instance()->layers_changed_signal_();
    }
  }
}

LayerManager::id_count_type LayerManager::GetLayerIdCount()
{
  id_count_type id_count;
  id_count.resize( 3 );

  id_count[ 0 ] = static_cast<int>( 
    Core::StateEngine::Instance()->get_next_statehandler_count( "layer" ) );

  id_count[ 1 ] = static_cast<int>( 
    Core::StateEngine::Instance()->get_next_statehandler_count( "group" ) );

  id_count[ 2 ] = static_cast<int>(
    MaskLayer::GetColorCount() );

  return id_count;
}

LayerManager::id_count_type LayerManager::GetLayerInvalidIdCount()
{
  id_count_type id_count;
  id_count.resize( 3, -1 );

  return id_count;
}

void LayerManager::SetLayerIdCount( id_count_type id_count )
{
  if ( id_count.size() > 0 && id_count[ 0 ] >= 0 )
  {
    Core::StateEngine::Instance()->set_next_statehandler_count( "layer", id_count[ 0 ] );
  }
  if ( id_count.size() > 1 && id_count[ 1 ] >= 0 )
  {
    Core::StateEngine::Instance()->set_next_statehandler_count( "group", id_count[ 1 ] );
  }
  if ( id_count.size() > 2 && id_count[ 2 ] >= 0 )
  {
    MaskLayer::SetColorCount( id_count[ 2 ] );
  }
}

} // end namespace seg3D
