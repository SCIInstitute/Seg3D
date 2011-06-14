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
#include <boost/lexical_cast.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/DataBlockManager.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/LayerManager/LayerScene.h>
#include <Application/LayerManager/LayerAvailabilityNotifier.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class LayerManagerPrivate
//////////////////////////////////////////////////////////////////////////

class LayerManagerPrivate
{
public:
  void update_layer_list();
  void handle_active_layer_state_changed( std::string layer_id );
  void handle_active_layer_changed();
  void reset();

  size_t signal_block_count_;
  
  typedef std::list < LayerGroupHandle > group_list_type;
  group_list_type group_list_;
  LayerHandle active_layer_;
  LayerManager* layer_manager_;
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

  this->layer_manager_->set_active_layer( this->layer_manager_->get_layer_by_id( layer_id ) );
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
  LayerManagerPrivate::group_list_type::iterator group_iterator = this->group_list_.begin();
  while ( group_iterator != this->group_list_.end() )
  {
    ( *group_iterator )->clear();
    ( *group_iterator )->invalidate();
    ++group_iterator;
  }
  this->active_layer_.reset();
  this->group_list_.clear();
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
  
bool LayerManager::insert_layer( LayerHandle layer )
{
  bool active_layer_changed = false;
  bool new_group = false;
  LayerGroupHandle group_handle;
  
  {
    lock_type lock( this->get_mutex() );
    
    CORE_LOG_MESSAGE( std::string("Insert New Layer: ") + layer->get_layer_id());
    
    // if its a mask, lets reset its color to one that doesn't exist if possible
    if( layer->get_type() == Core::VolumeType::MASK_E ) 
    {
      int mask_color = this->find_free_color();
      if( mask_color != -1 )
      { 
        boost::dynamic_pointer_cast< MaskLayer >( layer )->color_state_->set( mask_color );
      } 
    }
    
    
        
    for ( LayerManagerPrivate::group_list_type::iterator it = this->private_->group_list_.begin(); 
       it != this->private_->group_list_.end(); ++it )
    {
      // for testing 
      Core::GridTransform layer_grid_transform = layer->get_grid_transform();
      Core::GridTransform group_grid_transform = ( *it )->get_grid_transform();
                  
      if ( layer_grid_transform == group_grid_transform ) 
      {
        group_handle = *it;
        break;
      }
    }

    if ( !group_handle )  
    {
      new_group = true;
      group_handle = LayerGroupHandle( new LayerGroup(  layer->get_grid_transform(),
        layer->provenance_id_state_->get(), layer->get_meta_data() ) );
      this->private_->group_list_.push_front( group_handle );
      
      CORE_LOG_DEBUG( std::string( "Set Active Layer: " ) + layer->get_layer_id());

      if ( layer->has_valid_data() )
      {
        this->private_->active_layer_ = layer;
        active_layer_changed = true;
      } 
    }
    
    group_handle->insert_layer( layer );
      
    // Connect to the value_changed_signal of layer name
    // NOTE: LayerManager will always out-live layers, so it's safe to not disconnect.
    layer->name_state_->value_changed_signal_.connect( boost::bind(
      &LayerManager::handle_layer_name_changed, this, layer->get_layer_id(), _2 ) );
      
    // NOTE: Add a connection here to check when layer data state changes
    // This is need to switch on/off menu options in the interface
    layer->data_state_->state_changed_signal_.connect( boost::bind(
      &LayerManager::handle_layer_data_changed, this, layer ) );
        
  } // unlocked from here

  CORE_LOG_DEBUG( std::string( "Signalling that new layer was inserted" ) );
  CORE_LOG_DEBUG( std::string( "--- triggering signals ---" ) );

  if ( new_group )
  {
    this->groups_changed_signal_();
  }
  else
  {
    this->group_internals_changed_signal_( group_handle );
  }

  // This is no longer needed by the LayerManager Widget  
  this->layer_inserted_signal_( layer );
  this->layers_changed_signal_();
  
  if( active_layer_changed )
  {
    active_layer_changed_signal_( layer );
  }
  
  return true;
}

bool LayerManager::check_for_same_group( const std::string layer_to_insert_id, 
  const std::string layer_below_id )
{
  lock_type lock( this->get_mutex() );
  LayerGroupHandle top_group = this->get_layer_by_id( layer_to_insert_id )->get_layer_group();
  LayerGroupHandle bottom_group = this->get_layer_by_id( layer_below_id )->get_layer_group();
  
  return ( top_group == bottom_group );
}

bool LayerManager::move_group_above( std::string group_to_move_id, std::string group_below_id )
{
  {
    // Get the Lock
    lock_type lock( this->get_mutex() );

    LayerGroupHandle group_above = this->get_group_by_id( group_to_move_id );
    LayerGroupHandle group_below = this->get_group_by_id( group_below_id );

    if( ( !group_above || !group_below ) || ( group_above == group_below ) )
      return false;

    this->private_->group_list_.remove( group_above );
    this->insert_group( group_above, group_below );
  }

  this->groups_changed_signal_();
  this->layers_changed_signal_();
  this->layers_reordered_signal_();

  return true;
}

void LayerManager::insert_group( LayerGroupHandle group_above, LayerGroupHandle group_below )
{
  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    if( ( *i ) == group_below )
    {
      //we insert the layer
      this->private_->group_list_.insert( i, group_above );
    }
  }
}

