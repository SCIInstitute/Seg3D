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
#include <Core/Volume/Volume.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>

// Application action includes
#include <Application/LayerManager/LayerManager.h>


namespace Seg3D
{

const size_t LayerManager::VERSION_NUMBER_C = 1;

CORE_SINGLETON_IMPLEMENTATION( LayerManager );

LayerManager::LayerManager() :
  StateHandler( "layermanager", VERSION_NUMBER_C, false, 0 )
{ 
  std::vector< std::string> layers;
  this->add_state( "layers", this->layers_state_, layers );
  this->add_state( "active_layer", this->active_layer_state_, "" );
}

LayerManager::~LayerManager()
{
}
  
bool LayerManager::insert_layer( LayerHandle layer )
{
  bool active_layer_changed = false;
  
  {
    lock_type lock( this->get_mutex() );
    
    CORE_LOG_DEBUG( std::string("Insert New Layer: ") + layer->get_layer_id());
        
    LayerGroupHandle group_handle;
    for ( group_list_type::iterator it = group_list_.begin(); 
       it != group_list_.end(); ++it )
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
      group_handle = LayerGroupHandle( new LayerGroup(  layer->get_grid_transform() ) );
      group_list_.push_back( group_handle );
      
      CORE_LOG_DEBUG( std::string( "Set Active Layer: " ) + layer->get_layer_id());

      active_layer_ = layer;
      active_layer_changed = true;
      
    }
    if( layer->type() == Core::VolumeType::DATA_E )
      group_handle->insert_layer_front( layer );
    else 
      group_handle->insert_layer_back( layer );
      
    layer->set_layer_group( group_handle );
      
  } // unlocked from here

  CORE_LOG_DEBUG( std::string( "Signalling that new layer was inserted" ) );
  CORE_LOG_DEBUG( std::string( "--- triggering signals ---" ) );
  
  layer_inserted_signal_( layer );
  layers_changed_signal_();

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
  int index = 0;

  {
    // Get the Lock
    lock_type lock( this->get_mutex() );

    LayerGroupHandle group_above = get_layer_group( group_to_move_id );
    LayerGroupHandle group_below = get_layer_group( group_below_id );

    if( ( !group_above || !group_below ) || ( group_above == group_below ) )
      return false;

    this->group_list_.remove( group_above );
    index = this->insert_group( group_above, group_below );
  }

  group_inserted_at_signal_( group_to_move_id, index );
    
  return true;
}

int LayerManager::insert_group( LayerGroupHandle group_above, LayerGroupHandle group_below )
{
  int index = 0;

  for( group_list_type::iterator i = this->group_list_.begin(); 
    i != this->group_list_.end(); ++i )
  {
    if( ( *i ) == group_below )
    {
      // First we get the size of the list before the insert
      int list_size = static_cast< int >( this->group_list_.size() );

      // Second we insert the layer
      this->group_list_.insert( ++i, group_above );
      
      // Finally we return the proper location for the gui to insert the group
      return abs( index - list_size ) - 1;

    }
    index++;
  }
  return -1;
}

bool LayerManager::move_layer_above( LayerHandle layer_to_move, LayerHandle target_layer )
{
  // we will need to keep track of a few things outside of the locked scope
  // This keeps track of whether or not we delete the group we are moving from
  bool group_above_has_been_deleted = false;
  
  // This is the index we will send to tell the GUI where to put the layer
  int index;
  
  // These handles will let us send signals after we make the moves
  LayerGroupHandle group_above;
  LayerGroupHandle group_below;
  
  {
    // Get the Lock
    lock_type lock( this->get_mutex() );
      
    if( !layer_to_move || !target_layer ) return false;
    
    if ( !validate_layer_move( layer_to_move, target_layer ) )
      return false;
    
    group_above = layer_to_move->get_layer_group();
    group_below = target_layer->get_layer_group();
    
    // First we Delete the Layer from its list of layers
    group_above->delete_layer( layer_to_move, false );
    index = group_below->move_layer_above( layer_to_move, target_layer );
    
    // If they are in the same group ---
    if( group_above != group_below )
    {
      // If the group we are removing the layer from is empty we remove it
      //  from the list of groups and signal the GUI
      if( group_above->get_layer_list().empty() )
      {   
        group_list_.remove( group_above );
        group_above_has_been_deleted = true;
        
      }
      // Set the weak handle in the layer we've inserted to the proper group
      layer_to_move->set_layer_group( group_below );
    }
  
  
    if( group_above_has_been_deleted )
    {
      group_deleted_signal_( group_above );
    } 
    else
    {
      layer_deleted_signal_( layer_to_move );
    }
    
    layer_inserted_at_signal_( layer_to_move, index );
    layers_changed_signal_();

  } // We release the lock  here.
  
  return true;  
}

bool LayerManager::move_layer_below( LayerHandle layer_to_move, LayerHandle target_layer )
{

  return true;  
}


