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

#include <iostream>

#include <Utils/Converter/StringParser.h>
#include <Utils/Converter/StringConverter.h>

#include <Application/Action/ActionFactory.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ActionFactory );

// Definition of the internals of the class
class ActionFactoryPrivate
{
public:
  // Types used internally
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;

  // Mutex protecting the singleton interface
  typedef boost::unordered_map<std::string,ActionBuilderBase*> action_map_type;
  // List with builders that can be called to generate a new object
  action_map_type action_builders_;
  
  // Mutex for protecting registration
  mutex_type mutex_;
};
  

ActionFactory::ActionFactory() :
  private_( new ActionFactoryPrivate )
{
}

ActionFactory::~ActionFactory()
{
}

void ActionFactory::register_action( ActionBuilderBase* builder, std::string name )
{
  // Lock the factory
  lock_type lock( get_mutex() );

  // Ensure the name will be treaded case insensitive
  boost::to_lower( name );

  // Test is action was registered before.
  if ( private_->action_builders_.find( name ) != private_->action_builders_.end() )
  {
    // Actions that are registered twice, will cause problems
    // Hence the program will throw an exception.
    // As registration is done on startup, this will cause a
    // faulty program to fail always on startup.
    SCI_THROW_LOGICERROR( std::string( "Action '" ) + name + "' was registered twice" );
  }

  // Register the action
  private_->action_builders_[ name ] = builder;
  SCI_LOG_DEBUG( std::string("Registering action : ") +  name );  
}

bool ActionFactory::create_action( const std::string& action_string, ActionHandle& action,
    std::string& error, std::string& usage )
{
  ActionFactoryPrivate::lock_type lock( private_->mutex_ );
  std::string command;
  std::string::size_type pos = 0;

  usage = "";

  // Scan for the command that needs to be instanted.
  if ( !( Utils::ScanCommand( action_string, pos, command, error ) ) )
  {
    error = std::string( "SYNTAX ERROR: " ) + error;
    return ( false );
  }

  boost::to_lower( command );
  // NOTE: Factory is not locked as we assume that all actions are already
  // inserted.
  ActionFactoryPrivate::action_map_type::const_iterator it = private_->action_builders_.find( command );

  // If we cannot find the maker report error.
  if ( it == private_->action_builders_.end() )
  {
    error = std::string( "SYNTAX ERROR: Unknown command '" + command + "'" );
    return ( false );
  }

  // Build the action of the right type
  action = (*it).second->build();

  if ( !( action->import_from_string( action_string, error ) ) )
  {
    // the import_from_string function reports the error and hence
    // we do not need to set it here.

    // The action did build but the argument list is incorrect
    // Post the usage of the action for the user to help troubleshooting.
    usage = action->usage();
    return ( false );
  }

  return ( true );
}

bool ActionFactory::action_list( action_list_type& action_list )
{
  ActionFactoryPrivate::lock_type lock( private_->mutex_ );

  ActionFactoryPrivate::action_map_type::iterator it = private_->action_builders_.begin();
  ActionFactoryPrivate::action_map_type::iterator it_end = private_->action_builders_.end();

  while ( it != it_end )
  {
    action_list.push_back( ( *it ).first );
    ++it;
  }

  std::sort( action_list.begin(), action_list.end() );

  // indicate success
  return ( true );
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

} // end namespace seg3D
