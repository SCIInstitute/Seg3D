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

#include <Core/State/StateEngine.h>
#include <Core/State/StateOption.h>
#include <Core/Utils/Exception.h>

namespace Core
{

StateOption::StateOption( const std::string& stateid, const std::string& default_value, 
  const std::string& option_list ) :
  StateBase( stateid ),
  value_( Core::StringToLower( default_value ) )
{
  // Unwrap the option lists
  this->option_list_ = Core::SplitString( Core::StringToLower( option_list ), "|" );

  option_list_iterator_type it = 
    std::find( this->option_list_.begin(), this->option_list_.end(), this->value_ );
  if ( it == this->option_list_.end() )
  {
    CORE_THROW_LOGICERROR( std::string( "Option \"" ) + this->value_ +
      "\" not in the option list" );
  }
  this->index_ = static_cast<int>( it - this->option_list_.begin() );
}

StateOption::StateOption( const std::string& stateid, const std::string& default_value,
    const std::vector< std::string >& option_list ) :
  StateBase( stateid ),
  value_( Core::StringToLower( default_value ) )
{
  this->option_list_.resize( option_list.size() );
  for ( size_t j = 0; j < option_list.size(); j++ )
  {
    this->option_list_[ j ] = Core::StringToLower( option_list[ j ] );
  }

  option_list_iterator_type it = 
    std::find( this->option_list_.begin(), this->option_list_.end(), this->value_ );
  if ( it == this->option_list_.end() )
  {
    CORE_THROW_LOGICERROR( std::string( "Option \"" ) + this->value_ +
      "\" not in the option list" );
  }
  this->index_ = static_cast<int>( it - this->option_list_.begin() );
}

StateOption::~StateOption()
{
}

std::string StateOption::export_to_string() const
{
  return this->value_;
}

std::string StateOption::export_list_to_string() const
{
  std::string list_as_string = "";
  for( size_t i = 0; i < ( this->option_list_.size() - 1 ); ++i )
    list_as_string = list_as_string + option_list_[ i ] + "|";

  return list_as_string + this->option_list_[ ( this->option_list_.size() - 1 ) ];
}

bool StateOption::set( const std::string& input_value, ActionSource source )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
  
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

  std::string value = Core::StringToLower( input_value );
  if ( value != this->value_ )
  {
    option_list_iterator_type it = 
      std::find( this->option_list_.begin(), this->option_list_.end(), value );

    if ( this->option_list_.end() == it )
    {
      if ( source == ActionSource::INTERFACE_WIDGET_E )
      {
        lock.unlock();
        // NOTE: This is a special case in which the option requested by the
        // interface does not exist and hence the value may be out of sync and
        // hence needs to be set to the correct value. Hence we generate the
        // signal that indicates that the state has changed anyway.

        // Any other sources are fine as they do not reflect a different value
        // and are validated before the code can reach this point.
        if ( this->signals_enabled() )
        {       
          this->value_changed_signal_( value_, source );
          this->state_changed_signal_();
        }
      }
      return false;
    }
    this->value_ = value;
    this->index_ = static_cast<int>( it - this->option_list_.begin() );

    lock.unlock();
    
    if ( this->signals_enabled() )
    {
      this->value_changed_signal_( value_, source );
      this->state_changed_signal_();
    }
  }
  return true;
}

bool StateOption::import_from_string( const std::string& str, ActionSource source )
{
  std::string value;
  if ( !( Core::ImportFromString( str, value ) ) ) return false;

  return this->set( value, source );
}

void StateOption::export_to_variant( Variant& variant ) const
{
  variant.set( this->value_ );
}

bool StateOption::import_from_variant( Variant& variant, ActionSource source )
{
  std::string value;
  if ( !( variant.get( value ) ) ) return false;

  return this->set( value, source );
}

bool StateOption::validate_variant( Variant& variant, std::string& error )
{
  std::string value;
  if ( !( variant.get( value ) ) )
  {
    error = "Cannot convert the value '" + variant.export_to_string() + "'";
    return false;
  }

  value = Core::StringToLower( value );
  if ( this->option_list_.end() == std::find( this->option_list_.begin(),
    this->option_list_.end(), value ) )
  {
    error = "Option '" + value + "' is not a valid option";
    return false;
  }

  error = "";
  return true;
}

bool StateOption::is_option( const std::string& option )
{
  if ( this->option_list_.end() == std::find( this->option_list_.begin(), 
    this->option_list_.end(), Core::StringToLower( option ) ) ) return false;
  return true;
}

void StateOption::set_option_list( const std::vector< std::string >& option_list )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

  this->option_list_ = option_list;
  for ( size_t j = 0; j < this->option_list_.size(); j++ )
  {
    this->option_list_[ j ] = Core::StringToLower( this->option_list_[ j ] );
  }
  
  option_list_iterator_type it = 
    std::find( this->option_list_.begin(), this->option_list_.end(), this->value_ );

  if ( this->option_list_.end() == it )
  {
    if ( option_list.size() )
    {
      this->index_ = 0;
      this->value_ = option_list[ 0 ];
    }
    else
    {
      this->value_ = "";
      this->index_ = -1;
    }

    lock.unlock();
    
    if ( this->signals_enabled() )
    {
      this->value_changed_signal_( this->value_, ActionSource::NONE_E );
      this->state_changed_signal_();
    }
  }
  else
  {
    this->index_ = static_cast<int>( it - this->option_list_.begin() );
    lock.unlock();
  }

  if ( this->signals_enabled() )
  {
    this->optionlist_changed_signal_();
  }
}

void StateOption::set_option_list( const std::string& option_list )
{
  this->set_option_list( Core::SplitString( option_list, "|" ) );
}

void StateOption::set_option_list( const std::string& option_list, const std::string& option )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
  
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

  // Unwrap the option lists
  this->option_list_ = Core::SplitString( Core::StringToLower( option_list ), "|" );

  std::string lower_option = Core::StringToLower( option );
  bool value_changed = false;

  option_list_iterator_type it = 
    std::find( this->option_list_.begin(), this->option_list_.end(), lower_option );
  if ( this->option_list_.end() == it )
  {
    if ( this->option_list_.size() ) 
    {
      this->index_ = 0;
      this->value_ = this->option_list_[ 0 ];
    }
    else 
    {
      this->value_ = "";
      this->index_ = -1;
    }

    value_changed = true;
  }
  else
  {
    this->index_ = static_cast<int>( it - this->option_list_.begin() );
    if ( this->value_ != lower_option )
    {
      this->value_ = lower_option;
      value_changed = true;
    }
  }

  lock.unlock();
  
  if ( value_changed )
  {
    if ( this->signals_enabled() )
    {
      this->value_changed_signal_( value_, ActionSource::NONE_E );
      this->state_changed_signal_();
    }
  }
  
  if ( this->signals_enabled() )
  { 
    this->optionlist_changed_signal_();
  }
}

} // end namespace Core
