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

#include <Core/State/Actions/ActionOffset.h>
#include <Core/State/StateEngine.h>
#include <Core/Action/ActionFactory.h>

CORE_REGISTER_ACTION( Core, Offset )

namespace Core
{

ActionOffset::ActionOffset()
{
  this->add_argument( this->stateid_ );
  this->add_argument( this->offset_value_ );
}

ActionOffset::~ActionOffset()
{
}

bool ActionOffset::validate( ActionContextHandle& context )
{
  StateBaseHandle state( this->state_weak_handle_.lock() );

  if ( !state )
  {
    if ( !StateEngine::Instance()->get_state( this->stateid_.value(), state ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + stateid_.value() + "'" );
      return false;
    }
    StateRangedValueBaseHandle value_state = 
      boost::dynamic_pointer_cast< StateRangedValueBase >( state );
    if ( !value_state )
    {
      context->report_error( std::string( "State variable '") + this->stateid_.value() +
        "' doesn't support ActionOffset" );
      return false;
    }
    this->state_weak_handle_ = value_state; 
  }

  std::string error;
  if ( !state->validate_variant( this->offset_value_, error ) )
  {
    context->report_error( error );
    return false;
  }
  
  return true;
}

bool ActionOffset::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateRangedValueBaseHandle value_state( this->state_weak_handle_.lock() );
  if ( value_state )
  {
    return value_state->import_offset_from_variant( this->offset_value_, context->source() );
  }
  
  return false;
}




} // end namespace Core