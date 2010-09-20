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
  Core::StateOptionHandle menu_state_;

  // = Isosurface state variables =
  Core::StateOptionHandle isosurface_quality_state_;

  // = Group visibility =
  Core::StateBoolHandle visibility_state_;
  Core::StateBoolHandle show_layers_state_;
  
  // -- Layers contained within group --
protected:
  // NOTE: Only the layer manager groups the layers, this is all done
  // dynamically
  friend class LayerManager;

  // INSERT_LAYER:
  // Inserts a layer in the correct position
  void insert_layer( LayerHandle new_layer );

  // INSERT_LAYER_ABOVE:
  // Inserts a layer above a certain layer
    void move_layer_above( LayerHandle layer_above, LayerHandle layer_below );
  
  // DELETE_LAYER:
  // Delete a layer from the list
  void delete_layer( LayerHandle layer, bool invalidate_layerid = true  );
  
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

private:
  
  // The transformation that describes the grid dimensions and the spacing
  Core::GridTransform grid_transform_;
  
  // The list that contains the layers that are stored in this class
  layer_list_type layer_list_;
  
public:
  // Options for the state of the menus that are open
  const static std::string NO_MENU_C;
  const static std::string CROP_MENU_C;
  const static std::string FLIP_ROTATE_MENU_C;
  const static std::string RESAMPLE_MENU_C;
  const static std::string TRANSFORM_MENU_C;
  const static std::string ISO_MENU_C;
  const static std::string DELETE_MENU_C;
};

} // end namespace Seg3D

#endif
