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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIOINFLOODFILL_H
#define APPLICATION_TOOLS_ACTIONS_ACTIOINFLOODFILL_H

// Core includes
#include <Core/Action/Action.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerAction.h>

namespace Seg3D
{

class ActionFloodFillPrivate;
typedef boost::shared_ptr< ActionFloodFillPrivate > ActionFloodFillPrivateHandle;

class FloodFillInfo
{
public:
  std::string target_layer_id_;
  int slice_type_;
  size_t slice_number_;
  std::vector< Core::Point > seeds_;
  std::string data_constraint_layer_id_;
  double min_val_;
  double max_val_;
  bool negative_data_constraint_;
  std::string mask_constraint1_layer_id_;
  bool negative_mask_constraint1_;
  std::string mask_constraint2_layer_id_;
  bool negative_mask_constraint2_;
  bool erase_;
};

class ActionFloodFill : public LayerAction
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "FloodFill", "Flood fill the content of a mask slice "
    "starting from seed points." )
  CORE_ACTION_ARGUMENT( "target", "The ID of the target mask layer." )
  CORE_ACTION_ARGUMENT( "slice_type", "The slicing direction." )
  CORE_ACTION_ARGUMENT( "slice_number", "The slice number to be filled." )
  CORE_ACTION_ARGUMENT( "seed_points", "The world coordinates of seed points." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "data_constraint", "<none>", "The ID of data constraint layer." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "min_value", "0", "The minimum data constraint value." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "max_value", "0", "The maximum data constraint value." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "negative_data_constraint", "false", "Whether to negate the data constraint." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mask_constraint1", "<none>", "The ID of first mask constraint layer." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "negative_mask_constraint1", "false", "Whether to negate the first mask constraint." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mask_constraint2", "<none>", "The ID of second mask constraint layer." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "negative_mask_constraint2", "false", "Whether to negate the second mask constraint." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "erase", "false", "Whether to erase instead of fill." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)

public:
  ActionFloodFill();

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
  virtual void clear_cache();

private:
  ActionFloodFillPrivateHandle private_;

public:
  // DISPATCH:
  // Dispatch the action.
  static void Dispatch( Core::ActionContextHandle context, const FloodFillInfo& params );
};

} // end namespace Seg3D

#endif