bool LayerManager::move_layer_above( LayerHandle layer_to_move, LayerHandle target_layer )
{
  if( !layer_to_move || !target_layer ) return false;
  LayerGroupHandle layer_group = layer_to_move->get_layer_group();
  if ( layer_group != target_layer->get_layer_group() )
  {
    assert( false );
    CORE_LOG_ERROR( "Can't move layer between groups. Resampling required!" );
    return false;
  }

  {
    // Get the Lock
    lock_type lock( this->get_mutex() );
      
    // First we Delete the Layer from its list of layers
    layer_group->delete_layer( layer_to_move );
    layer_group->move_layer_above( layer_to_move, target_layer );
  } // We release the lock  here.

  this->group_internals_changed_signal_( layer_group );
  this->layers_changed_signal_();
  this->layers_reordered_signal_();
  
  return true;  
}

bool LayerManager::move_layer_below( const std::string& layer_id, const std::string& group_id )
{
  // we will need to keep track of a few things outside of the locked scope
  // This keeps track of whether or not we delete the group we are moving from
  bool group_above_has_been_deleted = false;
  bool layer_has_changed_groups = false;

  // These handles will let us send signals after we make the moves
  LayerGroupHandle from_group;
  LayerGroupHandle to_group;
  LayerHandle layer = this->get_layer_by_id( layer_id );

  {
    // Get the Lock
    lock_type lock( this->get_mutex() );

    from_group = this->get_layer_by_id( layer_id )->get_layer_group();
    to_group = this->get_group_by_id( group_id );

    // First we Delete the Layer from its list of layers
    from_group->delete_layer( layer );
    
    to_group->move_layer_below( layer );

    // If they are in the same group ---
    if( from_group != to_group )
    {
      // If the group we are removing the layer from is empty we remove it
      //  from the list of groups and signal the GUI
      if( from_group->get_layer_list().empty() )
      {   
        this->private_->group_list_.remove( from_group );
        group_above_has_been_deleted = true;
      }
      // Set the weak handle in the layer we've inserted to the proper group
      layer->set_layer_group( to_group );
      layer_has_changed_groups = true;
    }

    if( layer_has_changed_groups )
    {
      this->group_internals_changed_signal_( from_group );
      this->group_internals_changed_signal_( to_group );
    }
    else
    {
      this->group_internals_changed_signal_( from_group );
    }

    if( group_above_has_been_deleted )
    {
      this->groups_changed_signal_();
    } 
  } // We release the lock  here.

  this->layers_changed_signal_();
  this->layers_reordered_signal_();

  return true;    
}

// Here is the logic for inserting a layer
bool LayerManager::validate_layer_move( LayerHandle layer_above, LayerHandle layer_below )
{
  // Validate the most common move
  if( layer_above->get_type() == layer_below->get_type() )
    return true;
  
  return false;
}

void LayerManager::set_active_layer( LayerHandle layer )
{
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
  
void LayerManager::get_layers_in_order( std::vector< LayerHandle >& vector_of_layers )
{
  lock_type lock( this->get_mutex() ); 
  
  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    for( layer_list_type::iterator j = ( *i )->layer_list_.begin(); 
      j != ( *i )->layer_list_.end(); ++j )
    {
      vector_of_layers.push_back( *j );
    }
  }
}

