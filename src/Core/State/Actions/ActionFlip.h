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

#ifndef CORE_STATE_ACTIONS_ACTIONFLIP_H
#define CORE_STATE_ACTIONS_ACTIONFLIP_H

#include <Core/Action/Action.h>
#include <Core/Interface/Interface.h>
#include <Core/State/StateView2D.h>

namespace Core
{
class ActionFlip : public Action
{

CORE_ACTION( 
CORE_ACTION_TYPE("FlipView","This action flip the state in a view state variable.")
CORE_ACTION_ARGUMENT("stateid","The name of the state variable.")
CORE_ACTION_ARGUMENT("direction","Whether to flip in x or y.")
)

  // -- Constructor/Destructor --
public:
  ActionFlip();
  virtual ~ActionFlip();

  // -- Functions that describe action --
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

private:
  ActionParameter< std::string > stateid_;
  ActionParameter< int > direction_;

  StateView2DWeakHandle view2d_state_;

  // -- Create and dispatch this action --
public:

  // CREATE:
  // Create the action but do not dispatch it yet
  static ActionHandle Create( StateView2DHandle& state, Core::FlipDirectionType direction  );

  // DISPATCH:
  // Dispatch the action from the specified context
  static void Dispatch( ActionContextHandle context, StateView2DHandle& state, 
    Core::FlipDirectionType direction );
};
} // end namespace Core

#endif
