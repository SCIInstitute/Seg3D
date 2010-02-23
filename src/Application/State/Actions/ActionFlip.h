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

#ifndef APPLICATION_STATE_ACTIONS_ACTIONFLIP_H
#define APPLICATION_STATE_ACTIONS_ACTIONFLIP_H

#include <Application/Action/Action.h>
#include <Application/Interface/Interface.h>
#include <Application/State/StateView2D.h>

namespace Seg3D
{
  class ActionFlip : public Action
  {
    SCI_ACTION_TYPE("Flip", "Flip <key> <direction>", APPLICATION_E)

  public:
    ActionFlip();
    virtual ~ActionFlip();

    virtual bool validate(ActionContextHandle& context);
    virtual bool run(ActionContextHandle& context, ActionResultHandle& result);

  private:
    ActionParameter<std::string> stateid_;
    ActionParameter<int> direction_;

    StateView2DWeakHandle view2d_state_;

  public:
    static void Dispatch(StateView2DHandle& state, Utils::View2D::Direction direction);
  };
} // end namespace Seg3D

#endif