void LayerManager::set_next_layer_active()
{
  std::vector< LayerHandle > layers;
  this->get_layers_in_order( layers );
  
  int potential_next_active = -1;
    
  for( int i = static_cast< int >( layers.size() ) - 1; i >= 0; --i )
  {
    if( layers[ i ] != this->private_->active_layer_ )
    {
      if( layers[ i ]->get_layer_group()->group_widget_expanded_state_->get() )
      {
        potential_next_active = i;
      }
    }
    else if( potential_next_active != -1 )
    {
      this->set_active_layer( layers[ potential_next_active ] );
      return;
    }
  }
  
  if( potential_next_active != -1 )
  {
    this->set_active_layer( layers[ potential_next_active ] );
    return;
  }
}

  
void LayerManager::set_previous_layer_active()
{
  std::vector< LayerHandle > layers;
  this->get_layers_in_order( layers );
  
  int potential_previous_active = -1;
  
  for( int i = 0; i < static_cast< int >( layers.size() ); ++i )
  {
    if( layers[ i ] != this->private_->active_layer_ )
    {
      if( layers[ i ]->get_layer_group()->group_widget_expanded_state_->get() )
      {
        potential_previous_active = i;
      }
    }
    else if( potential_previous_active != -1 )
    {
      this->set_active_layer( layers[ potential_previous_active ] );
      return;
    }
  }
  
  if( potential_previous_active != -1 )
  {
    this->set_active_layer( layers[ potential_previous_active ] );
    return;
  }
  
}

LayerGroupHandle LayerManager::get_group_by_id( std::string group_id )
{
    lock_type lock( this->get_mutex() );
    
  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    if (( *i )->get_group_id() == group_id ) 
    {
      return ( *i );
    }
  }
  return LayerGroupHandle();
}

LayerGroupHandle LayerManager::get_group_by_provenance_id( ProvenanceID prov_id )
{
    lock_type lock( this->get_mutex() );
    
  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    if (( *i )->provenance_id_state_->get() == prov_id ) 
    {
      return ( *i );
    }
  }
  return LayerGroupHandle();
}


LayerHandle LayerManager::get_layer_by_id( const std::string& layer_id )
{
  lock_type lock( this->get_mutex() );

  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    for( layer_list_type::iterator j = ( *i )->layer_list_.begin(); 
      j != ( *i )->layer_list_.end(); ++j )
    {
      if( ( *j )->get_layer_id() == layer_id )
      {
        return ( *j );
      }
    }
  }
  return LayerHandle();
}

LayerHandle LayerManager::get_layer_by_provenance_id( ProvenanceID provenance_id )
{
  lock_type lock( this->get_mutex() );

  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    for( layer_list_type::iterator j = ( *i )->layer_list_.begin(); 
      j != ( *i )->layer_list_.end(); ++j )
    {
      if( ( *j )->provenance_id_state_->get() == provenance_id )
      {
        return ( *j );
      }
    }
  }
  return LayerHandle();
}


DataLayerHandle LayerManager::get_data_layer_by_id( const std::string& layer_id )
{
  return boost::dynamic_pointer_cast<DataLayer>( get_layer_by_id( layer_id ) );
}

MaskLayerHandle LayerManager::get_mask_layer_by_id( const std::string& layer_id )
{
  return boost::dynamic_pointer_cast<MaskLayer>( get_layer_by_id( layer_id ) );
}

LayerHandle LayerManager::get_layer_by_name( const std::string& layer_name )
{
  lock_type lock( this->get_mutex() );

  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    for( layer_list_type::iterator j = ( *i )->layer_list_.begin(); 
      j != ( *i )->layer_list_.end(); ++j )
    {
      if( ( *j )->get_layer_name() == layer_name )
      {
        return ( *j );
      }
    }
  }
  return LayerHandle();
}

void LayerManager::get_groups( std::vector< LayerGroupHandle > &vector_of_groups )
{
    lock_type lock( this->get_mutex() );
    
  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    vector_of_groups.push_back( *i );
  } 
}

