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
#include <Application/LayerManager/LayerScene.h>
#include <Application/LayerManager/LayerAvailabilityNotifier.h>

// Application action includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include "Core/State/StateIO.h"


namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( LayerManager );

LayerManager::LayerManager() :
  StateHandler( "layermanager", false )
{ 
  this->add_state( "active_layer", this->active_layer_state_, "" );
}

LayerManager::~LayerManager()
{
}
  
bool LayerManager::insert_layer( LayerHandle layer )
{
  bool active_layer_changed = false;
  bool new_group = false;
  LayerGroupHandle group_handle;
  
  {
    lock_type lock( this->get_mutex() );
    
    CORE_LOG_MESSAGE( std::string("Insert New Layer: ") + layer->get_layer_id());
        
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
      new_group = true;
      group_handle = LayerGroupHandle( new LayerGroup(  layer->get_grid_transform() ) );
      group_list_.push_back( group_handle );
      
      CORE_LOG_DEBUG( std::string( "Set Active Layer: " ) + layer->get_layer_id());

      active_layer_ = layer;
      active_layer_changed = true;
      
    }
    
    group_handle->insert_layer( layer );
      
    layer->set_layer_group( group_handle );
      
  } // unlocked from here

  CORE_LOG_DEBUG( std::string( "Signalling that new layer was inserted" ) );
  CORE_LOG_DEBUG( std::string( "--- triggering signals ---" ) );
  
  // This is no longer needed by the LayerManager Widget
  if ( new_group )
  {
    this->group_inserted_signal_( group_handle );
  }

  this->group_internals_changed_signal_( group_handle );

  // This is no longer needed by the LayerManager Widget  
  this->layer_inserted_signal_( layer );
  
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
  //int index = 0;

  {
    // Get the Lock
    lock_type lock( this->get_mutex() );

    LayerGroupHandle group_above = get_layer_group( group_to_move_id );
    LayerGroupHandle group_below = get_layer_group( group_below_id );

    if( ( !group_above || !group_below ) || ( group_above == group_below ) )
      return false;

    this->group_list_.remove( group_above );
    this->insert_group( group_above, group_below );
  }
  
  this->groups_changed_signal_();

  return true;
}

void LayerManager::insert_group( LayerGroupHandle group_above, LayerGroupHandle group_below )
{
  for( group_list_type::iterator i = this->group_list_.begin(); 
    i != this->group_list_.end(); ++i )
  {
    if( ( *i ) == group_below )
    {
      //we insert the layer
      this->group_list_.insert( i, group_above );
    }
  }
}

bool LayerManager::move_layer_above( LayerHandle layer_to_move, LayerHandle target_layer )
{
  // we will need to keep track of a few things outside of the locked scope
  // This keeps track of whether or not we delete the group we are moving from
  bool group_above_has_been_deleted = false;
  
  bool layer_has_changed_groups = false;
  
//  // This is the index we will send to tell the GUI where to put the layer
//  int index;
  
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
    group_below->move_layer_above( layer_to_move, target_layer );
    
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
      layer_has_changed_groups = true;
    }
  
    if( layer_has_changed_groups )
    {
      this->group_internals_changed_signal_( group_above );
      this->group_internals_changed_signal_( group_below );
    }
    else
    {
      this->group_internals_changed_signal_( group_above );
    }

    // dont need any of this after the update
      if( group_above_has_been_deleted )
      {
        group_deleted_signal_( group_above );
      } 


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
  bool active_layer_changed = false;
  bool group_deleted = false;

  std::vector< LayerHandle > layer_vector;
  
  { // start the lock scope
    lock_type lock( get_mutex() );  
    
    // get a temporary copy of the list of layers
    layer_list_type layer_list = group->get_layer_list();
    
    bool active_layer_deleted = false;
    for( layer_list_type::iterator it = layer_list.begin(); it != layer_list.end(); ++it )
    {
      if( ( *it )->selected_state_->get() )
      {   
        CORE_LOG_MESSAGE( std::string("Deleting Layer: ") + ( *it )->get_layer_id() );
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
      this->group_list_.remove( group );
    }

    if ( active_layer_deleted )
    {
      this->active_layer_.reset();
      if ( this->group_list_.size() > 0 )
      {
        this->active_layer_ = this->group_list_.front()->layer_list_.back();
        active_layer_changed = true;
      }
    }

    if ( group->is_empty() ) group_deleted = true;

  } // Unlocked from here:

  //signal the listeners
  if( group_deleted )
  {   
      this->group_deleted_signal_( group );
    this->groups_changed_signal_();
  }
  else
  {
    group->menu_state_->set( LayerGroup::NO_MENU_C );
    this->group_internals_changed_signal_( group );
  }

  this->layers_deleted_signal_( layer_vector );

  
  if ( active_layer_changed )
  {
    this->active_layer_changed_signal_( this->active_layer_ );
  }
  
} // end delete_layer

