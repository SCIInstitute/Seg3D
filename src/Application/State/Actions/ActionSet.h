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

#ifndef APPLICATION_STATE_ACTIONS_ACTIONSET_H
#define APPLICATION_STATE_ACTIONS_ACTIONSET_H

#include <Application/Action/Action.h>
#include <Application/Interface/Interface.h>
#include <Application/State/StateBase.h>

namespace Seg3D {

// CLASS ActionSet
// Set the value of a state variable

class ActionSet : public Action {
    SCI_ACTION_TYPE("Set","Set <state> <value>",APPLICATION_E)

// -- Constructor/Destructor --
  public:
    ActionSet()
    {
      add_argument(stateid_);
      add_argument(state_value_);
    }
    
    virtual ~ActionSet() {}
    
// -- Functions that describe action --
    virtual bool validate(ActionContextHandle& context);
    virtual bool run(ActionContextHandle& context, 
                     ActionResultHandle& result);

// -- Action parameters --
  private:
    // This one describes where the state is located
    ActionParameter<std::string> stateid_;

    // This one describes the value of the state variable
    ActionParameterVariant       state_value_;

// -- Action optimization --
  private:
    // This is an internal optimization to avoid the lookup in the state
    // database
    StateBaseWeakHandle state_weak_handle_;


// -- Dispatch this action from the interface --
  public:

    // CREATE:
    // Create the action but do not dispatch it yet
    template<class HANDLE, class T>
    static ActionHandle Create(HANDLE& state, const T& statevalue)
    {
      // Create new action
      ActionSet* action = new ActionSet;
    
      // Set action parameters
      action->stateid_.value() = state->stateid();
      action->state_value_.set_value(statevalue);    
      
      // Add optimization
      action->state_weak_handle_ = state;
      
      // return the new action
      return ActionHandle(action);
    }
  
    // DISPATCH:
    // Dispatch the action from the interface
    template<class HANDLE, class T>
    static void Dispatch(HANDLE& state, const T& statevalue)
    {
      // Post the new action
      Interface::PostAction(Create(state,statevalue));      
    }

};

} // end namespace Seg3D

#endif

