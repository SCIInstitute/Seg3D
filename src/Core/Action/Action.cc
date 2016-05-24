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

// Core includes
#include <Core/Utils/Exception.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/StringParser.h>
#include <Core/Action/ActionParameter.h>
#include <Core/Action/Action.h>

namespace Core
{

Action::Action() 
{
}

Action::~Action()
{
  // Clean out the parameter accessors
  for ( size_t j = 0; j < this->parameters_.size(); j++ )
  {
    delete this->parameters_[ j ];
  }
}

std::string Action::get_definition() const
{
  return this->get_action_info()->get_definition();
}

std::string Action::get_type() const
{
  return this->get_action_info()->get_type();
}

std::string Action::get_usage() const
{
  return this->get_action_info()->get_usage();
}

std::string Action::get_key( size_t index ) const
{
  return this->get_action_info()->get_key( index );
}

int Action::get_key_index( const std::string& key ) const
{
  return this->get_action_info()->get_key_index( key );
}

bool Action::changes_project_data()
{
  return this->get_action_info()->changes_project_data();
}

bool Action::is_undoable() const
{
  return this->get_action_info()->is_undoable();
}

std::string Action::get_default_key_value( size_t index ) const
{
  return this->get_action_info()->get_default_key_value( index );
}

bool Action::translate( ActionContextHandle& context )
{
  return true;
}

void Action::clear_cache()
{
}

std::string Action::export_to_string() const
{
#ifndef NDEBUG
  if ( this->num_params() != this->get_action_info()->get_num_key_value_pairs() )
  {
    CORE_THROW_LOGICERROR( "Number of parameters doesn't match action definition." );
  }
#endif

  // Add action name to string
  std::string command = std::string( this->get_type() ) + " ";
  
  for ( size_t j = 0; j < this->parameters_.size(); j++ )
  {
    if ( this->parameters_[ j ]->is_persistent() )
    {
      command += this->get_key( j ) + "=" + this->parameters_[ j ]->export_to_string() + " ";
    } 
  }

  // Return the command
  return command;
}

bool Action::import_from_string( const std::string& action )
{
  std::string error;
  return this->import_from_string( action, error );
}

bool Action::import_from_string( const std::string& action, std::string& error )
{
#ifndef NDEBUG
  if ( this->num_params() != this->get_action_info()->get_num_key_value_pairs() )
  {
    CORE_THROW_LOGICERROR( "Number of parameters doesn't match action definition." );
  }
#endif
  
  std::string::size_type pos = 0;
  std::string command;

  // First part of the string is the command
  if ( !( Core::ScanCommand( action, pos, command, error ) ) )
  {
    error = std::string( "SYNTAX ERROR: " ) + error;
    return false;
  }

  if ( command.empty() )
  {
    error = std::string( "ERROR: missing command." );
    return false;
  }

  // Get all the key value pairs and stream them into the action
  std::string key;
  std::string value;
  std::map< std::string, std::string > parameter_map;

  while ( true )
  {
    if ( !( Core::ScanKeyValuePair( action, pos, key, value, error ) ) )
    {
      error = std::string( "SYNTAX ERROR: " ) + error;
      return false;
    }

    if ( key.empty() ) break;

    parameter_map[ key ] = value;
  }

  for ( size_t i = 0; i < this->num_params(); ++i )
  {
    std::string param_name = this->get_key( i );
    std::string param_value;
    std::map< std::string, std::string >::iterator it = parameter_map.find( param_name );
    if ( it != parameter_map.end() )
    {
      param_value = ( *it ).second;
    }
    else
    {
      param_value = this->get_default_key_value( i );
      if ( param_value.empty() )
      {
        error = "Parameter '" + param_name + "' is required but not provided.";
        return false;
      }
    }

    if ( this->parameters_[ i ] == 0 )
    {
      CORE_THROW_LOGICERROR( "Encountered incorrectly constructed action" );
    }
    
    if ( !this->parameters_[ i ]->import_from_string( param_value ) )
    {
      error = "'" + param_value + "' is not a proper value for parameter '" + param_name + "'.";
      return false;
    }
  }
  
  return true;
}

} // end namespace Core
