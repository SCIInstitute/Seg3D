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

#ifndef APPLICATION_STATE_ACTIONS_ACTIONROTATE_H
#define APPLICATION_STATE_ACTIONS_ACTIONROTATE_H

// Utils includes
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Quaternion.h>

// Application includes
#include <Application/Action/Action.h>
#include <Application/Interface/Interface.h>
#include <Application/State/StateView3D.h>

namespace Seg3D {

class ActionRotate : public Action {
  SCI_ACTION_TYPE("Rotate", "Rotate <key> <axis> <angle>", APPLICATION_E)

  public:
    ActionRotate()
    {
      add_argument(stateid_);
      add_argument(state_);
      add_argument(rotation_);
    }
    
    virtual ~ActionRotate() {}
    
    virtual bool validate(ActionContextHandle& context);
    virtual bool run(ActionContextHandle& context, ActionResultHandle& result);
    

// -- Action parameters --
  private:
    // This one describes where the state is located
    ActionParameter<std::string>    stateid_;
    // The axis around which the rotation is performed
    ActionParameter<Utils::Vector>  axis_;
    // The angle of rotation
    ActionParameter<Utils::Vector>  angle_;

// -- Action optimization --
  private:
    // This is an internal optimization to avoid the lookup in the state
    // database
    StateBaseWeakHandle state_weak_handle_;
    
  public:
    template <class HANDLE>
    static void Create(HANDLE& state, Utils::Vector axis, double angle)
    {
      ActionRotate* action = new ActionRotate;
      action->stateid_ = state->stateid();
      action->axis_.value() = axis;
      action->angle_.value() = angle;
      
      // Add optimization
      action->state_weak_handle_ = state;
      
      return ActionHandle(action);
    }
  
    template <class HANDLE>
    static void Dispatch(HANDLE& state, Utils::Vector axis, double angle)
    {
      PostActionFromInterface(Create(state,axis,angle));
    }
};

} // End namespace Seg3D

#endif