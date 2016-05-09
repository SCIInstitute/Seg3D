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

#include <Core/Action/ActionFactory.h>
#include <Core/State/StateEngine.h>
#include <Core/State/Actions/ActionSetRange.h>

CORE_REGISTER_ACTION( Core, SetRange )

namespace Core
{

bool ActionSetRange::validate(  ActionContextHandle& context )
{
  StateBaseHandle state = this->state_weak_handle_.lock();
  if ( !state )
  {
    if ( !( StateEngine::Instance()->get_state( stateid_, state ) ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + stateid_ + "'" );
      return false;
    }

    StateRangedValueBaseHandle ranged_value =
      boost::dynamic_pointer_cast< StateRangedValueBase >( state );
    if ( !ranged_value )
    {
      context->report_error( std::string( "State variable '" ) + stateid_ + "' doesn't support ActionSetRange" );
      return false;
    }

    std::string error;
    if ( !ranged_value->validate_value_type_variant( this->min_value_, error ) ||
      !ranged_value->validate_value_type_variant( this->max_value_, error ) )
    {
      context->report_error( error );
      return false;
    }

    if ( state->get_locked() )
    {
      context->report_error( std::string( "State variable '" ) + stateid_ + "' has been locked." );
      return false; 
    }

    this->state_weak_handle_ = ranged_value;
  }

  return true;
}

bool ActionSetRange::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateRangedValueBaseHandle state = this->state_weak_handle_.lock();
  if ( state )
  {
     return state->import_range_from_variant( this->min_value_, this->max_value_, 
       context->source() );
  }

  return false;
}

} // end namespace Core
