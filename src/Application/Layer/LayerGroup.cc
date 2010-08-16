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

// Core includes
#include <Core/State/StateEngine.h>

// Application includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
//#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>


namespace Seg3D
{

LayerGroup::LayerGroup( Core::GridTransform grid_transform ) :
  StateHandler( "group", true )
{
  this->grid_transform_ = grid_transform;

  // Need to set ranges and default values for all parameters
  add_state( "mode", this->edit_mode_state_, "none", "none|TRANSFORM|COPY|RESAMPLE" );

    // = Transformation menu state variables =
  add_state( "transform_origin_x", this->transform_origin_x_state_, 0.0 );
  add_state( "transform_origin_y", this->transform_origin_y_state_ , 0.0 );
  add_state( "transform_origin_z", this->transform_origin_z_state_ , 0.0 );
  add_state( "transform_spacing_x", this->transform_spacing_x_state_, 1.0 );
  add_state( "transform_spacing_y", this->transform_spacing_y_state_, 1.0 );
  add_state( "transform_spacing_z", this->transform_spacing_z_state_, 1.0 );
  add_state( "transform_replace", this->transform_replace_state_, false );
  
      // = Set Crop menu state variables =
  add_state( "crop_center_x", this->crop_center_x_state_, 0.0, 00.0, 1000.0, 1.0 );
  add_state( "crop_center_y", this->crop_center_y_state_, 0.0, 00.0, 1000.0, 1.0 );
  add_state( "crop_center_z", this->crop_center_z_state_, 0.0, 00.0, 1000.0, 1.0 );
  add_state( "crop_size_x", this->crop_size_width_state_, 0.0, 00.0, 1000.0, 1.0 );
  add_state( "crop_size_y", this->crop_size_height_state_, 0.0, 00.0, 1000.0, 1.0 );
  add_state( "crop_size_z", this->crop_size_depth_state_, 0.0, 00.0, 1000.0, 1.0 );
  add_state( "crop_replace", this->crop_replace_state_, false );
  
  
  // = Set Resample state variables =
  add_state( "resample_factor", this->resample_factor_state_, 1.0, 0.10, 5.0, .10 );
  add_state( "resample_replace", this->resample_replace_state_, false );
  
  // = General state settings
  add_state( "show_layers", this->show_layers_state_, true );
  add_state( "visibility", this->visibility_state_, true );
  
  
}

LayerGroup::~LayerGroup()
{
  // Disconnect all current connections
  disconnect_all();
}


void LayerGroup::create_mask_layer()
{
  
}

void LayerGroup::insert_layer_back( LayerHandle new_layer )
{
  layer_list_.push_back( new_layer );
}

void LayerGroup::insert_layer_front( LayerHandle new_layer )
{
  layer_list_.push_front( new_layer );
}


int LayerGroup::move_layer_above( LayerHandle layer_above, LayerHandle layer_below )
{
  int index = 0;
  
  for( layer_list_type::iterator i = this->layer_list_.begin(); 
    i != this->layer_list_.end(); ++i )
  {
    if( ( *i ) == layer_below )
    { 
      // First we get the size of the list before the insert
      int list_size = static_cast< int >( this->layer_list_.size() );
      
      // Second we insert the layer
      this->layer_list_.insert( ++i, layer_above );
      
      // Finally we return the proper location for the gui to insert the layer
      return abs( index - list_size ) - 1;
    }
    index++;
  }
  // if things didnt work out, we return -1
  return -1 ;
}

void LayerGroup::delete_layer( LayerHandle layer, bool invalidate_layerid )
{
  if ( invalidate_layerid ) layer->invalidate();
  layer_list_.remove( layer );
}

void LayerGroup::crop_layer()
{
  
}

void LayerGroup::transform_layer()
{
  
}

void LayerGroup::resample_layer()
{
  
}

void LayerGroup::flip_layer()
{
  
}

void LayerGroup::get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
  Core::VolumeType type ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  layer_list_type::const_iterator it = this->layer_list_.begin();
  for ( ; it != this->layer_list_.end(); it++ )
  {
    if ( ( *it )->type() == type )
    {
      layer_names.push_back( std::make_pair( ( *it )->get_layer_id(),
        ( *it )->get_layer_name() ) );
    }
  }
}

void LayerGroup::get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
  Core::VolumeType type, LayerHandle excluded_layer ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  layer_list_type::const_iterator it = this->layer_list_.begin();
  for ( ; it != this->layer_list_.end(); it++ )
  {
    if ( *it != excluded_layer && ( *it )->type() == type )
    {
      layer_names.push_back( std::make_pair( ( *it )->get_layer_id(),
        ( *it )->get_layer_name() ) );
    }
  }
}

} // end namespace Seg3D