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

// STL includes
#include <algorithm>

// Application includes
#include <Core/State/StateAlias.h>

namespace Core
{

StateAlias::StateAlias( const std::string& stateid ) :
  StateBase( stateid )
{
}

StateAlias::StateAlias( const std::string& stateid, const std::string default_value ) :
  StateBase( stateid ),
  value_( StateEngine::CreateStateAlias( default_value ) )
{
  if ( !( value_.empty() ) )
  {
    StateEngine::Instance()->add_statealias( value_, baseid() );
  }
}

StateAlias::~StateAlias()
{
  if ( !( value_.empty() ) )
  {
    StateEngine::Instance()->remove_statealias( value_ );
  }
}

std::string StateAlias::export_to_string() const
{
  return Core::ExportToString( value_ );
}

bool StateAlias::import_from_string( const std::string& str, ActionSource source )
{
  std::string value;
  if ( !( Core::ImportFromString( str, value ) ) ) return ( false );

  return set( value, source );
}

bool StateAlias::set( const std::string& value, ActionSource source )
{
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

  std::string new_value = StateEngine::CreateStateAlias( value );
  if ( new_value != value ) source = ActionSource::NONE_E;

  if ( StateEngine::Instance()->is_statealias( new_value ) ) return false;

  if ( new_value != value_ )
  {
    if ( !( value_.empty() ) )
    {
      StateEngine::Instance()->remove_statealias( value_ );
    }

    value_ = new_value;
    if ( !( value_.empty() ) )
    {
      StateEngine::Instance()->add_statealias( value_, baseid() );
    }

    value_changed_signal_( value_, source );
    state_changed_signal_();
  }
  
  return true;
}

void StateAlias::export_to_variant( ActionParameterVariant& variant ) const
{
  variant.set_value( value_ );
}

bool StateAlias::import_from_variant( ActionParameterVariant& variant, ActionSource source )
{
  std::string value;
  if ( !( variant.get_value( value ) ) ) return false;

  return set( value, source );
}

bool StateAlias::validate_variant( ActionParameterVariant& variant, std::string& error )
{
  std::string value;
  if ( !( variant.get_value( value ) ) )
  {
    error = "Cannot convert the value '" + variant.export_to_string() + "' to an alias";
    return ( false );
  }

  error = "";
  return true;
}

} // end namespace Core