void LayerManager::get_layers( std::vector< LayerHandle > &vector_of_layers )
{
    lock_type lock( this->get_mutex() );
    
  for( LayerManagerPrivate::group_list_type::reverse_iterator i = 
    this->private_->group_list_.rbegin(); i != this->private_->group_list_.rend(); ++i )
  {
      for( layer_list_type::iterator j = ( *i )->layer_list_.begin(); 
    j != ( *i )->layer_list_.end(); ++j )
      {
          vector_of_layers.push_back( ( *j ) );
      }
  }
}

void LayerManager::get_layers_in_group( LayerGroupHandle group ,
  std::vector< LayerHandle > &vector_of_layers )
{
    lock_type lock( this->get_mutex() );
    
  for( layer_list_type::iterator j =  group->layer_list_.begin(); 
    j != group->layer_list_.end(); ++j )
  {
    vector_of_layers.push_back( ( *j ) );
  }
}

void LayerManager::delete_layers(  std::vector< LayerHandle > layers  )
{
  if ( layers.empty() ) return;
  
  bool active_layer_changed = false;
  bool group_deleted = false;
  
  std::vector<LayerHandle> deleted_layers;
  
  LayerGroupHandle group;
  
  { // start the lock scope
    lock_type lock( this->get_mutex() );  
    
    for( size_t i = 0; i < layers.size(); ++i )
    {
      if( !layers[ i ] ) continue;
      
      CORE_LOG_MESSAGE( std::string( "Deleting Layer: " ) + layers[ i ]->get_layer_id() );

      // NOTE: Layer invalidation has been moved to the LayerUndoBufferItem.
      // A layer will only be invalidated when the corresponding undo buffer item has been deleted.
      // This also applies to layer groups.
      //layer->invalidate();

      // Abort any filter that might be running on the layer
      //layer->abort_signal_();

      group = layers[ i ]->get_layer_group();
      group->delete_layer( layers[ i ] );
      
      if( group->is_empty() )
      {   
        //group->invalidate();
        this->private_->group_list_.remove( group );
      }

      if ( this->private_->active_layer_ == layers[ i ] )
      {
        this->private_->active_layer_.reset();
        active_layer_changed = true;
      }
      
      if ( group->is_empty() ) group_deleted = true;
      deleted_layers.push_back( layers[ i ] );
    }

    if ( active_layer_changed && this->private_->group_list_.size() > 0 )
    {
      this->private_->active_layer_ = this->private_->group_list_.front()->layer_list_.back();
    }
  } 
  
  // signal the listeners
  if( group_deleted )
  {   
    this->groups_changed_signal_();
  }
  else
  {
    group->gui_state_group_->clear_selection();
    this->group_internals_changed_signal_( group );
  }
  
  this->layers_changed_signal_();
  this->layers_deleted_signal_( deleted_layers );
  
  if ( active_layer_changed && this->private_->active_layer_ )
  {
    this->active_layer_changed_signal_( this->private_->active_layer_ );
  }
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
  LayerManagerPrivate::group_list_type::reverse_iterator group_iterator = 
    this->private_->group_list_.rbegin();
  for ( ; group_iterator != this->private_->group_list_.rend(); group_iterator++)
  {
    
    layer_list_type layer_list = ( *group_iterator )->get_layer_list();

    layer_list_type::reverse_iterator layer_iterator = layer_list.rbegin();
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
  LayerManagerPrivate::group_list_type::iterator group_iterator = this->private_->group_list_.begin();
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

void LayerManager::get_layer_names_from_group( LayerGroupHandle group,
  std::vector< LayerIDNamePair >& layer_names, int type )
{
  lock_type lock( this->get_mutex() );

  std::vector< LayerHandle > layers;
  LayerManager::Instance()->get_layers_in_group( group, layers );
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

bool LayerManager::pre_save_states( Core::StateIO& state_io )
{
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
  
  for( LayerManagerPrivate::group_list_type::reverse_iterator i = 
    this->private_->group_list_.rbegin(); i != this->private_->group_list_.rend(); ++i )
  {
    if( ( *i )->has_a_valid_layer() )
    {
      ( *i )->save_states( state_io );
    }
  }

  state_io.pop_current_element();

  // TODO: Need to check this logic, this most liky saves too much data
  return Core::MaskDataBlockManager::Instance()->save_data_blocks( 
    ProjectManager::Instance()->get_current_project()->get_project_data_path(),
    PreferencesManager::Instance()->compression_state_->get(),
    PreferencesManager::Instance()->compression_level_state_->get() );
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

  const TiXmlElement* group_element = groups_element->FirstChildElement();
  while ( group_element != 0 )
  {
    std::string group_id( group_element->Value() );
    LayerGroupHandle group( new LayerGroup( group_id ) );

    if ( group->load_states( state_io ) )
    {
      this->private_->group_list_.push_front( group );

      layer_list_type layer_list = group->get_layer_list();
      layer_list_type::iterator it = layer_list.begin();
      for ( ; it != layer_list.end(); it++ )
      {
        // Connect to the value_changed_signal of layer name
        // NOTE: LayerManager will always out-live layers, so it's safe to not disconnect.
        ( *it )->name_state_->value_changed_signal_.connect( boost::bind(
          &LayerManager::handle_layer_name_changed, this, ( *it )->get_layer_id(), _2 ) );
          
        // NOTE: Add a connection here to check when layer data state changes
        // This is need to switch on/off menu options in the interface
        ( *it )->data_state_->state_changed_signal_.connect( boost::bind(
          &LayerManager::handle_layer_data_changed, this, ( *it ) ) );

        this->layer_inserted_signal_( ( *it ) );
      }
    }

    group_element = group_element->NextSiblingElement();
  }

  state_io.pop_current_element();

  this->groups_changed_signal_();
  this->layers_changed_signal_();

  return true;
}

bool LayerManager::post_load_states( const Core::StateIO& state_io )
{ 
  // If there are layers loaded, restore the active layer state
  if ( this->private_->group_list_.size() > 0 )
  {
    LayerHandle active_layer = this->get_layer_by_id( this->active_layer_state_->get() );
    if ( !active_layer )
    {
      CORE_LOG_WARNING( "Incorrect active layer state loaded from session" );
      active_layer = this->private_->group_list_.front()->layer_list_.back(); 
    }
    Core::ScopedCounter signal_block( this->private_->signal_block_count_ );
    this->set_active_layer( active_layer );
  }
  
  return true;
}


// == static functions ==

bool LayerManager::CheckLayerExistance( const std::string& layer_id )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistance can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  if ( !( LayerManager::Instance()->get_layer_by_id( layer_id ) ) )
  {
    return false;
  }

  return true;
}


bool LayerManager::CheckLayerExistance( const std::string& layer_id, 
  Core::ActionContextHandle context )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistance can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  if ( !( LayerManager::Instance()->get_layer_by_id( layer_id ) ) )
  {
    context->report_error( std::string( "Incorrect layerid: layer '") + layer_id + 
      "' does not exist." );
    return false;
  }

  return true;
}


bool LayerManager::CheckGroupExistance( const std::string& group_id, 
  std::string& error )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckGroupExistance can only be called from the"
      " application thread." );
  }
  
  // Clear error string
  error = "";

  // Check whether layer exists
  if ( !( LayerManager::Instance()->get_group_by_id( group_id ) ) )
  {
    error = std::string( "Incorrect groupid: group '") + group_id + "' does not exist.";
    return false;
  }

  return true;
}


