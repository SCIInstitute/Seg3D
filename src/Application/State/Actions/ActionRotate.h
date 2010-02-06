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
#include <Utils/Geometry/Quaternion.h>

// Application includes
#include <Application/Action/Action.h>
#include <Application/Interface/Interface.h>
#include <Application/State/StateView3D.h>

namespace Seg3D {

class ActionRotate : public Action {
  SCI_ACTION_TYPE("Rotate", "Rotate <key> <rotation>", APPLICATION_E)

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
    
  private:
    ActionParameter<std::string> stateid_;
    ActionParameter<StateView3DWeakHandle> state_;
    ActionParameter<Utils::Quaternion> rotation_;
    
  public:
    static void Dispatch(StateView3DHandle& view3d_state, const Utils::Quaternion& rotation)
    {
      ActionRotate* action = new ActionRotate;
      
      action->stateid_.value() = view3d_state->stateid();
      action->state_.value() = StateView3DWeakHandle(view3d_state);
      action->rotation_.set_value(rotation);
      
      PostActionFromInterface(ActionHandle(action));
    }
};

} // End namespace Seg3D

#endif