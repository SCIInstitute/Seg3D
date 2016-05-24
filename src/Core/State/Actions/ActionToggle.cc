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
#include <Core/State/Actions/ActionToggle.h>

CORE_REGISTER_ACTION( Core, Toggle )

namespace Core
{

bool ActionToggle::validate(  ActionContextHandle& context )
{
  StateBaseHandle state = this->state_weak_handle_.lock();
  if ( !state )
  {
    if ( !( StateEngine::Instance()->get_state( stateid_, state ) ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + stateid_ + "'" );
      return false;
    }

    StateBoolHandle typed_state =
      boost::dynamic_pointer_cast< StateBool >( state );
    if ( !typed_state )
    {
      context->report_error( std::string( "State variable '" ) + stateid_ + "' doesn't support ActionToggle" );
      return false;
    }
    
    this->state_weak_handle_ = typed_state;
  }

  if ( state->get_locked() )
  {
    context->report_error( std::string( "State variable '" ) + stateid_ + "' has been locked." );
    return false; 
  }

  return true;
}

bool ActionToggle::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateBoolHandle state = this->state_weak_handle_.lock();
  if ( state )
  {
    state->set( !state->get() );
    return true;
  }

  return false;
}

bool ActionToggle::changes_project_data()
{
  StateBaseHandle state( state_weak_handle_.lock() );

  // If not the state cannot be retrieved report an error
  if ( !state )
  {
    if ( !( StateEngine::Instance()->get_state( stateid_, state ) ) )
    {
      return false;
    }
  }

  // Keep track of whether the state changes the data of the program
  return state->is_project_data();
}

} // end namespace Core
