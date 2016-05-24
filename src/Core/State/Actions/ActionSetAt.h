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

#ifndef CORE_STATE_ACTIONS_ACTIONSETAT_H
#define CORE_STATE_ACTIONS_ACTIONSETAT_H

#include <Core/Action/Actions.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/State/StateVector.h>

namespace Core
{

class ActionSetAt : public Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "SetAt", "Set the value at the given position of a vector state." )
  CORE_ACTION_ARGUMENT( "stateid", "The stateid of the vector state" )
  CORE_ACTION_ARGUMENT( "index", "The index of the value to be changed" )
  CORE_ACTION_ARGUMENT( "value", "The new value." )
)

public:
  ActionSetAt()
  {
    this->add_parameter( this->stateid_ );
    this->add_parameter( this->index_ );
    this->add_parameter( this->value_ );
  } 

  // -- Functions that describe action --
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

  // -- Function that describes whether the action changes the data of the program --
  virtual bool changes_project_data();


private:
  std::string stateid_;
  size_t index_;
  Variant value_;

  StateVectorBaseWeakHandle state_weak_handle_;

public:
  // DISPATCH:
  // Dispatch the action from the interface
  template< class T >
  static void Dispatch( ActionContextHandle context,
    const typename StateVector< T >::handle_type& state, size_t index, const T& value )
  {
    ActionSetAt* action = new ActionSetAt;
    action->stateid_ = state->get_stateid();
    action->index_ = index;
    action->value_.set( value );
    action->state_weak_handle_ = state;

    ActionDispatcher::PostAction( ActionHandle( action ), context );
  }
};

} // end namespace Core

#endif
