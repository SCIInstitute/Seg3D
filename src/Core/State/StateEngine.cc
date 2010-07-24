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

// Boost includes
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// STL includes
#include <string>

// Application includes
#include <Core/State/StateEngine.h>
#include <Core/State/StateHandler.h>
#include <Core/Utils/AtomicCounter.h>

namespace Core
{

typedef std::map< std::string, Core::AtomicCounterHandle > 
  state_handler_counter_map_type;
typedef std::map< std::string, StateHandler* > state_handler_map_type;

class StateEnginePrivate
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

bool  StateEngine::load_session_states()
{
//  lock_type lock( get_mutex() );
  
  state_handler_map_type::iterator it = this->private_->state_handler_map_.begin();
  state_handler_map_type::iterator it_end = this->private_->state_handler_map_.end();

  std::vector< std::string > handler_order;
  handler_order.resize( 3 );
  while( it != it_end )
  {
    if( ( ( *it ).second->get_save_priority() ) != -1 )
    {
      handler_order[ ( *it ).second->get_save_priority() ] = ( *it ).first;
    }
    ++it;
  }
  for( int i = 0; i < 3; ++i )
  {
    if( !( * ( this->private_->state_handler_map_.find( handler_order[ i ] ) ) ).second->
        load_states( this->private_->session_states_ ) )
    {
      return false;
    }
  }

  return true;
}


bool StateEngine::populate_session_vector()
{
  lock_type lock( get_mutex() );
  
  this->private_->session_states_.clear();

  state_handler_map_type::iterator it = this->private_->state_handler_map_.begin();
  state_handler_map_type::iterator it_end = this->private_->state_handler_map_.end();

  while( it != it_end )
  {
    if( ( ( *it ).second->get_save_priority() ) != -1 )
    {
      if( !( *it ).second->populate_session_states() )
        return false;
    }
    ++it;
  }
  return true;
}


bool StateEngine::get_state( const std::string& state_id, StateBaseHandle& state )
{
  lock_type lock( get_mutex() );

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
  lock_type lock( get_mutex() );

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
  lock_type lock( get_mutex() );

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


std::string StateEngine::register_state_handler( const std::string &type_str, 
  Core::StateHandler* state_handler, bool auto_id )
{
  lock_type lock( this->get_mutex() );

  std::string handler_id;
  if ( auto_id )
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
  // Here we handle the case where we have previously loaded layers from file and the layer
  // counting has already begun.
  else
  {
    if( SplitString( type_str, "_" ).size() > 1 )
    {
      std::string name = SplitString( type_str, "_" )[ 0 ];
      long new_count = boost::lexical_cast< long >( SplitString( type_str, "_" )[ 1 ] );
      
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
    }

    handler_id = type_str;
  }

  if ( this->private_->state_handler_map_.find( handler_id ) !=
    this->private_->state_handler_map_.end() )
  {
    CORE_THROW_LOGICERROR( std::string( "Trying to register state handler " ) +
      handler_id + " that already exists " );
  }

  this->private_->state_handler_map_[ handler_id ] = state_handler;

  CORE_LOG_DEBUG( std::string( "State handler registered with ID " ) + handler_id );

  return handler_id;
}

void StateEngine::remove_state_handler( const std::string& handler_id )
{
  lock_type lock( this->get_mutex() );
  if ( this->private_->state_handler_map_.erase( handler_id ) == 0 )
  {
    CORE_LOG_ERROR( std::string( "Trying to remove a state handler that does not exist: " ) +
      handler_id );
  }
}

void StateEngine::get_session_states( std::vector< std::string >& states )
{
  lock_type lock( this->get_mutex() );
  states = this->private_->session_states_;
}

void StateEngine::set_session_states( std::vector< std::string >& states )
{
  lock_type lock( this->get_mutex() );
  this->private_->session_states_ = states;
}

} // end namespace Core
