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

#include <Application/State/Actions/ActionRotateView3D.h>

namespace Seg3D
{

CORE_REGISTER_ACTION( RotateView3D );

ActionRotateView3D::ActionRotateView3D()
{
  add_argument( this->stateid_ );
  add_argument( this->axis_ );
  add_argument( this->angle_ );
}

ActionRotateView3D::~ActionRotateView3D()
{
}

bool ActionRotateView3D::validate( ActionContextHandle& context )
{
  StateBaseHandle state = this->view3d_state_.lock();
  if ( !state )
  {
    if ( !( StateEngine::Instance()->get_state( stateid_.value(), state ) ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + stateid_.value()
          + "'" );
      return false;
    }

    if ( typeid(*state) != typeid(StateView3D) )
    {
      context->report_error( std::string( "State variable '" ) + stateid_.value()
          + "' doesn't support ActionRotateView3D" );
      return false;
    }

    this->view3d_state_ = StateView3DWeakHandle(
        boost::dynamic_pointer_cast< StateView3D >( state ) );
  }

  return true;
}

bool ActionRotateView3D::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateView3DHandle state = this->view3d_state_.lock();

  if ( state )
  {
    state->rotate( this->axis_.value(), this->angle_.value() );
    return true;
  }

  return false;
}

void ActionRotateView3D::Dispatch( StateView3DHandle& view3d_state, const Utils::Vector& axis,
    double angle )
{
  ActionRotateView3D* action = new ActionRotateView3D;
  action->stateid_.value() = view3d_state->stateid();
  action->axis_.value() = axis;
  action->angle_.value() = angle;
  action->view3d_state_ = StateView3DWeakHandle( view3d_state );

  Interface::PostAction( ActionHandle( action ) );
}

} // end namespace Seg3D
