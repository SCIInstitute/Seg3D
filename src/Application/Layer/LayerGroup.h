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

#ifndef APPLICATION_LAYER_LAYERGROUP_H
#define APPLICATION_LAYER_LAYERGROUP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <string>
#include <vector>
#include <list>

// Boost includes 
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// Core includes
#include <Core/Geometry/GridTransform.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Action/Action.h>
#include <Core/State/State.h>

// Application includes
#include <Application/Layer/Layer.h>

namespace Seg3D
{

// CLASS LayerGroup
// This is the class that records the layers that are grouped together

// Forward declarations
typedef std::list< LayerHandle > layer_list_type;

// Class definition
class LayerGroup : public Core::StateHandler
{

  // -- constructor/destructor --
public:

  LayerGroup( Core::GridTransform grid_transform );
  virtual ~LayerGroup();

  // -- state variables --
public:

  // This mode records whether the state of the group is being modified
  // and how: NONE, TRANSFORM, CROP, RESAMPLE
  Core::StateOptionHandle edit_mode_state_;

  // = Transformation menu state variables =

  // Origin where the center of the dataset should
  Core::StateDoubleHandle transform_origin_x_state_;
  Core::StateDoubleHandle transform_origin_y_state_;
  Core::StateDoubleHandle transform_origin_z_state_;

  // The new Size information
  Core::StateDoubleHandle transform_spacing_x_state_;
  Core::StateDoubleHandle transform_spacing_y_state_;
  Core::StateDoubleHandle transform_spacing_z_state_;

  Core::StateBoolHandle transform_replace_state_;

  // = Crop menu state variables =
  Core::StateRangedDoubleHandle crop_center_x_state_;
  Core::StateRangedDoubleHandle crop_center_y_state_;
  Core::StateRangedDoubleHandle crop_center_z_state_;
  
  Core::StateRangedDoubleHandle crop_size_width_state_;
  Core::StateRangedDoubleHandle crop_size_height_state_;
  Core::StateRangedDoubleHandle crop_size_depth_state_;

  Core::StateBoolHandle crop_replace_state_;

  // = Resample state variables =
  Core::StateRangedDoubleHandle resample_factor_state_;
  Core::StateBoolHandle resample_replace_state_;

  // = Group visibility =

  Core::StateBoolHandle visibility_state_;
  Core::StateBoolHandle show_layers_state_;
  
  // = Layers
  //Core::StateStringVectorHandle layers_state_;


  // -- Layers contained within group --
protected:
  // NOTE: Only the layer manager groups the layers, this is all done
  // dynamically
  friend class LayerManager;
  
  // CREATE_LAYER:
  // Creates a new layer in the correct position
  void create_mask_layer();

  // INSERT_NEW_LAYER_BACK:
  // Inserts a layer in the top most position
  void insert_layer_back( LayerHandle new_layer );
  
  // INSERT_NEW_LAYER_FRONT:
  // Inserts a layer in the top most position
  void insert_layer_front( LayerHandle new_layer );

  // INSERT_LAYER_ABOVE:
  // Inserts a layer above a certain layer
    int move_layer_above( LayerHandle layer_above, LayerHandle layer_below );
  
  // DELETE_LAYER:
  // Delete a layer from the list
  void delete_layer( LayerHandle layer );
  
  // CROP_LAYER:
  // Crops the selected layers based on the state of the crop panel
  void crop_layer();
    
  // TRANSFORM_LAYER:
  // Performs a transform on the selected layers based on the state of the transform panel
  void transform_layer();
  
  // RESAMPLE_LAYER:
  // Performs a resample on the selected layers based on the state of the resample panel
  void resample_layer();
  
  // FLIP_LAYER:
  // Performs a flip or rotate on the selected layers based on which layers were selected
  void flip_layer();
  
  //TODO add move_layer function();

  // -- Group transformation information --
public:
  // GRID_TRANSFORM
  // Get the transform of the layer
  const Core::GridTransform& get_grid_transform() const
  {
    return grid_transform_;
  }
  
  std::string get_group_id() const
  {
    return get_statehandler_id();
  }
  
  layer_list_type get_layer_list() const
  {
    return layer_list_;
  }

  
  
  size_t get_list_size() const
  {
    return layer_list_.size();
  }
  
  bool is_empty() const
  {
    return layer_list_.empty();
  }

  void get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
    Core::VolumeType type ) const;
  void get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
    Core::VolumeType type, LayerHandle excluded_layer ) const;
  void get_layer_names( std::vector< std::string >& layer_ids );
  
private:

/*  void destroy_layers();*/

private:
  
  // The transformation that describes the grid dimensions and the spacing
  Core::GridTransform grid_transform_;
  
  // The list that contains the layers that are stored in this class
  layer_list_type layer_list_;

};

} // end namespace Seg3D

#endif
