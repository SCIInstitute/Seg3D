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

#if defined(_MSC_VER)
#pragma warning(disable : 4267 4244)
#endif

#include <cctype>
#include <set>

#include <boost/regex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <Core/State/StateEngine.h>
#include <Core/State/StateName.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>

namespace Core
{

class StateNamePrivate
{
public:
  StateNamePrivate( const std::string value )
  {
    this->value_ = GenerateName( value );
  }

  ~StateNamePrivate()
  {
    if ( !this->valid_ )
    {
      return;
    }
    
    lock_type lock( GetMutex() );
    if ( NameSet.erase( this->value_ ) == 0 )
    {
      CORE_LOG_ERROR( std::string( "Trying to delete name \"" ) + this->value_ +
        "\" that does not exist" );
    }
  }

public:
  // Actual storage for the state
  std::string value_;
  bool valid_;

  // -- Static members --
public:

  typedef std::set< std::string > name_set_type;
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  static std::string GenerateName( const std::string& str )
  {
    lock_type lock( GetMutex() );

    std::string input_str = boost::regex_replace( str, REGEX_C, "_" );

    if ( NameSet.count( input_str ) == 0 )
    {
      NameSet.insert( input_str );
      return input_str;
    }

    std::string str_prefix = input_str + "_";
    size_t loc = input_str.find_last_of( "_" );
    if ( loc != std::string::npos )
    {
      size_t j = loc;
      bool has_num_surfix = true;
      while( ++j < input_str.size() )
      {
        if ( !isdigit( input_str[ j ] ) )
        {
          has_num_surfix = false;
          break;
        }
      }
      if ( has_num_surfix )
      {
        str_prefix = input_str.substr( 0, loc + 1 );
      }
    }
    
    std::string result;
    int i = 1;
    do 
    {
      result = str_prefix + ExportToString( i++ );
    } while ( NameSet.count( result ) != 0 );

    NameSet.insert( result );
    return result;
  }

  static void RemoveName( const std::string& value )
  {
    lock_type lock( GetMutex() );
    NameSet.erase( value );
  }

  static bool NameExists( const std::string& value )
  {
    lock_type lock( GetMutex() );
    return NameSet.count( value ) != 0;
  }

  static mutex_type& GetMutex()
  {
    return NameSetMutex;
  }

public:
  // Static data structure for keeping track of names in use
  static name_set_type NameSet;
  static mutex_type NameSetMutex;
  static const boost::regex REGEX_C;
};

const std::string StateName::REGEX_VALIDATOR_C( "[\\w\\s\\.\\-]*" );
StateNamePrivate::name_set_type StateNamePrivate::NameSet;
StateNamePrivate::mutex_type StateNamePrivate::NameSetMutex;
const boost::regex StateNamePrivate::REGEX_C( "[^\\w\\s\\.\\-]" );

StateName::StateName( const std::string& stateid, const std::string& value ) :
  StateBase( stateid )
{
  this->private_ = StateNamePrivateHandle( new StateNamePrivate( value ) );
  this->private_->valid_ = true;
}

StateName::~StateName()
{
}

std::string StateName::export_to_string() const
{
  return this->private_->value_;
}

bool StateName::import_from_string( const std::string& str, ActionSource source )
{
  std::string value;
  if ( !Core::ImportFromString( str, value ) )
  {
    return false;
  }
  
  return this->set( value, source );
}

void StateName::export_to_variant( Variant& variant ) const
{
  variant.set( this->private_->value_ );
}

bool StateName::import_from_variant( Variant& variant, ActionSource source )
{
  std::string value;
  if ( !variant.get( value ) )
  {
    return false;
  }
  
  return this->set( value, source );
}

bool StateName::validate_variant( Variant& variant, std::string& error )
{
  std::string value;
  if ( !variant.get( value ) )
  {
    error = "Cannot convert the value '" + variant.export_to_string() + "'";
    return false;
  }
  
  error = "";
  return true;
}

const std::string& StateName::get() const
{
  return this->private_->value_;
}

bool StateName::set( const std::string& value, ActionSource source )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

  std::string old_value;
  std::string new_value;

  {
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

    if ( this->private_->value_ == value )
    {
      return true;
    }

    old_value = this->private_->value_;

    if ( value.size() == 0 )
    {
      lock.unlock();
      
      if ( this->signals_enabled() )
      {   
        this->value_changed_signal_( old_value, old_value, source );
        this->state_changed_signal_();
      }
      return false;
    }

    StateNamePrivate::RemoveName( old_value );
    this->private_->value_ = StateNamePrivate::GenerateName( value );
    new_value = this->private_->value_;
  }

  if ( this->signals_enabled() )
  {
    this->value_changed_signal_( old_value, new_value, source );
    this->state_changed_signal_();
  }
  
  return true;
}

void StateName::invalidate()
{
  if( !this->private_->valid_ )
  {
    return;
  }

  this->private_->valid_ = false;
  
  StateNamePrivate::RemoveName( this->private_->value_ );
  this->private_->value_ = "";
}

} // end namespace Core
