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

#ifndef CORE_STATE_ACTIONS_ACTIONADD_H
#define CORE_STATE_ACTIONS_ACTIONADD_H

#include <Core/Action/Actions.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/State/StateSet.h>
#include <Core/State/StateVector.h>

namespace Core
{

class ActionAdd : public Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "Add", "Add an item to a vector or set state.")
  CORE_ACTION_ARGUMENT( "stateid", "The stateid of the state")
  CORE_ACTION_ARGUMENT( "value", "The value that needs to be added." )
)

public:
  ActionAdd()
  {
    this->add_parameter( this->stateid_ );
    this->add_parameter( this->value_ );
  } 

  // -- Functions that describe action --
  virtual bool validate( ActionContextHandle& context ) override;
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result ) override;

  // -- Function that describes whether the action changes the data of the program --
  virtual bool changes_project_data() override;

private:
  std::string stateid_;
  Variant   value_;

  StateBaseWeakHandle state_weak_handle_;

public:
  template< class HANDLE, class T >
  static ActionHandle Create( const HANDLE& state, const T& value );

  template< class T >
  static void Dispatch( ActionContextHandle context,
    StateVectorBaseHandle state, const T& value );

  template< class T >
  static void Dispatch( ActionContextHandle context,
    StateSetBaseHandle state, const T& value );
};

template< class HANDLE, class T >
ActionHandle ActionAdd::Create( const HANDLE& state, const T& value )
{
  ActionAdd* action = new ActionAdd;
  action->stateid_ = state->get_stateid();
  action->value_.set( value );
  action->state_weak_handle_ = state;

  return ActionHandle( action );
}

template< class T >
void ActionAdd::Dispatch( ActionContextHandle context, 
    StateVectorBaseHandle state, const T& value )
{
  ActionDispatcher::PostAction( Create( state, value ), context );
}

template< class T >
void ActionAdd::Dispatch( ActionContextHandle context, 
    StateSetBaseHandle state, const T& value )
{
  ActionDispatcher::PostAction( Create( state, value ), context );
}

} // end namespace Core

#endif
