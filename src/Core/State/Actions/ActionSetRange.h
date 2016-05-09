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

#ifndef CORE_STATE_ACTIONS_ACTIONSETRANGE_H
#define CORE_STATE_ACTIONS_ACTIONSETRANGE_H

#include <boost/smart_ptr.hpp>

#include <Core/Action/Actions.h>
#include <Core/State/StateRangedValue.h>

namespace Core
{

class ActionSetRange : public Action
{
CORE_ACTION(
  CORE_ACTION_TYPE( "SetRange", "This action sets the range of a state variable." )
  CORE_ACTION_ARGUMENT( "stateid", "The name of the state variable." )
  CORE_ACTION_ARGUMENT( "min", "The new minimum value of the state variable." )
  CORE_ACTION_ARGUMENT( "max", "The new maximum value of the state variable." )
)

public:
  ActionSetRange()
  {
    this->add_parameter( this->stateid_ );
    this->add_parameter( this->min_value_ );
    this->add_parameter( this->max_value_ );
  }

  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

private:
  std::string stateid_;
  Variant min_value_;
  Variant max_value_;

  StateRangedValueBaseWeakHandle state_weak_handle_;

public:

  template< class T >
  static void Dispatch( ActionContextHandle context, 
    typename StateRangedValue< T >::handle_type& state_handle, 
    T min_value, T max_value )
  {
    ActionSetRange* action = new ActionSetRange;
    action->stateid_ = state_handle->get_stateid();
    action->min_value_.set( min_value );
    action->max_value_.set( max_value );
    action->state_weak_handle_ = state_handle;

    ActionDispatcher::PostAction( ActionHandle( action ), context );
  }
};

} // end namespace Core

#endif
