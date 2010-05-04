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

#include <Core/Action/ActionFactory.h>
#include <Core/State/StateEngine.h>
#include <Core/State/Actions/ActionSetRange.h>

CORE_REGISTER_ACTION( Core, SetRange )

namespace Core
{

ActionSetRange::ActionSetRange()
{
  add_argument( this->stateid_ );
  add_argument( this->min_value_ );
  add_argument( this->max_value_ );
}

bool ActionSetRange::validate(  ActionContextHandle& context )
{
  StateBaseHandle state = this->state_weak_handle_.lock();
  if ( !state )
  {
    if ( !( StateEngine::Instance()->get_state( stateid_.value(), state ) ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + stateid_.value()
          + "'" );
      return false;
    }
  }

  if ( typeid( *state ) != typeid( StateRangedInt ) &&
     typeid( *state ) != typeid( StateRangedDouble ) )
  {
    context->report_error( std::string( "State variable '" ) + stateid_.value()
        + "' doesn't support ActionSetRange" );
    return false;
  }

  this->state_weak_handle_ = state;
  return true;
}

bool ActionSetRange::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateBaseHandle state = this->state_weak_handle_.lock();
  if ( state )
  {
    if ( typeid( *state ) == typeid( StateRangedInt ) )
    {
      StateRangedInt* ranged_int_state = 
        dynamic_cast< StateRangedInt* >( state.get() );
      ranged_int_state->set_range( static_cast<int>( this->min_value_.value() ),
        static_cast<int>( this->max_value_.value() ), context->source() );
      return true;
    }

    if ( typeid( *state ) == typeid( StateRangedDouble ) )
    {
      StateRangedDouble* ranged_double_state = 
        dynamic_cast< StateRangedDouble* >( state.get() );
      ranged_double_state->set_range( this->min_value_.value(), this->max_value_.value(),
        context->source() );
      return true;
    }
  }

  return false;
}

} // end namespace Core