void LayerManager::delete_layer( LayerHandle layer )
{
  bool active_layer_changed = false;
  bool group_deleted = false;
  
  LayerGroupHandle group;
  
  { // start the lock scope
    lock_type lock( this->get_mutex() );  

    CORE_LOG_MESSAGE( std::string( "Deleting Layer: " ) + layer->get_layer_id() );
    
    group = layer->get_layer_group();
    group->delete_layer( layer );
    
    if( group->is_empty() )
    {   
      group->invalidate();
      this->group_list_.remove( group );
    }

    if ( this->active_layer_ == layer )
    {
      this->active_layer_.reset();
      if ( this->group_list_.size() > 0 )
      {
        this->active_layer_ = this->group_list_.front()->layer_list_.back();
        active_layer_changed = true;
      }
    }
    
    if ( group->is_empty() ) group_deleted = true;
  } 
  
  // signal the listeners
  if( group_deleted )
  {   
      this->group_deleted_signal_( group );
    this->groups_changed_signal_();
  }
  else
  {
    group->menu_state_->set( LayerGroup::NO_MENU_C );
    this->group_internals_changed_signal_( group );
  }
  
  std::vector<LayerHandle> layer_vector( 1 );
  layer_vector[ 0 ] = layer;
  this->layers_deleted_signal_( layer_vector );

  
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
    if ( !( *group_iterator )->visibility_state_->get() )
    {
      continue;
    }
    
    layer_list_type layer_list = ( *group_iterator )->get_layer_list();

    layer_list_type::iterator layer_iterator = layer_list.begin();
    // For each layer in the group
    for ( ; layer_iterator != layer_list.end(); layer_iterator++ )
    {
      LayerHandle layer = *layer_iterator;
      
      // Skip processing this layer if it's not visible or that is not valid.
      // NOTE: Layers that are not valid include the layers that are currently
      // under construction.
      if ( !layer->visible_state_[ viewer_id ]->get() || !layer->is_valid() )
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

bool LayerManager::pre_save_states( Core::StateIO& state_io )
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

  return true;
}
  

bool LayerManager::post_save_states( Core::StateIO& state_io )
{
  TiXmlElement* lm_element = state_io.get_current_element();
  assert( this->get_statehandler_id() == lm_element->Value() );
  TiXmlElement* layers_element = new TiXmlElement( "layers" );
  lm_element->LinkEndChild( layers_element );

  state_io.push_current_element();
  state_io.set_current_element( layers_element );

  std::vector< LayerHandle > layers;
  this->get_layers( layers );

  //for( size_t i = 0; i < layers.size(); ++i )
  int number_of_layers = static_cast< int >( layers.size() );
  
  for( int i = ( number_of_layers - 1 ); i >= 0; i-- )
  {
    layers[ i ]->save_states( state_io );
  }
  

  state_io.pop_current_element();

  return Core::MaskDataBlockManager::Instance()->save_data_blocks( 
    Seg3D::ProjectManager::Instance()->get_project_data_path() );
} 
  
bool LayerManager::pre_load_states( const Core::StateIO& state_io )
{
  return this->delete_all();
}

bool LayerManager::post_load_states( const Core::StateIO& state_io )
{
  const TiXmlElement* layers_element = state_io.get_current_element()->
    FirstChildElement( "layers" );
  if ( layers_element == 0 )
  {
    return false;
  }

  state_io.push_current_element();
  state_io.set_current_element( layers_element );

  bool success = true;
  const TiXmlElement* layer_element = layers_element->FirstChildElement();
  while ( layer_element != 0 )
  {
    std::string layer_id( layer_element->Value() );
    std::string layer_type( layer_element->Attribute( "type" ) );
    LayerHandle layer;
    if ( layer_type == "data" )
    {
      layer.reset( new DataLayer( layer_id ) );
    }
    else if ( layer_type == "mask" )
    {
      layer.reset( new MaskLayer( layer_id ) );
    }
    else
    {
      CORE_LOG_ERROR( "Unsupported layer type" );
    }

    if ( layer && layer->load_states( state_io ) )
    {
      this->insert_layer( layer );
    }
    else
    {
      success = false;
    }

    layer_element = layer_element->NextSiblingElement();
  }

  state_io.pop_current_element();

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
  
  return success;
}


// == static functions ==

bool LayerManager::CheckLayerExistance( const std::string& layer_id, std::string& error )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistance can only be called from the"
      " application thread." );
  }
  
  // Clear error string
  error = "";

  // Check whether layer exists
  if ( !( LayerManager::Instance()->get_layer_by_id( layer_id ) ) )
  {
    error = std::string( "Incorrect layerid: layer '") + layer_id + "' does not exist.";
    return false;
  }

  return true;
}