bool LayerManager::CheckLayerExistanceAndType( const std::string& layer_id, Core::VolumeType type, 
    Core::ActionContextHandle context )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistanceAndType can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( layer_id );
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
    Core::ActionContextHandle context )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerSize can only be called from the"
      " application thread." );
  }

  // Check whether layer exists
  LayerHandle layer1 = LayerManager::Instance()->get_layer_by_id( layer_id1 );
  if ( !layer1 )
  {
    context->report_error( std::string( "Incorrect layerid: layer '") + layer_id1 + 
      "' does not exist." );
    return false;
  }

  LayerHandle layer2 = LayerManager::Instance()->get_layer_by_id( layer_id2 );
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
    Core::ActionContextHandle context )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerAvailabilityForProcessing can only be called from the"
      " application thread." );
  }
  
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( layer_id );
  // Check whether layer exists
  if ( !layer )
  {
    CORE_THROW_LOGICERROR( "Layer does not exist, please check existance "
      "before availability" );
  }

  std::string layer_state = layer->data_state_->get();
  bool lock_state = layer->locked_state_->get();
  if ( layer_state == Layer::AVAILABLE_C && lock_state == false)
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
    Core::ActionContextHandle context )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerAvailabilityForUse can only be called from the"
      " application thread." );
  }
  
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( layer_id );
  // Check whether layer exists
  if ( !layer )
  {
    CORE_THROW_LOGICERROR( "Layer does not exist, please check existance "
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
    Core::ActionContextHandle context  )
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
    return LayerManager::CheckLayerAvailabilityForProcessing( layer_id, context );
  }
  else
  {
    return LayerManager::CheckLayerAvailabilityForUse( layer_id, context ); 
  }
}

