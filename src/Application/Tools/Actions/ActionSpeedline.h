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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIOINSPEEDLINE_H
#define APPLICATION_TOOLS_ACTIONS_ACTIOINSPEEDLINE_H

// Core includes
#include <Core/Volume/VolumeSlice.h>

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/LayerAction.h>

namespace Seg3D
{

class ActionSpeedlineAlgo;
typedef boost::shared_ptr< ActionSpeedlineAlgo > ActionSpeedlineAlgoHandle;

class ActionSpeedline : public LayerAction
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "Speedline", "Fill or erase a slice of a mask layer within "
                    "the region enclosed by the Speedline.")
  CORE_ACTION_ARGUMENT( "target", "The ID of the target mask layer." )
  CORE_ACTION_ARGUMENT( "slice_type", "The slicing direction to be painted on." )
  CORE_ACTION_ARGUMENT( "slice_number", "The slice number to be painted on." )
  CORE_ACTION_ARGUMENT( "erase", "Whether to erase." )
  CORE_ACTION_ARGUMENT( "vertices", "The 2D coordinates of Speedline vertices." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)

typedef Core::Point VertexCoord;

public:
  ActionSpeedline();

  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context );

  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

  // CLEAR_CACHE:
  // Clear any objects that were given as a short cut to improve performance.
  //virtual void clear_cache();

private:
  //ActionSpeedlineAlgoHandle algo_;

  std::string target_layer_id_;
  int slice_type_;
  size_t slice_number_;
  bool erase_;
  std::vector< ActionSpeedline::VertexCoord > vertices_;

  DataLayerHandle target_layer_;
  Core::DataVolumeSliceHandle vol_slice_;

  int iterations_;
  double termination_;
  std::string speedline_tool_id_;

public:
  //static void Dispatch( Core::ActionContextHandle context, const std::string& layer_id,
  //  Core::VolumeSliceType slice_type, size_t slice_number, bool erase,
  //  const std::vector< VertexCoord >& vertices );

  static void Dispatch( Core::ActionContextHandle context, const std::string& layer_id,
    Core::VolumeSliceType slice_type, size_t slice_number, bool erase,
    const std::vector< VertexCoord >& vertices, int iterations, double termination,
    std::string tool_id );
};

} // end namespace Seg3D

#endif
