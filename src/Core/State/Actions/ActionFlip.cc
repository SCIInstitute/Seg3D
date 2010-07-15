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

#include <Core/State/Actions/ActionFlip.h>

CORE_REGISTER_ACTION( Core, Flip )

namespace Core
{

ActionFlip::ActionFlip()
{
  add_argument( this->stateid_ );
  add_argument( this->direction_ );
}

ActionFlip::~ActionFlip()
{
}

bool ActionFlip::validate( ActionContextHandle &context )
{
  StateBaseHandle state( this->view2d_state_.lock() );
  if ( !state )
  {
    if ( !( StateEngine::Instance()->get_state( stateid_.value(), state ) ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + stateid_.value()
          + "'" );
      return false;
    }

    if ( typeid(*state) != typeid(StateView2D) )
    {
      context->report_error( std::string( "State variable '" ) + stateid_.value()
        + "' doesn't support ActionFlip" );
      return false;
    }

    this->view2d_state_ = StateView2DWeakHandle(
      boost::dynamic_pointer_cast< StateView2D >( state ) );
  }

  return true;
}

bool ActionFlip::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateView2DHandle state = this->view2d_state_.lock();

  if ( state )
  {
    state->flip( static_cast< Core::FlipDirectionType::enum_type > ( this->direction_.value() ) );
    return true;
  }

  return false;
}

ActionHandle ActionFlip::Create( StateView2DHandle& state, Core::FlipDirectionType direction )
{
  ActionFlip* action = new ActionFlip;
  action->stateid_.value() = state->stateid();
  action->direction_.value() = direction;
  action->view2d_state_ = state;
  return ActionHandle( action );  
}

void ActionFlip::Dispatch( ActionContextHandle context, StateView2DHandle& state, 
  Core::FlipDirectionType direction )
{
  ActionDispatcher::PostAction( Create( state, direction ), context );
}

} // end namespace Core
