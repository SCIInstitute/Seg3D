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
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/StateEngine.h>

CORE_REGISTER_ACTION( Core, Add )

namespace Core
{

bool ActionAdd::validate( ActionContextHandle& context )
{
  StateBaseHandle state( this->state_weak_handle_.lock() );
  if ( !state )
  {
    if ( !StateEngine::Instance()->get_state( this->stateid_, state ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + stateid_ + "'" );
      return false;
    }
    this->state_weak_handle_ = state;
  }

  if ( state->get_locked() )
  {
    context->report_error( std::string( "State variable '" ) + stateid_ + "' has been locked." );
    return false; 
  }

  StateVectorBase* vector_state = dynamic_cast< StateVectorBase* >( state.get() );
  if ( vector_state != 0 )
  {
    std::string error;
    if ( !vector_state->validate_element_variant( this->value_, error ) )
    {
      context->report_error( error );
      return false;
    }
    return true;
  }
  
  StateSetBase* set_state = dynamic_cast< StateSetBase* >( state.get() );
  if ( set_state != 0 )
  {
    std::string error;
    if ( !set_state->validate_element_variant( this->value_, error ) )
    {
      context->report_error( error );
      return false;
    }
    return true;
  }
  
  context->report_error( std::string( "State variable '") + this->stateid_ +
    "' doesn't support ActionAdd" );
  return false;
}

bool ActionAdd::run( ActionContextHandle& context, ActionResultHandle& result )
{
  StateBaseHandle state( this->state_weak_handle_.lock() );

  StateVectorBase* vector_state = dynamic_cast< StateVectorBase* >( state.get() );
  if ( vector_state != 0 )
  {
    return vector_state->add( this->value_, context->source() );
  }
  
  StateSetBase* set_state = dynamic_cast< StateSetBase* >( state.get() );
  if ( set_state != 0 )
  {
    return set_state->add( this->value_, context->source() );
  }

  return false;
}

bool ActionAdd::changes_project_data()
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
