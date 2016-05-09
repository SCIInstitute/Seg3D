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

#ifndef CORE_STATE_ACTIONS_ACTIONGET_H
#define CORE_STATE_ACTIONS_ACTIONGET_H

#include <Core/Action/Action.h>

namespace Core
{

class ActionGet : public Action
{

CORE_ACTION( 
CORE_ACTION_TYPE( "Get", "Get the value of a state variable." )
CORE_ACTION_ARGUMENT( "stateid", "The name of the state variable." )
);

  // -- Constructor/Destructor --
public:
  ActionGet()
  {
    this->add_parameter( this->stateid_ );
  }

  // -- Functions that describe action --
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

  // -- Action parameters --
private:
  // This one describes where the state is located
  std::string stateid_;

  // -- Action optimization --
private:
  // This is an internal optimization to avoid the lookup in the state
  // database
  StateBaseWeakHandle state_weak_handle_;

  // -- Create and dispatch this action --
public:
  // DISPATCH:
  // Dispatch the action from the specified context
  static void Dispatch( ActionContextHandle context, StateBaseHandle& state );
};

} // end namespace Core

#endif

