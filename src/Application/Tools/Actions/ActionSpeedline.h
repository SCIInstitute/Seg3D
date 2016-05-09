/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIOINSPEEDLINE_H
#define APPLICATION_TOOLS_ACTIONS_ACTIOINSPEEDLINE_H

// Core includes
#include <Core/Volume/VolumeSlice.h>
#include <Core/Utils/AtomicCounter.h>

// Application includes
#include <Application/Layer/LayerAction.h>

namespace Seg3D
{

class ActionSpeedlinePrivate;
typedef boost::shared_ptr< ActionSpeedlinePrivate > ActionSpeedlinePrivateHandle;

class ActionSpeedline : public LayerAction
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "Speedline", "Fill or erase a slice of a mask layer within "
                    "the region enclosed by the Speedline.")
  CORE_ACTION_ARGUMENT( "target", "The ID of the target data layer." )
  CORE_ACTION_ARGUMENT( "mask", "The layerid of the mask that needs to be applied." )
  CORE_ACTION_ARGUMENT( "slice_type", "The slicing direction to be painted on." )
  CORE_ACTION_ARGUMENT( "slice_number", "The slice number to be painted on." )
  CORE_ACTION_ARGUMENT( "vertices", "The 2D coordinates of free Speedline vertices." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "roi_mask", "<none>", "Region of interest." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "grad_mag_weight", "0.43", "Gradient magnitude weight for cost function." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "zero_cross_weight", "0.43", "Zero cross weight for cost function." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "grad_dir_weight", "0.14", "Gradient direction weight for cost function." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "image_spacing", "true", "Determines the scaling factor for the neighborhood weighting." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "face_conn", "true", "Determines the local neighborhood." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "world_path_state_id", "", "The statid of the state variable into which world coordinate path values will be written." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)

public:
  ActionSpeedline();

  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context ) override;

  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result ) override;

//  // CLEAR_CACHE:
//  // Clear any objects that were given as a short cut to improve performance.
//  virtual void clear_cache() override;

private:
  ActionSpeedlinePrivateHandle private_;

public:
  static void Dispatch( Core::ActionContextHandle context,
                        const std::string& layer_id,
                        const std::string& mask_id,
                        const std::string& roi_mask_id,
                        Core::VolumeSliceType slice_type,
                        size_t slice_number,
                        const std::vector< Core::Point > vertices,
                        const double grad_mag_weight,
                        const double zero_cross_weight,
                        const double grad_dir_weight,
                        const bool image_spacing,
                        const bool face_conn,
                        const std::string& world_path_state_id
                      );

};

} // end namespace Seg3D

#endif
