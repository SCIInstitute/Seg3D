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

#include <Boost/lexical_cast.hpp>
#include <vector>

#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionInsertLayer.h>


#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>


namespace Seg3D
{

LayerManager::LayerManager() :
  StateHandler( "LayerManager" )
{ 

  for( int i = 0; i < 2; i++ )
  {
    std::string new_name;
    new_name = "new_layer_" + boost::lexical_cast< std::string >( i );

    Utils::VolumeHandle volume0;
    MaskLayerHandle mask_layer_new0;
    
    mask_layer_new0 = MaskLayerHandle ( new MaskLayer( new_name, volume0 ) );
    mask_layer_new0->set_grid_transform( 256, 640, 480 );
    LayerHandle plain_layer = mask_layer_new0;
    //insert_layer( plain_layer );
    ActionInsertLayer::Dispatch( plain_layer );
    
  }
}

LayerManager::~LayerManager()
{
}
  
bool LayerManager::insert_layer( LayerHandle layer )
{
    
  
  
  lock_type lock( group_handle_list_mutex_ );
  
  SCI_LOG_DEBUG( std::string("Insert new layer: ") + layer->get_layer_id());
  
  for ( group_handle_list_type::iterator i = group_handle_list_.begin(); 
     i != group_handle_list_.end(); ++i )
  {
          
    if (layer->get_grid_transform() == ( *i )->get_grid_transform()) 
    {
      ( *i )->insert_layer( layer );
      layer->set_layer_group( *i );
      layer_inserted_signal_( layer );
      return true;
    }
  }
  
  //TODO add logic for handling where the layer should good
  
  LayerGroupHandle new_group = create_group( layer->get_grid_transform() );
  layer->set_layer_group( new_group );
  new_group->insert_layer( layer );
  group_handle_list_.push_back( new_group );
  
  // Send a signal alerting the UI that we have inserted a layer
  group_layers_changed_signal_( new_group );

  return true;
}
  
  bool LayerManager::insert_layer( LayerGroupHandle group )
  {
    
    return true;
  
  }
  
  LayerGroupHandle LayerManager::check_for_group( std::string group_id )
  {
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
  std::string new_name;
  StateEngine::Instance()->create_stateid( "group", new_name );
  return LayerGroupHandle( new LayerGroup( new_name, transform  ));
}


  void LayerManager::return_group_vector( std::vector< LayerGroupHandle > &vector_of_groups )
  {
    for( group_handle_list_type::iterator i = group_handle_list_.begin(); 
      i != group_handle_list_.end(); ++i )
    {
      vector_of_groups.push_back( *i );
    }
  }
  
  
void LayerManager::insert_layer_top( LayerHandle layer )
{
  
}

void LayerManager::set_active_layer( LayerHandle layer )
{
  active_layer_ = layer;
  active_layer_changed_signal_( layer );
}

void LayerManager::delete_layer( LayerHandle layer )
{
  LayerGroupHandle group = layer->get_layer_group();
  if ( group ){
    group->delete_layer( layer );
  }
} // end delete_layer

LayerHandle LayerManager::get_active_layer()
{
  return active_layer_;
}

LayerGroupWeakHandle LayerManager::get_active_group()
{
  return active_layer_->get_layer_group();
}

LayerManager::mutex_type&
LayerManager::get_mutex()
{
  return group_handle_list_mutex_;
}
  
  

} // end namespace seg3D
