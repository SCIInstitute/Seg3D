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

#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>

#include <boost/lexical_cast.hpp>
#include <vector>

#include <Application/LayerManager/Actions/ActionInsertLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/LayerScene.h>


#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>


namespace Seg3D
{

LayerManager::LayerManager() :
  StateHandler( "LayerManager" )
{ 
}

LayerManager::~LayerManager()
{
}
  
bool LayerManager::insert_layer( LayerHandle layer )
{
  bool active_layer_changed = false;
  {
    lock_type lock( this->get_mutex() );
    
    SCI_LOG_DEBUG( std::string("Insert New Layer: ") + layer->get_layer_id());
    
    LayerGroupHandle group_handle;
    for ( group_handle_list_type::iterator it = group_handle_list_.begin(); 
       it != group_handle_list_.end(); ++it )
    {
            
      if (layer->get_grid_transform() == ( *it )->get_grid_transform()) 
      {
        group_handle = *it;
        break;
      }
    }

    if ( !group_handle )  
    {
      //TODO add logic for handling where the layer should go
      group_handle = LayerGroupHandle( new LayerGroup(  layer->get_grid_transform() ) );
      group_handle_list_.push_back( group_handle );
      
      SCI_LOG_DEBUG( std::string("Set Active Layer: ") + layer->get_layer_id());

      // deactivate the previous active layer
      if ( active_layer_ )
        active_layer_->set_active( false ); 

      active_layer_ = layer;
      active_layer_->set_active( true );
      
      active_layer_changed = true;
      
    }
      
    group_handle->insert_layer( layer );
    layer->set_layer_group( group_handle );
      
  }

  layer_inserted_signal_( layer );  

  
  if( active_layer_changed )
  {
    active_layer_changed_signal_( layer );
  }
  
  return true;
}

bool LayerManager::insert_layer_above( std::string layer_to_insert_id, std::string layer_below_id )
{
  // we will need to keep track of a few things outside of the locked scope
  // This keeps track of whether or not we delete the group we are moving from
  bool group_above_has_been_deleted = false;
  
  // These handles will let us send signals after we make the moves
  LayerGroupHandle group_above;
  LayerGroupHandle group_below;
  
  {
    // Get the Lock
    lock_type lock( this->get_mutex() );
  
    // First we get LayerHandles for the Layers
    LayerHandle layer_above = this->get_layer_by_id( layer_to_insert_id );
    LayerHandle layer_below = this->get_layer_by_id( layer_below_id );
    
    if ( !validate_layer_move( layer_above, layer_below ) )
      return false;
    
    if( !layer_above || !layer_below )
      return false;
    
    group_above = layer_above->get_layer_group();
    group_below = layer_below->get_layer_group();
    
    // First we Delete the Layer from its list of layers
    group_above->delete_layer( layer_above );
    group_below->insert_layer_above( layer_above, layer_below );
    
    // If they are in the same group ---
    if( group_above != group_below )
    {
      // If the group we are removing the layer from is empty we remove it
      //  from the list of groups and signal the GUI
      if( group_above->get_layer_list().empty() )
      {   
        group_handle_list_.remove( group_above );
        group_above_has_been_deleted = true;
        
      }
      // Set the weak handle in the layer we've inserted to the proper group
      layer_above->set_layer_group( group_below );
    }
  
  } // We release the lock  here.
  
  if( group_above_has_been_deleted )
  {
    group_deleted_signal_( group_above );
  } 
  else
  {
    group_changed_signal_( group_above );
  }
  
  if( group_above != group_below )
    group_changed_signal_( group_below );
  
  return true;
  
}

// Here is the logic for inserting a layer
bool LayerManager::validate_layer_move( LayerHandle layer_above, LayerHandle layer_below )
{
  // Validate the most common move
  if ( layer_above->type() == layer_below->type() )
    return true;
    
//  Another Easy one
//      if ( ( layer_above->type() == Utils::VolumeType::DATA_E ) && ( layer_below->type() == Utils::VolumeType::DATA_E ) )
//        return true;
  
  return false;
}


void LayerManager::set_active_layer( LayerHandle layer )
{
  {
    lock_type lock( this->get_mutex() );    
    
    // Do nothing if this layer is already the active one
    if ( this->active_layer_ == layer )
    {
      return;
    }
    
    active_layer_->set_active( false );
  
    SCI_LOG_DEBUG( std::string("Set Active Layer: ") + layer->get_layer_id());
    
    active_layer_ = layer;
    active_layer_->set_active( true );
  }

  active_layer_changed_signal_( layer );  
}


LayerGroupHandle LayerManager::get_LayerGroupHandle_from_group_id( std::string group_id )
{
    lock_type lock( this->get_mutex() );
    
  for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
    i != group_handle_list_.end(); ++i )
  {
    if (( *i )->get_group_id() == group_id ) 
    {
      return ( *i );
    }
  }
  return LayerGroupHandle();
}

Seg3D::LayerHandle LayerManager::get_layer_by_id( const std::string& layer_id )
{
  lock_type lock( this->get_mutex() );

  for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
    i != group_handle_list_.end(); ++i )
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

Seg3D::LayerHandle LayerManager::get_layer_by_name( const std::string& layer_name )
{
  lock_type lock( this->get_mutex() );

  for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
    i != group_handle_list_.end(); ++i )
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
    
  for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
    i != group_handle_list_.end(); ++i )
  {
    vector_of_groups.push_back( *i );
  } 
}