bool LayerManager::CheckLayerExistanceAndType( const std::string& layer_id, Core::VolumeType type, 
    std::string& error )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerExistanceAndType can only be called from the"
      " application thread." );
  }

  // Clear error string
  error = "";
  
  // Check whether layer exists
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( layer_id );
  if ( !layer )
  {
    error = std::string( "Incorrect layerid: layer '") + layer_id + "' does not exist.";
    return false;
  }

  // Check whether the type of the layer is correct
  if ( layer->type() != type )
  {
    if ( type == Core::VolumeType::DATA_E )
    {
      error = std::string( "Layer '") + layer_id + "' is not a data layer.";
    }
    else if ( type == Core::VolumeType::MASK_E )
    {
      error = std::string( "Layer '") + layer_id + "' is not a mask layer.";
    }
    else if ( type == Core::VolumeType::LABEL_E )
    {
      error = std::string( "Layer '") + layer_id + "' is not a label layer.";
    }
    else
    {
      error = std::string( "Layer '") + layer_id + "' is of an incorrect type.";
    }
    return false;
  }
  
  return true;
}

bool LayerManager::CheckLayerSize( const std::string& layer_id1, const std::string& layer_id2,
    std::string& error )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "CheckLayerSize can only be called from the"
      " application thread." );
  }

  // Clear error string
  error = "";
  
  // Check whether layer exists
  LayerHandle layer1 = LayerManager::Instance()->get_layer_by_id( layer_id1 );
  if ( !layer1 )
  {
    error = std::string( "Incorrect layerid: layer '") + layer_id1 + "' does not exist.";
    return false;
  }

  LayerHandle layer2 = LayerManager::Instance()->get_layer_by_id( layer_id2 );
  if ( !layer2 )
  {
    error = std::string( "Incorrect layerid: layer '") + layer_id2 + "' does not exist.";
    return false;
  }
  
  if ( layer1->get_grid_transform() != layer2->get_grid_transform() )
  {
    error = std::string( "Layer '" ) + layer_id1 + "' and layer '" + layer_id2 + 
      "' are not of the same size and origin.";
    return false;
  }
  
  return true;
}

