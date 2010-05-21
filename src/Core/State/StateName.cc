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
    lock_type lock( GetMutex() );
    if ( NAME_SET_S.erase( this->value_ ) == 0 )
    {
      CORE_LOG_ERROR( std::string( "Trying to delete name \"" ) + this->value_ +
        "\" that does not exist" );
    }
  }

public:
  // Actual storage for the state
  std::string value_;

  // -- Static members --
public:

  typedef std::set< std::string > name_set_type;
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  static std::string GenerateName( const std::string& str )
  {
    lock_type lock( GetMutex() );

    std::string input_str = boost::regex_replace( str, REGEX_C, "_" );

    if ( NAME_SET_S.count( input_str ) == 0 )
    {
      NAME_SET_S.insert( input_str );
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
    } while ( NAME_SET_S.count( result ) != 0 );

    NAME_SET_S.insert( result );
    return result;
  }

  static void RemoveName( const std::string& value )
  {
    lock_type lock( GetMutex() );
    NAME_SET_S.erase( value );
  }

  static bool NameExists( const std::string& value )
  {
    lock_type lock( GetMutex() );
    return NAME_SET_S.count( value ) != 0;
  }

  static mutex_type& GetMutex()
  {
    return NAME_SET_MUTEX_S;
  }

public:
  // Static data structure for keeping track of names in use
  static name_set_type NAME_SET_S;
  static mutex_type NAME_SET_MUTEX_S;
  static const boost::regex REGEX_C;
};

const std::string StateName::REGEX_VALIDATOR_C( "[\\w\\s]*" );
StateNamePrivate::name_set_type StateNamePrivate::NAME_SET_S;
StateNamePrivate::mutex_type StateNamePrivate::NAME_SET_MUTEX_S;
const boost::regex StateNamePrivate::REGEX_C( "[^\\w\\s]" );

StateName::StateName( const std::string& stateid, const std::string& value ) :
  StateBase( stateid )
{
  this->private_ = StateNamePrivateHandle( new StateNamePrivate( value ) );
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

void StateName::export_to_variant( ActionParameterVariant& variant ) const
{
  variant.set_value( this->private_->value_ );
}

bool StateName::import_from_variant( ActionParameterVariant& variant, ActionSource source )
{
  std::string value;
  if ( !variant.get_value( value ) )
  {
    return false;
  }
  
  return this->set( value, source );
}

bool StateName::validate_variant( ActionParameterVariant& variant, std::string& error )
{
  std::string value;
  if ( !variant.get_value( value ) )
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
  std::string old_value;

  {
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

    if ( this->private_->value_ == value )
    {
      return true;
    }

    if ( value.size() == 0 )
    {
      this->value_changed_signal_( this->private_->value_, this->private_->value_, source );
      this->state_changed_signal_();
      return false;
    }

    old_value = this->private_->value_;
    StateNamePrivate::RemoveName( old_value );
    this->private_->value_ = StateNamePrivate::GenerateName( value );
  }

  this->value_changed_signal_( old_value, this->private_->value_, source );
  this->state_changed_signal_();
  return true;
}

} // end namespace Core