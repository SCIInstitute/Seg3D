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


#include <Application/Layer/LayerGroup.h>

namespace Seg3D
{

  LayerGroup::LayerGroup( const std::string &name, Utils::GridTransform grid_transform ) :
  StateHandler( name )
  {
    group_id_ = name;
    
    grid_transform_ = grid_transform;
    
    
    
    // Need to set ranges and default values for all parameters
    add_state( "mode", edit_mode_state_, "none", "none|TRANSFORM|COPY|RESAMPLE" );
    
    
    // = Transformation menu state variables =
    add_state( "origin_x", transform_origin_x_state_ , 0.0 );
    add_state( "origin_y", transform_origin_y_state_ , 0.0 );
    add_state( "origin_z", transform_origin_z_state_ , 0.0 );
    add_state( "size_x", transform_size_x_state_, 0.0);
    add_state( "size_y", transform_size_y_state_, 0.0);
    add_state( "size_z", transform_size_z_state_, 0.0);
    add_state( "transform_replace", transform_replace_state_, false );
    
    
    // = Set Crop menu state variables =
    add_state( "min_x", crop_min_x_state_, 0.0 );
    add_state( "min_y", crop_min_y_state_, 0.0 );
    add_state( "min_z", crop_min_z_state_, 0.0 );
    add_state( "max_x", crop_max_x_state_, 0.0 );
    add_state( "max_y", crop_max_y_state_, 0.0 );
    add_state( "max_z", crop_max_z_state_, 0.0 );
    add_state( "crop_replace", crop_replace_state_, 0.0 );
    
    
    // = Set Resample state variables =
    add_state( "factor", resample_factor_state_, 1.0);
    add_state( "resample_replace", resample_replace_state_, false );
    
  }
  
  LayerGroup::~LayerGroup()
  {
    // Disconnect all current connections
    disconnect_all();
  }
  

  void LayerGroup::create_mask_layer()
  {
    
  }

  void LayerGroup::insert_layer( LayerHandle new_layer )
  {
    layer_list_.push_back( new_layer );
  }

  void LayerGroup::insert_layer_above( LayerHandle layer, LayerHandle layer_below )
  {
    
  }

  void LayerGroup::delete_layer( LayerHandle layer )
  {
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
  
} // end namespace Seg3D