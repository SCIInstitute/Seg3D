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

// boost includes
#include <boost/unordered/unordered_map.hpp>

// Core includes
#include <Core/Utils/StringParser.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Exception.h>
#include <Core/Action/ActionFactory.h>

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( ActionFactory );

class ActionFactoryEntry
{
public:
  ActionBuilderHandle  builder_;
  ActionInfoHandle  info_;
};


// Definition of the internals of the class
class ActionFactoryPrivate
{
public:
  // List with builders that can be called to generate a new object
  typedef boost::unordered_map<std::string,ActionFactoryEntry> action_map_type;
  action_map_type actions_;
};
  

ActionFactory::ActionFactory() :
  private_( new ActionFactoryPrivate )
{
}

ActionFactory::~ActionFactory()
{
}

void ActionFactory::register_action( ActionBuilderHandle builder, ActionInfoHandle info )
{
  // Only register valid actions
  if ( ! info->is_valid() ) return;
  
  // Make a new entry for this action
  ActionFactoryEntry entry;
  entry.builder_ = builder;
  entry.info_ = info;

  // Lock the factory, so the internal list can be modified
  lock_type lock( get_mutex() );

  // Ensure the name will be treated case insensitive
  std::string name = StringToLower( info->get_type() );

  // Test is action was registered before.
  if ( private_->actions_.find( name ) != private_->actions_.end() )
  {
    // Actions that are registered twice, will cause problems
    // Hence the program will throw an exception.
    // As registration is done on startup, this will cause a
    // faulty program to fail always on startup.
    CORE_THROW_LOGICERROR( std::string( "Action '" ) + name + "' was registered twice" );
  }

  // Register the action
  private_->actions_[ name ] = entry;
}

bool ActionFactory::create_action( const std::string& action_string, ActionHandle& action,
    std::string& error, std::string& usage )
{
  // Ensure that the default output has been cleared
  action.reset();
  error = "";
  usage = "";

  // Lock the factory
  lock_type lock( get_mutex() );
  
  std::string command;
  std::string::size_type pos = 0;

  // Scan for the command that needs to be instanted.
  if ( !( Core::ScanCommand( action_string, pos, command, error ) ) )
  {
    error = std::string( "SYNTAX ERROR: " ) + error;
    return false;
  }

  command = StringToLower( command );
  
  ActionFactoryPrivate::action_map_type::const_iterator it = 
    this->private_->actions_.find( command );

  // If we cannot find the maker report error.
  if ( it == private_->actions_.end() )
  {
    error = std::string( "SYNTAX ERROR: Unknown command '" + command + "'" );
    return false;
  }

  // Build the action of the right type
  action = (*it).second.builder_->build();
  usage = (*it).second.info_->get_usage();

  return action->import_from_string( action_string, error );
}

bool ActionFactory::action_list( std::vector<std::string>& action_list )
{
  // Lock the factory
  lock_type lock( get_mutex() );

  ActionFactoryPrivate::action_map_type::iterator it = private_->actions_.begin();
  ActionFactoryPrivate::action_map_type::iterator it_end = private_->actions_.end();

  while ( it != it_end )
  {
    action_list.push_back( ( *it ).first );
    ++it;
  }

  std::sort( action_list.begin(), action_list.end() );

  // indicate success
  return true;
}

bool ActionFactory::CreateAction( const std::string& actionstring, ActionHandle& action,
    std::string& error )
{
  std::string dummy;
  return ActionFactory::Instance()->create_action( actionstring, action, error, dummy );
}

bool ActionFactory::CreateAction( const std::string& actionstring, ActionHandle& action,
    std::string& error, std::string& usage )
{
  return ActionFactory::Instance()->create_action( actionstring, action, error, usage );
}

} // end namespace Core