LayerHandle LayerManager::FindLayer( const std::string& layer_id )
{
  return LayerManager::Instance()->get_layer_by_id( layer_id );
}

LayerHandle LayerManager::FindLayer( ProvenanceID prov_id )
{
  return LayerManager::Instance()->get_layer_by_provenance_id( prov_id );
}

LayerGroupHandle LayerManager::FindGroup( const std::string& group_id )
{
  return LayerManager::Instance()->get_group_by_id( group_id );
}

LayerGroupHandle LayerManager::FindGroup( ProvenanceID prov_id )
{
  return LayerManager::Instance()->get_group_by_provenance_id( prov_id );
}


MaskLayerHandle LayerManager::FindMaskLayer( const std::string& layer_id )
{
  return boost::shared_dynamic_cast<MaskLayer>(  
    LayerManager::Instance()->get_layer_by_id( layer_id ) );
}

DataLayerHandle LayerManager::FindDataLayer( const std::string& layer_id )
{
  return boost::shared_dynamic_cast<DataLayer>(  
    LayerManager::Instance()->get_layer_by_id( layer_id ) );
}

LayerGroupHandle LayerManager::FindLayerGroup( const std::string& group_id )
{
  return LayerManager::Instance()->get_group_by_id( group_id );
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
    LayerHandle& layer, const LayerMetaData& meta_data, filter_key_type key )
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
  LayerManager::Instance()->insert_layer( layer );
  
  return true;
}

bool LayerManager::CreateAndLockDataLayer( Core::GridTransform transform, const std::string& name, 
  LayerHandle& layer, const LayerMetaData& meta_data, filter_key_type key )
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
  LayerManager::Instance()->insert_layer( layer );
  
  return true;
}

void LayerManager::DispatchDeleteLayer( LayerHandle layer, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchDeleteLayer, 
      layer, key ) );
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
      std::vector< LayerHandle > layers( 1, layer );
      LayerManager::Instance()->delete_layers( layers );
    }
    else
    {
      CORE_THROW_LOGICERROR( "Could not delete that is connected to another filter" );
    }
  }
}

void LayerManager::DispatchUnlockLayer( LayerHandle layer, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchUnlockLayer, 
      layer, key ) );
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

void LayerManager::DispatchUnlockOrDeleteLayer( LayerHandle layer, filter_key_type key  )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchUnlockOrDeleteLayer, 
      layer, key ) );
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
        CORE_THROW_LOGICERROR( "Could not delete as filter is connected to another filter" );
      }
      layer->reset_filter_handle();
      layer->reset_allow_stop();

      std::vector< LayerHandle > layers( 1, layer );
      LayerManager::Instance()->delete_layers( layers );
    }
  }
}

void LayerManager::DispatchInsertDataVolumeIntoLayer( DataLayerHandle layer, 
  Core::DataVolumeHandle data, ProvenanceID prov_id, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertDataVolumeIntoLayer, layer, data, prov_id, key ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    if ( layer->set_data_volume( data ) )
    {
      layer->provenance_id_state_->set( prov_id );
      LayerManager::Instance()->layer_volume_changed_signal_( layer );
      LayerManager::Instance()->layers_changed_signal_();
    }
  }
}

void LayerManager::DispatchInsertMaskVolumeIntoLayer( MaskLayerHandle layer, 
  Core::MaskVolumeHandle mask, ProvenanceID prov_id, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertMaskVolumeIntoLayer, layer, mask, prov_id, key ) );
    return;
  }
  
  // Only do work if the unique key is a match
  if ( layer->check_filter_key( key ) )
  {
    if ( layer->set_mask_volume( mask ) )
    {
      layer->provenance_id_state_->set( prov_id );
      LayerManager::Instance()->layer_volume_changed_signal_( layer );
      LayerManager::Instance()->layers_changed_signal_();
    }
  }
}

