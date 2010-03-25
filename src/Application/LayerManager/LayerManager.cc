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
  lock_type lock( this->group_handle_list_mutex_ );
  
  SCI_LOG_DEBUG( std::string("Insert New Layer: ") + layer->get_layer_id());
  
  for ( group_handle_list_type::iterator it = group_handle_list_.begin(); 
     it != group_handle_list_.end(); ++it )
  {
          
    if (layer->get_grid_transform() == ( *it )->get_grid_transform()) 
    {
      ( *it )->insert_layer( layer );
      layer->set_layer_group( *it );

      // NOTE: Unlock ASAP to avoid potential deadlock. It is especially important to unlock
      // before triggering any signals
      lock.unlock();
      
      layer_inserted_signal_( layer );
      set_active_layer( layer );
      
      return true;
    }
  }
  
  //TODO add logic for handling where the layer should go
  
  LayerGroupHandle new_group = create_group( layer->get_grid_transform() );
  layer->set_layer_group( new_group );
  new_group->insert_layer( layer );
  group_handle_list_.push_back( new_group );
  
  // NOTE: Unlock ASAP to avoid potential deadlock. It is especially important to unlock
  // before triggering any signals
  lock.unlock();

  // Send a signal alerting the UI that we have inserted a layer
  layer_inserted_signal_( layer );
  set_active_layer( layer );
  return true;
}

void LayerManager::set_active_layer( LayerHandle layer )
{
    lock_type lock( this->group_handle_list_mutex_ );    

    SCI_LOG_DEBUG( std::string("Set Active Layer: ") + layer->get_layer_id());
   
  active_layer_ = layer;
    
  // NOTE: Unlock ASAP to avoid potential deadlock. It is especially important to unlock
  // before triggering any signals
  lock.unlock();
  
  active_layer_changed_signal_( layer );
  
}


LayerGroupHandle LayerManager::check_for_group( std::string group_id )
{
    lock_type lock( this->group_handle_list_mutex_ );
    
  for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
    i != group_handle_list_.end(); ++i )
  {
    if (( *i )->get_group_id() == group_id ) {
      return ( *i );
    }
  }
  return LayerGroupHandle();
}

LayerGroupHandle LayerManager::create_group( const Utils::GridTransform&  transform ) const
{
  return LayerGroupHandle( new LayerGroup( transform  ));
}

void LayerManager::return_group_vector( std::vector< LayerGroupHandle > &vector_of_groups )
{
    lock_type lock( this->group_handle_list_mutex_ );
    
  for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
    i != group_handle_list_.end(); ++i )
  {
    vector_of_groups.push_back( *i );
  } 
}


void LayerManager::return_layers_vector( std::vector< LayerHandle > &vector_of_layers )
{
    lock_type lock( this->group_handle_list_mutex_ );
    
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
    lock_type lock( this->group_handle_list_mutex_ );  
    
    std::vector< LayerHandle > layer_vector;
    
    // get a temporary copy of the list of layers
    layer_list_type layer_list = group->get_layer_list();
  
  for( layer_list_type::iterator i = layer_list.begin(); 
      i != layer_list.end(); ++i )
  {
        if( ( *i )->selected_state_->get() )
        {   
            SCI_LOG_DEBUG( std::string("Deleting Layer: ") + ( *i )->get_layer_id());
            layer_vector.push_back(( *i ));
            group->delete_layer( ( *i ) );
        }
  }
  
    // NOTE: Unlock ASAP to avoid potential deadlock. It is especially important to unlock
  // before triggering any signals
  lock.unlock();
  
  if( group->get_layer_list().empty() )
  {   
      delete_group_signal_( group );
      group_handle_list_.remove( group );
  }
  

  
  // signal the listeners
  layers_deleted_signal_( layer_vector );
  layers_finished_deleting_signal_( group );
  
} // end delete_layer



LayerGroupWeakHandle LayerManager::get_active_group()
{
  return active_layer_->get_layer_group();
}

LayerManager::mutex_type& LayerManager::get_mutex()
{
  return group_handle_list_mutex_;
}
  
LayerSceneHandle LayerManager::compose_layer_scene( size_t viewer_id )
{
  // Lock the LayerManager
  lock_type lock( this->group_handle_list_mutex_ );

  LayerSceneHandle layer_scene( new LayerScene );

  // For each group, generate a LayerGroupSceneItem
  group_handle_list_type::iterator group_iterator = this->group_handle_list_.begin();
  for ( ; group_iterator != this->group_handle_list_.end(); group_iterator++)
  {
    LayerGroupSceneItemHandle layer_group_scene_item( new LayerGroupSceneItem );
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
        assert( false );
        break;
      } // end switch

      layer_scene_item->layer_id_ = layer->get_layer_id();
      layer_scene_item->opacity_ = layer->opacity_state_->get();
      layer_scene_item->grid_transform_ = layer->get_grid_transform();

      layer_group_scene_item->push_back( layer_scene_item );
    } // end for each layer

    // only added the group to the scene if its number of visible layers isn't 0
    if ( layer_group_scene_item->size() > 0 )
    {
      layer_scene->push_back( layer_group_scene_item );
    }

  } // end for each group

  return layer_scene;
}

} // end namespace seg3D
