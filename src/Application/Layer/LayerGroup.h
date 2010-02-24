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

// Utils includes
#include <Utils/Geometry/GridTransform.h>

// Application includes
#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>

#include <Application/Action/Action.h>
#include <Application/State/State.h>

#include <Application/Layer/Layer.h>

namespace Seg3D {

// CLASS LayerGroup
// This is the class that records the layers that are grouped together

// Forward declarations
class LayerGroup;
typedef boost::shared_ptr<LayerGroup> LayerGroupHandle;

// Class definition
class LayerGroup : public StateHandler {
    
// -- constructor/destructor --    
  public:

    LayerGroup(std::string& name);
    virtual ~LayerGroup();

// -- state variables --
  public:
    
    // This mode records whether the state of the group is being modified
    // and how: NONE, TRANSFORM, CROP, RESAMPLE
    StateOptionHandle edit_mode_state_;
    
    // = Transformation menu state variables =
    
    // Origin where the center of the dataset should
    StateDoubleHandle transform_origin_x_state_;
    StateDoubleHandle transform_origin_y_state_;
    StateDoubleHandle transform_origin_z_state_;

    // The new Size information
    StateDoubleHandle transform_size_x_state_;
    StateDoubleHandle transform_size_y_state_;
    StateDoubleHandle transform_size_z_state_;
    
    StateBoolHandle   transform_replace_state_;
    
    // = Crop menu state variables =
    
    StateDoubleHandle crop_min_x_state_;
    StateDoubleHandle crop_max_x_state_;
    
    StateDoubleHandle crop_min_y_state_;
    StateDoubleHandle crop_max_y_state_;
    
    StateDoubleHandle crop_min_z_state_;
    StateDoubleHandle crop_max_z_state_;

    StateBoolHandle   crop_replace_state_;

    // = Resample state variables =
    
    StateDoubleHandle resample_factor_state_;
    
    StateBoolHandle   resample_replace_state_;

    // = Group visibility =
    
    StateBoolHandle   visibility_state_;
    

// -- Group transformation information --
  
  public:
    // GRID_TRANSFORM
    // Get the transform of the layer  
    const GridTransform& grid_transform() const { return grid_transform_; }
    
  private:
    // The transformation that describes the grid dimensions and the spacing
    // information
    GridTransform grid_transform_;
    
// -- Layers contained within group --
  protected:
    // NOTE: Only the layer manager groups the layers, this is all done
    // dynamically
    friend class LayerManager;
    
    // DELETE_LAYER:
    // Delete a layer from the list
    void delete_layer(LayerHandle layer);
    
    // INSERT_LAYER_TOP:
    // Inserts a layer in the top most position
    void insert_layer_top(LayerHandle new_layer);
    
    // INSERT_LAYER_ABOVE:
    // Inserts a layer above a certain layer
    void insert_layer_above(LayerHandle new_layer, LayerHandle layer);
  
    // MOVE_LAYER:
    // Moves a layer up or down
    void move_layer_above(LayerHandle move_layer, LayerHandle layer);
    
  private:
    typedef std::list<LayerHandle> layer_list_type;
    
    // The list that contains the layers that are stored in this class
    layer_list_type layer_list_;
    
};

} // end namespace Seg3D

#endif
