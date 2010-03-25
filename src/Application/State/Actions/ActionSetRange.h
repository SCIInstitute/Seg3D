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

#ifndef APPLICATION_STATE_ACTIONS_ACTIONSETRANGE_H
#define APPLICATION_STATE_ACTIONS_ACTIONSETRANGE_H

#include <boost/smart_ptr.hpp>

#include <Application/Action/Action.h>
#include <Application/State/StateRangedValue.h>
#include <Application/Interface/Interface.h>

namespace Seg3D
{

class ActionSetRange : public Action
{
SCI_ACTION_TYPE("SetRange", "SetRange <state> <min> <max>", ActionPropertiesType::APPLICATION_E)

public:
  ActionSetRange();
  virtual ~ActionSetRange() {}

  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

private:
  ActionParameter< std::string > stateid_;
  ActionParameter< double > min_value_;
  ActionParameter< double > max_value_;

  StateBaseWeakHandle state_weak_handle_;

public:
  template< class STATE_HANDLE >
  static void Dispatch( STATE_HANDLE& state_handle, double min_value, double max_value )
  {
    ActionSetRange* action = new ActionSetRange;
    action->stateid_.value() = state_handle->stateid();
    action->min_value_.value() = min_value;
    action->max_value_.value() = max_value;
    action->state_weak_handle_ = state_handle;

    Interface::PostAction( ActionHandle( action ) );
  }
};

} // end namespace Seg3D

#endif