bool LayerManager::CheckLayerAvailabilityForProcessing( const std::string& layer_id, 
    Core::NotifierHandle& notifier )
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
  if ( layer_state == Layer::AVAILABLE_C )
  {
    notifier.reset();
    return true;
  }
  else
  {
    // This notifier will inform the calling process when the layer will be available again.
    notifier = Core::NotifierHandle( new LayerAvailabilityNotifier( layer ) );
    return false; 
  }
}

bool LayerManager::CheckLayerAvailabilityForUse( const std::string& layer_id, 
    Core::NotifierHandle& notifier )
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

  // TODO: Need to implement the case that the layer is IN_USE_E, in which case we
  // should allow the use of the layer.
  std::string layer_state = layer->data_state_->get();
  if ( layer_state == Layer::AVAILABLE_C )
  {
    notifier.reset();
    return true;
  }
  else
  {
    // This notifier will inform the calling process when the layer will be available again.
    notifier = Core::NotifierHandle( new LayerAvailabilityNotifier( layer ) );
    return false; 
  }
}

bool LayerManager::CheckLayerAvailability( const std::string& layer_id, bool replace,
    Core::NotifierHandle& notifier )
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
    return LayerManager::CheckLayerAvailabilityForProcessing( layer_id, notifier );
  }
  else
  {
    return LayerManager::CheckLayerAvailabilityForUse( layer_id, notifier );  
  }
}

bool LayerManager::LockForUse( LayerHandle layer )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "LockForUse can only be called from the"
      " application thread." );
  }
  
  if ( layer->data_state_->get() != Layer::AVAILABLE_C ) return false;

  layer->data_state_->set( Layer::IN_USE_C );
  return true;
}

bool LayerManager::LockForProcessing( LayerHandle layer )
{
  // NOTE: Security check to keep the program logic sane
  // Only the Application Thread guarantees that nothing is changed in the program
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    CORE_THROW_LOGICERROR( "LockForProcessing can only be called from the"
      " application thread." );
  }
  
  if ( layer->data_state_->get() != Layer::AVAILABLE_C ) return false;

  layer->data_state_->set( Layer::PROCESSING_C );
  return true;
}

bool LayerManager::CreateAndLockMaskLayer( Core::GridTransform transform, const std::string& name, 
    LayerHandle& layer )
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
  
  // Insert the layer into the layer manager.
  LayerManager::Instance()->insert_layer( layer );
  
  return true;
}

bool LayerManager::CreateAndLockDataLayer( Core::GridTransform transform, const std::string& name, 
    LayerHandle& layer )
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
  
  // Insert the layer into the layer manager.
  LayerManager::Instance()->insert_layer( layer );
  
  return true;
}

void LayerManager::DispatchDeleteLayer( LayerHandle layer )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchDeleteLayer, layer) );
    return;
  }

  // Insert the layer into the layer manager.
  LayerManager::Instance()->delete_layer( layer );
}

void LayerManager::DispatchUnlockLayer( LayerHandle layer )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &LayerManager::DispatchUnlockLayer, layer) );
    return;
  }

  layer->data_state_->set( Layer::AVAILABLE_C );
}

void LayerManager::DispatchInsertDataVolumeIntoLayer( DataLayerHandle layer, 
  Core::DataVolumeHandle data )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertDataVolumeIntoLayer, layer, data ) );
    return;
  }
  
  layer->set_data_volume( data );
}

void LayerManager::DispatchInsertMaskVolumeIntoLayer( MaskLayerHandle layer, 
  Core::MaskVolumeHandle mask )
{
  // Move this request to the Application thread
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( 
      &LayerManager::DispatchInsertMaskVolumeIntoLayer, layer, mask ) );
    return;
  }
  
  layer->set_mask_volume( mask );
}

} // end namespace seg3D