void LayerManager::DispatchInsertVolumeIntoLayer( LayerHandle layer, 
  Core::VolumeHandle volume, ProvenanceID prov_id, filter_key_type key )
{
  if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    MaskLayerHandle mask = boost::shared_dynamic_cast<MaskLayer>( layer );
    Core::MaskVolumeHandle mask_volume = boost::shared_dynamic_cast<Core::MaskVolume>( volume );
    if ( mask && mask_volume )
    {
      DispatchInsertMaskVolumeIntoLayer( mask, mask_volume, prov_id, key );
    }
  }
  else if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    DataLayerHandle data = boost::shared_dynamic_cast<DataLayer>( layer );
    Core::DataVolumeHandle data_volume = boost::shared_dynamic_cast<Core::DataVolume>( volume );
    if ( data && data_volume )
    {
      DispatchInsertDataVolumeIntoLayer( data, data_volume, prov_id, key );
    }
  }
}

void LayerManager::DispatchInsertDataSliceIntoLayer( DataLayerHandle layer,
    Core::DataSliceHandle data,  ProvenanceID prov_id, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertDataSliceIntoLayer, layer, data, prov_id, key ) );
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
      LayerManager::Instance()->layer_volume_changed_signal_( layer );
      LayerManager::Instance()->layers_changed_signal_();
    }
  }
}

void LayerManager::DispatchInsertDataSlicesIntoLayer( DataLayerHandle layer,
    std::vector<Core::DataSliceHandle> data, ProvenanceID prov_id, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertDataSlicesIntoLayer, layer, data, prov_id, key ) );
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
    LayerManager::Instance()->layer_volume_changed_signal_( layer );
    LayerManager::Instance()->layers_changed_signal_();
  }
}


void LayerManager::DispatchInsertMaskSliceIntoLayer(MaskLayerHandle layer,
    Core::MaskDataSliceHandle mask, ProvenanceID prov_id, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertMaskSliceIntoLayer, layer, mask, prov_id, key ) );
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
      LayerManager::Instance()->layer_volume_changed_signal_( layer );
      LayerManager::Instance()->layers_changed_signal_();
    }
  }
}

void LayerManager::DispatchInsertMaskSlicesIntoLayer( MaskLayerHandle layer,
    std::vector<Core::MaskDataSliceHandle> mask, ProvenanceID prov_id, filter_key_type key )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertMaskSlicesIntoLayer, layer, mask, prov_id, key ) );
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
    LayerManager::Instance()->layer_volume_changed_signal_( layer );
    LayerManager::Instance()->layers_changed_signal_();
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

void LayerManager::handle_layer_name_changed( std::string layer_id, std::string name )
{
  this->layer_name_changed_signal_( layer_id, name );
}

void LayerManager::handle_layer_data_changed( LayerHandle layer )
{
  this->layer_data_changed_signal_( layer );
}

int LayerManager::find_free_color()
{
  std::set< int > used_colors;
  for( LayerManagerPrivate::group_list_type::iterator i = this->private_->group_list_.begin(); 
    i != this->private_->group_list_.end(); ++i )
  {
    for( layer_list_type::iterator j = ( *i )->layer_list_.begin(); 
      j != ( *i )->layer_list_.end(); ++j )
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

size_t LayerManager::get_group_position( LayerGroupHandle group )
{
  ASSERT_IS_APPLICATION_THREAD();

  LayerManagerPrivate::group_list_type::const_iterator it =  this->private_->group_list_.begin();
  LayerManagerPrivate::group_list_type::const_iterator it_end = this->private_->group_list_.end();
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
      LayerManagerPrivate::group_list_type::iterator group_it = std::find( 
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
  
  if ( new_groups.size() > 0 )
  {
    this->groups_changed_signal_();
  }

  std::set< LayerGroupHandle >::iterator it = changed_groups.begin();
  for ( ; it != changed_groups.end(); ++it )
  {
    this->group_internals_changed_signal_( *it );
  }
  
  this->layers_changed_signal_();
  for ( size_t i = 0; i < layers.size(); ++i )
  {
    this->layer_inserted_signal_( layers[ i ] );
  }
  if ( new_active_layer )
  {
    this->active_layer_changed_signal_( layers[ 0 ] );
  }
}


} // end namespace seg3D
