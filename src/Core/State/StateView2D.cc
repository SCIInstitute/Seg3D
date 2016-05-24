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
#include <Core/State/StateView2D.h>

namespace Core
{

StateView2D::StateView2D( const std::string& stateid ) :
  StateViewBase( stateid )
{
}

StateView2D::~StateView2D()
{
}

std::string StateView2D::export_to_string() const
{
  return Core::ExportToString( value_ );
}

bool StateView2D::import_from_string( const std::string& str, ActionSource source )
{
  Core::View2D value;
  if ( !( Core::ImportFromString( str, value ) ) ) return false;

  return this->set( value, source );
}

void StateView2D::scale( double ratio )
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

void StateView2D::translate( const Core::Vector& offset )
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
  
void StateView2D::dolly( double dz )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  View2D val;
  {
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );
    this->value_.dolly( dz );
    val = this->value_;
  }
  
  if ( this->signals_enabled() ) 
  {
    this->value_changed_signal_( val, ActionSource::NONE_E );
    this->state_changed_signal_();
  }
}

bool StateView2D::set( const Core::View2D& value, ActionSource source )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

  if ( this->value_ != value )
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

void StateView2D::export_to_variant( Variant& variant ) const
{
  variant.set( this->value_ );
}

bool StateView2D::import_from_variant( Variant& variant, ActionSource source )
{
  Core::View2D value;
  if ( !( variant.get( value ) ) ) return false;

  return this->set( value, source );
}

bool StateView2D::validate_variant( Variant& variant, std::string& error )
{
  Core::View2D value;
  if ( !( variant.get( value ) ) )
  {
    error = "Cannot convert the value '" + variant.export_to_string()
        + "' to a 2D Camera position";
    return false;
  }

  error = "";
  return true;
}

} // end namespace Core
