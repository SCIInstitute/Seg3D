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

#ifndef CORE_STATE_ACTIONS_ACTIONOFFSET_H
#define CORE_STATE_ACTIONS_ACTIONOFFSET_H

#include <Core/Action/Actions.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/State/StateRangedValue.h>

namespace Core
{

class ActionOffset : public Action
{

CORE_ACTION( 
CORE_ACTION_TYPE("Offset","Add an offset to a state variable.")
CORE_ACTION_ARGUMENT("stateid","The name of the state variable.")
CORE_ACTION_ARGUMENT("value","The offset that needs to be added.")
)

public:
  ActionOffset()
  {
    this->add_parameter( this->stateid_ );
    this->add_parameter( this->offset_value_ );
  }

  // -- Functions that describe action --
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

  // -- Function that describes whether the action changes the data of the program --
  virtual bool changes_project_data();

private:
  std::string stateid_;
  Variant offset_value_;

  StateRangedValueBaseWeakHandle state_weak_handle_;

public:
  template< class T >
  static void Dispatch( ActionContextHandle context,  
    const typename StateRangedValue< T >::handle_type& state, const T& offset )
  {
    ActionOffset* action = new ActionOffset;
    action->stateid_ = state->get_stateid();
    action->offset_value_.set( offset );
    action->state_weak_handle_ = state;

    ActionDispatcher::PostAction( ActionHandle( action ), context );
  }

};

} // end namespace Core

#endif
