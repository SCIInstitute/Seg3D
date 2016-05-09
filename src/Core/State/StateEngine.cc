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
#include <string>
#include <queue>
#include <map>

// Application includes
#include <Core/State/StateEngine.h>
#include <Core/State/StateHandler.h>
#include <Core/Utils/AtomicCounter.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

namespace Core
{

typedef std::map< std::string, Core::AtomicCounterHandle > 
  state_handler_counter_map_type;
typedef std::map< std::string, StateHandler* > state_handler_map_type;

class StateEnginePrivate : public RecursiveLockable
{
public:
  StateEnginePrivate() {}
  ~StateEnginePrivate() {}

  state_handler_counter_map_type state_handler_counter_map_;
  state_handler_map_type state_handler_map_;
  
  std::vector< std::string > session_states_; 
};

CORE_SINGLETON_IMPLEMENTATION( StateEngine );

StateEngine::StateEngine()
{
  this->private_ = new StateEnginePrivate;
}

StateEngine::~StateEngine()
{
  delete this->private_;
}

typedef std::pair< int, std::string > HandlerEntry;

bool operator<( const HandlerEntry& left, const HandlerEntry& right )
{
  return left.first < right.first;
}

bool operator>( const HandlerEntry& left, const HandlerEntry& right )
{
  return left.first > right.first;
}

bool  StateEngine::load_states( const StateIO& state_io )
{
  this->pre_load_states_signal_();

  // Put all the current state handlers in a priority queue in the descending order of priorities
  std::priority_queue< HandlerEntry > state_handlers;
  {
    StateEnginePrivate::lock_type lock( this->private_->get_mutex() );
    state_handler_map_type::iterator it = this->private_->state_handler_map_.begin();
    state_handler_map_type::iterator it_end = this->private_->state_handler_map_.end();
    while ( it != it_end )
    {
      if ( ( *it ).second->get_session_priority() != -1 )
      {
        state_handlers.push( std::make_pair( ( *it ).second->get_session_priority(),
          ( *it ).second->get_statehandler_id() ) );
      }
      ++it;
    }
  }

  bool success = true;
  // Call load_states on each state handler if it still exists.
  while ( !state_handlers.empty() )
  {
    std::string statehandler_id = state_handlers.top().second;
    state_handlers.pop();

    StateEnginePrivate::lock_type lock( this->private_->get_mutex() );
    state_handler_map_type::iterator it = this->private_->
      state_handler_map_.find( statehandler_id );
    if ( it != this->private_->state_handler_map_.end() )
    {
      lock.unlock();
      if ( ! ( *it ).second->load_states( state_io  ) )
      {
        success = false;
        std::string error = std::string( "Could not load states for '" ) + statehandler_id +
          "'.";
        CORE_LOG_ERROR( error );
      }
    }
  }

  this->post_load_states_signal_();
  return success;
}

bool StateEngine::save_states( StateIO& state_io )
{
  // Put all the current state handlers in a priority queue in the ascending order of priorities
  std::priority_queue< HandlerEntry, std::vector< HandlerEntry>, 
    std::greater< HandlerEntry > > state_handlers;
  {
    StateEnginePrivate::lock_type lock( this->private_->get_mutex() );
    state_handler_map_type::iterator it = this->private_->state_handler_map_.begin();
    state_handler_map_type::iterator it_end = this->private_->state_handler_map_.end();
    while ( it != it_end )
    {
      if ( ( *it ).second->get_session_priority() != -1 )
      {
        state_handlers.push( std::make_pair( ( *it ).second->get_session_priority(),
          ( *it ).second->get_statehandler_id() ) );
      }
      ++it;
    }
  }

  bool success = true;
  while ( !state_handlers.empty() )
  {
    std::string statehandler_id = state_handlers.top().second;
    state_handlers.pop();

    StateEnginePrivate::lock_type lock( this->private_->get_mutex() );
    state_handler_map_type::iterator it = this->private_->
      state_handler_map_.find( statehandler_id );
    if ( it != this->private_->state_handler_map_.end() )
    {
      lock.unlock();
      if( !( *it ).second->save_states( state_io ) ) success = false;
    }
  }

  return success;
}

bool StateEngine::get_state( const std::string& state_id, StateBaseHandle& state )
{
  state.reset();

  std::string state_handler_id;
  size_t loc = state_id.find( "::" );
  if ( loc != std::string::npos )
  {
    state_handler_id = state_id.substr( 0, loc );
  }
  else
  {
    CORE_LOG_ERROR( std::string( "Invalid state ID " ) + state_id );
    return false;
  }

  StateEnginePrivate::lock_type lock( this->private_->get_mutex() );

  state_handler_map_type::iterator it = this->private_->state_handler_map_.
    find( state_handler_id );
  if ( it != this->private_->state_handler_map_.end() )
  {
    return ( *it ).second->get_state( state_id, state );
  }

  return false;
}

size_t StateEngine::number_of_states()
{
  StateEnginePrivate::lock_type lock( this->private_->get_mutex() );

  state_handler_map_type::iterator it = this->private_->state_handler_map_.begin();
  state_handler_map_type::iterator it_end = this->private_->state_handler_map_.end();

  size_t num_states = 0;
  while ( it != it_end )
  {
    num_states += (*it).second->number_of_states();
    ++it;
  }

  return num_states;
}

bool StateEngine::get_state( const size_t idx, StateBaseHandle& state)
{
  StateEnginePrivate::lock_type lock( this->private_->get_mutex() );

  state_handler_map_type::iterator it = this->private_->state_handler_map_.begin();
  state_handler_map_type::iterator it_end = this->private_->state_handler_map_.end();

  size_t num_states = 0;
  
  state.reset();
    
  while ( it != it_end )
  {
    if ( idx < num_states + (*it).second->number_of_states() )
    {
      // We are in range
      return (*it).second->get_state( idx - num_states, state );
    }
    num_states += (*it).second->number_of_states();
    ++it;
  }
  
  return false;
}

size_t StateEngine::get_next_statehandler_count( const std::string& stateid )
{
  StateEnginePrivate::lock_type lock( this->private_->get_mutex() );

  state_handler_counter_map_type::iterator it = 
    this->private_->state_handler_counter_map_.find( stateid );

  if ( it == this->private_->state_handler_counter_map_.end() ) return 0;
  else return *( *it ).second;
}

void StateEngine::set_next_statehandler_count( const std::string& stateid, size_t count )
{
  StateEnginePrivate::lock_type lock( this->private_->get_mutex() );

  state_handler_counter_map_type::iterator it = 
    this->private_->state_handler_counter_map_.find( stateid );

  if ( it == this->private_->state_handler_counter_map_.end() ) 
  {
    Core::AtomicCounterHandle state_handler_counter;
    state_handler_counter = Core::AtomicCounterHandle( 
      new Core::AtomicCounter( static_cast< long >( count ) ) );
    this->private_->state_handler_counter_map_.insert( 
      state_handler_counter_map_type::value_type( stateid, state_handler_counter  ) );
  }
  else
  {
    ( *it ).second = Core::AtomicCounterHandle( new Core::AtomicCounter( 
      static_cast< long >( count ) ) );
  }
}


std::string StateEngine::register_state_handler( const std::string &type_str, 
  Core::StateHandler* state_handler, bool auto_id )
{
  StateEnginePrivate::lock_type lock( this->private_->get_mutex() );

  std::vector<std::string> id_components = Core::SplitString( type_str, "_" ); 

  std::string handler_id;
  if ( auto_id )
  {
    if ( id_components.size() > 1 )
    {
      std::string name = id_components[ 0 ];
      int new_count;
      Core::ImportFromString( id_components[ 1 ], new_count );
      
      Core::AtomicCounterHandle state_handler_counter;
      state_handler_counter_map_type::iterator it = 
        this->private_->state_handler_counter_map_.find( name );
      
      if ( it == this->private_->state_handler_counter_map_.end() )
      {
        state_handler_counter = Core::AtomicCounterHandle( 
          new Core::AtomicCounter( new_count ) );
        this->private_->state_handler_counter_map_.insert( 
          state_handler_counter_map_type::value_type( name, state_handler_counter ) );
      }
      else
      { 
        if( ( *( *it ).second ) < new_count )
        {
          new_count++;
          ( *it ).second = Core::AtomicCounterHandle( new Core::AtomicCounter( new_count ) );
        }
      }
      
      handler_id = type_str;
    }
    else
    {
    
      Core::AtomicCounterHandle state_handler_counter;
      std::string id_number = "_0";
      state_handler_counter_map_type::iterator it = 
        this->private_->state_handler_counter_map_.find( type_str );
      
      if ( it == this->private_->state_handler_counter_map_.end() )
      {
        state_handler_counter = Core::AtomicCounterHandle( new Core::AtomicCounter );
        this->private_->state_handler_counter_map_.insert( 
          state_handler_counter_map_type::value_type( type_str, state_handler_counter ) );
      }
      else
      {
        state_handler_counter = ( *it ).second;
        id_number = "_" + Core::ExportToString( ++( *state_handler_counter ) );
      }
    
      handler_id = type_str + id_number;
    }
  }
  else
  {
    handler_id = type_str;
  }

  if ( this->private_->state_handler_map_.find( handler_id ) !=
    this->private_->state_handler_map_.end() )
  {
    CORE_THROW_LOGICERROR( std::string( "Trying to register state handler " ) +
      handler_id + " that already exists " );
  }

  this->private_->state_handler_map_[ handler_id ] = state_handler;

  CORE_LOG_MESSAGE( std::string( "State handler registered with ID " ) + handler_id );

  return handler_id;
}

void StateEngine::remove_state_handler( const std::string& handler_id )
{
  StateEnginePrivate::lock_type lock( this->private_->get_mutex() );

  if ( this->private_->state_handler_map_.erase( handler_id ) == 0 )
  {
    CORE_LOG_ERROR( std::string( "Trying to remove a state handler that does not exist: " ) +
      handler_id );
  }
}

StateEngine::mutex_type& StateEngine::get_mutex() const
{
  return Application::GetMutex();
}

} // end namespace Core
