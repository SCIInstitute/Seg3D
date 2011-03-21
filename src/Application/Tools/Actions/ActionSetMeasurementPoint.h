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

#ifndef APPLICATION_TOOLS_ACTIONS_ActionSetMeasurementPoint_H
#define APPLICATION_TOOLS_ACTIONS_ActionSetMeasurementPoint_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>
#include <Core/State/StateValue.h>
#include <Core/State/StateVector.h>

namespace Seg3D
{

class ActionSetMeasurementPointPrivate;
typedef boost::shared_ptr< ActionSetMeasurementPointPrivate > ActionSetMeasurementPointPrivateHandle;

class ActionSetMeasurementPoint : public Core::Action
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "SetMeasurementPoint", "Save the given note to the active measurement.")
  CORE_ACTION_ARGUMENT( "measurements_stateid", "The stateid of the measurements state vector variable." )
  CORE_ACTION_ARGUMENT( "measurement_id", "ID if the specific measurement to be modified." )
  CORE_ACTION_ARGUMENT( "point_index", "Index of the point in measurement (0 or 1) to be modified." )
  CORE_ACTION_ARGUMENT( "world_point", "World point to move the measurement point to." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)

public:
  ActionSetMeasurementPoint();
  virtual ~ActionSetMeasurementPoint();

  // -- Functions that describe action --
public:
  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context );

  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  ActionSetMeasurementPointPrivateHandle private_;

public:
 
  // CREATE:
  // Create action.
  static Core::ActionHandle Create( const Core::StateMeasurementVectorHandle& measurements_state, 
    const std::string& measurement_id, int point_index, const Core::Point& world_point );

  // DISPATCH:
  // Dispatch the action.
  static void Dispatch( Core::ActionContextHandle context, 
    const Core::StateMeasurementVectorHandle& measurements_state, 
    const std::string& measurement_id, int point_index, const Core::Point& world_point );
};

} // end namespace Seg3D

#endif