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

// STD includes
#include <iostream>

// Boost includes
#include <boost/algorithm/string.hpp>

// Core includes
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/StringParser.h>

// Application includes
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

bool Action::changes_provenance_data() const
{
  return this->get_action_info()->changes_provenance_data();
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
  // Add action name to string
  std::string command = std::string( this->get_type() ) + " ";

  size_t num_arguments = this->get_action_info()->get_num_arguments();

  // Loop through all the arguments and add them
  for ( size_t j = 0; j < num_arguments && j < this->parameters_.size(); j++ )
  {
    command += this->parameters_[ j ]->export_to_string() + " ";
  }
  
  for ( size_t j = num_arguments; j < this->parameters_.size(); j++ )
  {
    command += this->get_key( j - num_arguments ) + "=" + this->parameters_[ j ]->export_to_string() + " ";
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
  std::string::size_type pos = 0;
  std::string command;
  std::string value;

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

  size_t num_arguments = this->get_action_info()->get_num_arguments();

  for ( size_t j = 0; j < num_arguments; j++ )
  {
    if ( !( Core::ScanValue( action, pos, value, error ) ) )
    {
      error = std::string( "SYNTAX ERROR: " ) + error;
    }

    if ( value.empty() )
    {
      error = std::string( "ERROR: not enough arguments, argument " ) + 
        ExportToString( j + 1 ) + " is missing.";
      return false;
    }

    if ( !( this->parameters_[ j ]->import_from_string( value ) ) )
    {
      error = std::string( "SYNTAX ERROR: Could not interpret '" + value + "'" );
      return false;
    }
  }

  // Get all the key value pairs and stream them into the action
  std::string key;

  while ( true )
  {
    if ( !( Core::ScanKeyValuePair( action, pos, key, value, error ) ) )
    {
      error = std::string( "SYNTAX ERROR: " ) + error;
      return false;
    }

    if ( key.empty() ) break;

    int index = this->get_key_index( Core::StringToLower( key ) );
    if ( index == -1 )
    {
      error = std::string( "SYNTAX ERROR: Could not interpret '" + value + "'" );
      return false;   
    }

    if ( this->parameters_[ index + num_arguments ] == 0 )
    {
      CORE_THROW_LOGICERROR( "Encountered incorrectly constructed action" );
    }
    
    this->parameters_[ index + num_arguments ]->import_from_string( value );
  }

  return true;
}

} // end namespace Core
