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

// STL includes
#include <algorithm>

// Application includes
#include <Core/State/StateView3D.h>

namespace Core
{

StateView3D::StateView3D( const std::string& stateid ) :
  StateViewBase( stateid )
{
}

StateView3D::~StateView3D()
{
}

std::string StateView3D::export_to_string() const
{
  return ( Core::ExportToString( this->value_ ) );
}

bool StateView3D::import_from_string( const std::string& str, ActionSource source )
{
  Core::View3D value;
  if ( !( Core::ImportFromString( str, value ) ) ) return false;

  return this->set( value, source );
}

bool StateView3D::set( const Core::View3D& value, ActionSource source )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

  if ( value != this->value_ )
  {
    this->value_ = value;
    lock.unlock();
    
    if ( this->signals_enabled() )
    {
      this->value_changed_signal_( value, source );
      this->state_changed_signal_();
    }
  }
  return true;
}

void StateView3D::rotate( const Core::Vector& axis, double angle )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  {
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );
    this->value_.rotate( axis, angle );
  }
  
  if ( this->signals_enabled() )
  {
    this->state_changed_signal_();
  }
}

void StateView3D::scale( double ratio )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  {
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );
    this->value_.scale( ratio );
  }
  
  if ( this->signals_enabled() )
  {
    this->state_changed_signal_();
  }
}

void StateView3D::translate( const Core::Vector& offset )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  {
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );
    this->value_.translate( offset );
  }
  
  if ( this->signals_enabled() )
  {
    this->state_changed_signal_();
  }
}

void StateView3D::export_to_variant( Variant& variant ) const
{
  variant.set( this->value_ );
}

bool StateView3D::import_from_variant( Variant& variant, ActionSource source )
{
  Core::View3D value;
  if ( !( variant.get( value ) ) ) return false;

  return this->set( value, source );
}

bool StateView3D::validate_variant( Variant& variant, std::string& error )
{
  Core::View3D value;
  if ( !( variant.get( value ) ) )
  {
    error = "Cannot convert the value '" + variant.export_to_string()
        + "' to a 3D Camera position";
    return false;
  }

  error = "";
  return true;
}

} // end namespace Core