// Here is the logic for inserting a layer
bool LayerManager::validate_layer_move( LayerHandle layer_above, LayerHandle layer_below )
{
  // Validate the most common move
  if( layer_above->type() == layer_below->type() )
    return true;
  
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
    
    CORE_LOG_DEBUG( std::string("Set Active Layer: ") + layer->get_layer_id());
    
    active_layer_ = layer;
        
  } // We release the lock  here.

  active_layer_changed_signal_( layer );  
}


LayerGroupHandle LayerManager::get_layer_group( std::string group_id )
{
    lock_type lock( this->get_mutex() );
    
  for( group_list_type::iterator i = group_list_.begin(); 
    i != group_list_.end(); ++i )
  {
    if (( *i )->get_group_id() == group_id ) 
    {
      return ( *i );
    }
  }
  return LayerGroupHandle();
}

LayerHandle LayerManager::get_layer_by_id( const std::string& layer_id )
{
  lock_type lock( this->get_mutex() );

  for( group_list_type::iterator i = group_list_.begin(); 
    i != group_list_.end(); ++i )
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

LayerHandle LayerManager::get_layer_by_name( const std::string& layer_name )
{
  lock_type lock( this->get_mutex() );

  for( group_list_type::iterator i = group_list_.begin(); 
    i != group_list_.end(); ++i )
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
    
  for( group_list_type::iterator i = group_list_.begin(); 
    i != group_list_.end(); ++i )
  {
    vector_of_groups.push_back( *i );
  } 
}


void LayerManager::get_layers( std::vector< LayerHandle > &vector_of_layers )
{
    lock_type lock( this->get_mutex() );
    
  for( group_list_type::iterator i = group_list_.begin(); 
    i != group_list_.end(); ++i )
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
  
  { // start the lock scope
    lock_type lock( get_mutex() );  
    
    // get a temporary copy of the list of layers
    layer_list_type layer_list = group->get_layer_list();
    
    bool active_layer_deleted = false;
    for( layer_list_type::iterator it = layer_list.begin(); it != layer_list.end(); ++it )
    {
      if( ( *it )->selected_state_->get() )
      {   
        CORE_LOG_DEBUG( std::string("Deleting Layer: ") + ( *it )->get_layer_id() );
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
      group->invalidate();
      group_list_.remove( group );
    }

    if ( active_layer_deleted )
    {
      this->active_layer_.reset();
      if ( this->group_list_.size() > 0 )
      {
        this->active_layer_ = this->group_list_.front()->layer_list_.back();
        //this->active_layer_->set_active( true );
        active_layer_changed = true;
      }
    }
  } // Unlocked from here:

// signal the listeners
  if( group->is_empty() )
  {   
      group_deleted_signal_( group );
  }
  
  layers_deleted_signal_( layer_vector );
  layers_changed_signal_();
  
  if ( active_layer_changed )
  {
    this->active_layer_changed_signal_( this->active_layer_ );
  }
  
} // end delete_layer

bool LayerManager::delete_all()
{
  lock_type lock( get_mutex() );

  // Cycle through all the groups and delete all the layers
  group_list_type::iterator group_iterator = this->group_list_.begin();
  for ( ; group_iterator != this->group_list_.end(); )
  {
    // set all of the layers to selected so they are deleted.
    layer_list_type layer_list = ( *group_iterator )->get_layer_list();
    for( layer_list_type::iterator it = layer_list.begin(); it != layer_list.end(); ++it)
    {
      ( *it )->selected_state_->set( true );
    }

    group_list_type::iterator it_temp = group_iterator;
    ++it_temp;

    this->delete_layers( *group_iterator );
    if( group_list_.empty() )
    {
      break;
    }
    group_iterator = it_temp;
  }
  return true;
}


LayerHandle LayerManager::get_active_layer()
{
  lock_type lock( this->get_mutex() );  
  return this->active_layer_;
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
  group_list_type::iterator group_iterator = this->group_list_.begin();
  for ( ; group_iterator != this->group_list_.end(); group_iterator++)
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
      case Core::VolumeType::DATA_E:
        {
          DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
          DataLayerSceneItem* data_layer_scene_item = new DataLayerSceneItem;
          layer_scene_item = LayerSceneItemHandle( data_layer_scene_item );
          data_layer_scene_item->data_volume_ = data_layer->get_data_volume();
          data_layer_scene_item->contrast_ = data_layer->contrast_state_->get();
          data_layer_scene_item->brightness_ = data_layer->brightness_state_->get();
          data_layer_scene_item->volume_rendered_ = data_layer->
            volume_rendered_state_->get();
        }
        break;
      case Core::VolumeType::MASK_E:
        {
          MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
          MaskLayerSceneItem* mask_layer_scene_item = new MaskLayerSceneItem;
          layer_scene_item = LayerSceneItemHandle( mask_layer_scene_item );
          mask_layer_scene_item->mask_volume_ = mask_layer->get_mask_volume();
          mask_layer_scene_item->color_ = mask_layer->color_state_->get();
          mask_layer_scene_item->border_ = mask_layer->border_state_->index();
          mask_layer_scene_item->fill_ = mask_layer->fill_state_->get();
          mask_layer_scene_item->show_isosurface_ = mask_layer->
            show_isosurface_state_->get();
        }
        break;
      default:
        CORE_THROW_LOGICERROR("Unknow layer type");
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

Core::BBox LayerManager::get_layers_bbox()
{
  // NOTE: This functions is called from the Rendering Thread
  // Lock the LayerManager
  lock_type lock( this->get_mutex() );

  Core::BBox bbox;
  group_list_type::iterator group_iterator = this->group_list_.begin();
  for ( ; group_iterator != this->group_list_.end(); group_iterator++)
  {
    LayerGroupHandle group = *group_iterator;
    const Core::GridTransform& grid_trans = group->get_grid_transform();
    Core::Point pt( 0, 0, 0 );
    bbox.extend( grid_trans * pt );
    pt = Core::Point( static_cast< double >( grid_trans.get_nx() - 1 ), 
      static_cast< double >( grid_trans.get_ny() - 1 ), 
      static_cast< double >( grid_trans.get_nz() - 1 ) );
    bbox.extend( grid_trans * pt );
  }

  return bbox;
}

void LayerManager::get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
    Core::VolumeType type )
{
  lock_type lock( this->get_mutex() );

  std::vector< LayerHandle > layers;
  LayerManager::Instance()->get_layers( layers );
  size_t num_of_layers = layers.size();
  for ( size_t i = 0; i < num_of_layers; i++ )
  {
    if ( layers[ i ]->type() == type )
    {
      layer_names.push_back( std::make_pair( layers[ i ]->get_layer_id(), 
        layers[ i ]->get_layer_name() ) );
    }
  }
}

void LayerManager::get_layer_names( std::vector< LayerIDNamePair >& layer_names )
{
  lock_type lock( this->get_mutex() );

  std::vector< LayerHandle > layers;
  LayerManager::Instance()->get_layers( layers );
  size_t num_of_layers = layers.size();
  for ( size_t i = 0; i < num_of_layers; i++ )
  {
    layer_names.push_back( std::make_pair( layers[ i ]->get_layer_id(), 
      layers[ i ]->get_layer_name() ) );
  }
}


  
bool LayerManager::pre_save_states()
{
  lock_type lock( this->get_mutex() );

  if( this->active_layer_ )
  {
    this->active_layer_state_->set( this->active_layer_->get_layer_id() );
  }
  else
  {
    this->active_layer_state_->set( "none" );
  }

  std::vector< std::string > layers_vector;
  this->layers_state_->set( layers_vector );
  
  group_list_type::reverse_iterator group_iterator = this->group_list_.rbegin();
  for ( ; group_iterator != this->group_list_.rend(); ++group_iterator )
  {
    std::string group = ( *group_iterator )->get_group_id();
    std::vector< std::string > group_layers_vector; 
    ( *group_iterator )->get_layer_names( group_layers_vector );
    for( int i = 0; i < static_cast< int >( group_layers_vector.size() ); ++i  )
    //for( int i = static_cast< int >( group_layers_vector.size() ) - 1; i >= 0; --i  )
    {
      layers_vector.push_back( group_layers_vector[ i ] );
    }
  }
  
  this->layers_state_->set( layers_vector );
  return true;
}
  

bool LayerManager::post_save_states()
{
  lock_type lock( this->get_mutex() );
  
  std::vector< LayerHandle > layers;
  this->get_layers( layers );

  for( int i = static_cast< int >( layers.size() ) -1 ; i >= 0; --i )
  {
    if( !layers[ i ]->populate_session_states() )
    {
      return false;
    }
  }
  return Core::MaskDataBlockManager::Instance()->save_data_blocks();
}
  
  
bool LayerManager::pre_load_states()
{
  return this->delete_all();
}

  
bool LayerManager::post_load_states()
{
  std::vector< std::string > state_values;
  Core::StateEngine::Instance()->get_session_states( state_values );

  std::vector< std::string > layer_vector = this->layers_state_->get();
  for( int j = 0; j < static_cast< int >( layer_vector.size() ); ++j )
  //for( int j = static_cast< int >( layer_vector.size() ) - 1; j >= 0; --j )
  {
    if( layer_vector[ j ] == "]" )
    {
      return true;
    }

    LayerHandle restored_layer;
    std::vector< std::string > layer = Core::SplitString( layer_vector[ j ], "|" );
  
    if( layer[ 1 ] == "DATA_E" )
    {
      restored_layer = LayerHandle( new DataLayer( layer[ 0 ] ) );
    }
    else if( layer[ 1 ] == "MASK_E" )
    {
      restored_layer = LayerHandle( new MaskLayer(  layer[ 0 ] ) );
    }
    else
    {
      return false;
    }
    
    if( !restored_layer->load_states( state_values ) )
    {
      return false;
    }
    
    this->insert_layer( restored_layer );
  }

  // If there are layers loaded, restore the active layer state
  if ( this->group_list_.size() > 0 )
  {
    LayerHandle active_layer = this->get_layer_by_id( this->active_layer_state_->get() );
    if ( !active_layer )
    {
      CORE_LOG_ERROR( "Incorrect active layer state loaded from session" );
      active_layer = this->group_list_.front()->layer_list_.back(); 
    }
    this->set_active_layer( active_layer );
  }
  
  return true;
}



} // end namespace seg3D
