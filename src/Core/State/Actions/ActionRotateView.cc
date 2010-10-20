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

#include <Core/State/Actions/ActionRotateView.h>

CORE_REGISTER_ACTION( Core, RotateView )

namespace Core
{

ActionRotateView::ActionRotateView()
{
  add_argument( this->stateid_ );
  add_argument( this->axis_ );
  add_argument( this->angle_ );
}

ActionRotateView::~ActionRotateView()
{
}

bool ActionRotateView::validate( ActionContextHandle& context )
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
          + "' doesn't support ActionRotateView" );
      return false;
    }

    this->view3d_state_ = StateView3DWeakHandle(
        boost::dynamic_pointer_cast< StateView3D >( state ) );
  }

  return true;
}

bool ActionRotateView::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateView3DHandle state = this->view3d_state_.lock();

  if ( state )
  {
    state->rotate( this->axis_.value(), this->angle_.value() );
    return true;
  }

  return false;
}

ActionHandle ActionRotateView::Create( StateView3DHandle& view3d_state, const Core::Vector& axis,
    double angle )
{
  ActionRotateView* action = new ActionRotateView;
  action->stateid_.value() = view3d_state->get_stateid();
  action->axis_.value() = axis;
  action->angle_.value() = angle;
  action->view3d_state_ = StateView3DWeakHandle( view3d_state );
  return ActionHandle( action );
}

void ActionRotateView::Dispatch( ActionContextHandle context, StateView3DHandle& view3d_state, 
  const Core::Vector& axis, double angle )
{
  ActionDispatcher::PostAction( Create( view3d_state, axis, angle), context );
}

bool ActionRotateView::changes_project_data()
{
  StateBaseHandle state( view3d_state_.lock() );

  // If not the state cannot be retrieved report an error
  if ( !state )
  {
    if ( !( StateEngine::Instance()->get_state( stateid_.value(), state ) ) )
    {
      return false;
    }
  }

  // Keep track of whether the state changes the data of the program
  return state->is_project_data();
}

} // end namespace Core