void LayerManager::get_layers( std::vector< LayerHandle > &vector_of_layers )
{
    lock_type lock( this->get_mutex() );
    
  for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
    i != group_handle_list_.end(); ++i )
  {
      for( layer_list_type::iterator j = ( *i )->layer_list_.begin(); 
    j != ( *i )->layer_list_.end(); ++j )
      {
          vector_of_layers.push_back( ( *j ) );
      }
  }
}

void LayerManager::delete_layers( LayerGroupHandle group )
{
  std::vector< LayerHandle > layer_vector;
  bool active_layer_changed = false;
  {
    lock_type lock( get_mutex() );  
    
    // get a temporary copy of the list of layers
    layer_list_type layer_list = group->get_layer_list();
    
    bool active_layer_deleted = false;
    for( layer_list_type::iterator it = layer_list.begin(); it != layer_list.end(); ++it )
    {
      if( ( *it )->selected_state_->get() )
      {   
        SCI_LOG_DEBUG( std::string("Deleting Layer: ") + ( *it )->get_layer_id() );
        layer_vector.push_back( *it );
        group->delete_layer( *it );
        if ( *it == this->active_layer_ )
        {
          active_layer_deleted = true;
        }
      }
    }
    
    if( group->is_empty() )
    {   
      group_handle_list_.remove( group );
    }

    if ( active_layer_deleted )
    {
      this->active_layer_.reset();
      if ( this->group_handle_list_.size() > 0 )
      {
        this->active_layer_ = this->group_handle_list_.front()->layer_list_.back();
        this->active_layer_->set_active( true );
        active_layer_changed = true;
      }
    }
    
  } // Unlocked from here:

  if( group->is_empty() )
  {   
      group_deleted_signal_( group );
  }
  
  // signal the listeners
  layers_deleted_signal_( layer_vector );
  layers_finished_deleting_signal_( group );
  if ( active_layer_changed )
  {
    this->active_layer_changed_signal_( this->active_layer_ );
  }
} // end delete_layer

LayerGroupHandle LayerManager::get_active_group()
{
  lock_type lock( this->get_mutex() );  
  return this->active_layer_->get_layer_group();
}

Seg3D::LayerHandle LayerManager::get_active_layer()
{
  lock_type lock( this->get_mutex() );  
  return this->active_layer_;
}

LayerManager::mutex_type& LayerManager::get_mutex()
{
  return StateEngine::GetMutex();
}
  
LayerSceneHandle LayerManager::compose_layer_scene( size_t viewer_id )
{
  // NOTE: This functions is called from the Rendering Thread
  // Lock the LayerManager
  lock_type lock( this->get_mutex() );
  
  // NOTE: Need to lock the state engine to prevetn the application thread from making any changes
  // to the current state of the layers and layer groups. 
  // StateEngine::lock_type state_lock( StateEngine::GetMutex() ) ;

  LayerSceneHandle layer_scene( new LayerScene );

  // For each layer group
  group_handle_list_type::iterator group_iterator = this->group_handle_list_.begin();
  for ( ; group_iterator != this->group_handle_list_.end(); group_iterator++)
  {
    layer_list_type layer_list = ( *group_iterator )->get_layer_list();

    layer_list_type::iterator layer_iterator = layer_list.begin();
    // For each layer in the group
    for ( ; layer_iterator != layer_list.end(); layer_iterator++ )
    {
      LayerHandle layer = *layer_iterator;
      
      // Skip processing this layer if it's not visible
      if ( !layer->visible_state_[ viewer_id ]->get() )
      {
        continue;
      }

      LayerSceneItemHandle layer_scene_item;

      switch( layer->type() )
      {
      case Utils::VolumeType::DATA_E:
        {
          DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
          DataLayerSceneItem* data_layer_scene_item = new DataLayerSceneItem;
          layer_scene_item = LayerSceneItemHandle( data_layer_scene_item );
          data_layer_scene_item->data_volume_ = data_layer->get_data_volume();
          data_layer_scene_item->contrast_ = data_layer->contrast_state_->get();
          data_layer_scene_item->brightness_ = data_layer->brightness_state_->get();
          data_layer_scene_item->volume_rendered_ = data_layer->volume_rendered_state_->get();
        }
        break;
      case Utils::VolumeType::MASK_E:
        {
          MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
          MaskLayerSceneItem* mask_layer_scene_item = new MaskLayerSceneItem;
          layer_scene_item = LayerSceneItemHandle( mask_layer_scene_item );
          mask_layer_scene_item->mask_volume_ = mask_layer->get_mask_volume();
          mask_layer_scene_item->color_ = mask_layer->color_state_->get();
          mask_layer_scene_item->border_ = mask_layer->border_state_->get();
          mask_layer_scene_item->fill_ = mask_layer->fill_state_->get();
          mask_layer_scene_item->show_isosurface_ = mask_layer->show_isosurface_state_->get();
        }
        break;
      default:
        SCI_THROW_LOGICERROR("Unknow layer type");
        break;
      } // end switch

      layer_scene_item->layer_id_ = layer->get_layer_id();
      layer_scene_item->opacity_ = layer->opacity_state_->get();
      layer_scene_item->grid_transform_ = layer->get_grid_transform();

      layer_scene->push_back( layer_scene_item );
    } // end for each layer

  } // end for each group

  return layer_scene;
}

} // end namespace seg3D
