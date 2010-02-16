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

#include <Application/State/Actions/ActionScaleView3D.h>

namespace Seg3D
{

SCI_REGISTER_ACTION(ScaleView3D);

ActionScaleView3D::ActionScaleView3D()
{
  add_argument(stateid_);
  add_argument(scale_ratio_);
}

bool ActionScaleView3D::validate( ActionContextHandle& context )
{
  StateBaseHandle state = this->view3d_state_.lock();
  if (!state)
  {
    if (!(StateEngine::Instance()->get_state(stateid_.value(), state)))
    {
      context->report_error(std::string("Unknown state variable '") + stateid_.value() + "'");
      return false;
    }

    if (typeid(*state) != typeid(StateView3D))
    {
      context->report_error(std::string("State variable '") + stateid_.value() 
        + "' doesn't support ActionScaleView3D");
      return false;
    }

    this->view3d_state_ = StateView3DWeakHandle(boost::dynamic_pointer_cast<StateView3D>(state));
  }

  return true;
}

bool ActionScaleView3D::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateView3DHandle state = this->view3d_state_.lock();

  if (state)
  {
    state->scale(this->scale_ratio_.value());
    return true;
  }

  return false;
}

void ActionScaleView3D::Dispatch( StateView3DHandle& view3d_state, double ratio )
{
  ActionScaleView3D* action = new ActionScaleView3D;
  action->stateid_ = view3d_state->stateid();
  action->scale_ratio_ = ratio;
  action->view3d_state_ = StateView3DWeakHandle(view3d_state);

  Interface::PostAction(ActionHandle(action));
}

} // end namespace Seg3D