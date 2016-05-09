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

#ifndef CORE_STATE_ACTIONS_ACTIONTOGGLE_H
#define CORE_STATE_ACTIONS_ACTIONTOGGLE_H

#include <Core/Action/Action.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/State/StateValue.h>

namespace Core
{

// CLASS ActionSet
// Set the value of a state variable

class ActionToggle : public Action
{

CORE_ACTION( 
CORE_ACTION_TYPE( "Toggle", "This action toggles the value of a boolean state variable." )
CORE_ACTION_ARGUMENT( "stateid", "The name of the state variable." )
)

  // -- Constructor/Destructor --
public:
  ActionToggle()
  {
    this->add_parameter( this->stateid_ );
  }

  // -- Functions that describe action --
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

  // -- Function that describes whether the action changes the data of the program --
  virtual bool changes_project_data();

  // -- Action parameters --
private:
  // This one describes where the state is located
  std::string stateid_;

  // -- Action optimization --
private:
  // This is an internal optimization to avoid the lookup in the state
  // database
  StateBoolWeakHandle state_weak_handle_;

  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  // Dispatch the action from the interface
  static void Dispatch( ActionContextHandle context, StateBoolHandle& state )
  {
    // Create new action
    ActionToggle* action = new ActionToggle;

    // Set action parameters
    action->stateid_ = state->get_stateid();

    // Add optimization
    action->state_weak_handle_ = state;

    // Post the new action
    ActionDispatcher::PostAction( ActionHandle( action ), context );
  }
  
};

} // end namespace Core

#endif

