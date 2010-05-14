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

//bool StateEngine::add_state( const std::string& state_id, StateBaseHandle& state )
//{
//  lock_type lock( get_mutex() );
//
//  state_map_type::iterator it = state_map_.find( state_id );
//
//  if ( it != state_map_.end() )
//  {
//    // State is already there
//    state_map_.erase( it );
//  }
//
//  state_map_[ state_id ] = state;
//  state_list_.push_back(state);
//  
//  state_changed_signal_();
//  
//  return ( true );
//}

bool StateEngine::get_state( const std::string& state_id, StateBaseHandle& state )
{
  lock_type lock( get_mutex() );

  std::string state_handler_id;
  size_t loc = state_id.find( "::" );
  if ( loc != std::string::npos )
  {
    state_handler_id = state_id.substr( 0, loc );
  }
  else
  {
    state_handler_id = state_id;
  }

  state_handler_map_type::iterator it = this->private_->state_handler_map_.
    find( state_handler_id );
  if ( it != this->private_->state_handler_map_.end() )
  {
    return ( *it ).second->get_state( state_id, state );
  }

  state.reset();
  return false;


  //state_map_type::const_iterator it;
  //if ( state_id.size() > 0 && state_id[ 0 ] == '$' )
  //{
  //  // This name is an alias
  //  std::string state_alias = state_id.substr( 1 );
  //  // Find the name in the alias list
  //  if ( statealias_list_.find( state_alias ) == statealias_list_.end() )
  //  {
  //    state.reset();
  //    return ( false );
  //  }

  //  // fill in the proper state id
  //  it = state_map_.find( statealias_list_[ state_alias ] );
  //}
  //else
  //{
  //  it = state_map_.find( state_id );
  //}
  //if ( it == state_map_.end() )
  //{
  //  // make the handle invalid
  //  state.reset();
  //  return ( false );
  //}

  //state = StateBaseHandle( ( *it ).second );
  //return ( true );
}

//void StateEngine::remove_state( const std::string& remove_state_id )
//{
//  lock_type lock( get_mutex() );
//
//  // ensure that we can change it
//  std::string state_id = remove_state_id;
//
//  state_list_type::iterator it = state_list_.begin();
//  state_list_type::iterator it_end = state_list_.end();
//
//  while ( it != it_end )
//  {
//    std::string state_name = (*it)->stateid();
//    
//    if ( state_name.size() >= state_id.size() )
//    {
//      if ( state_name.compare( 0, state_id.size(), state_id, 0, state_id.size() ) == 0 )
//      {
//        if ( state_name.size() > state_id.size() )
//        {
//          if ( state_name[ state_id.size() ] == ':' )
//          {
//            state_map_.erase( state_map_.find( state_name ) );
//            it = state_list_.erase(it);
//            it_end = state_list_.end();
//            continue;
//          }
//        }
//        else
//        {
//          state_map_.erase( state_map_.find( state_name ) );
//          it = state_list_.erase(it);
//          it_end = state_list_.end();
//          continue;
//        }
//      }
//    }
//    ++it;
//  }
//  
//  state_changed_signal_();  
//}
//
size_t StateEngine::num_states()
{
  //lock_type lock( get_mutex() );
  //return state_list_.size();
  return 0;
}

bool StateEngine::get_state( const size_t idx, StateBaseHandle& state)
{
  //lock_type lock( get_mutex() );
  //if (idx < state_list_.size())
  //{
  //  state = state_list_[idx];
  //  return true;
  //}
  //else
  {
    state.reset();
    return false;
  }
}

//void StateEngine::add_stateid( const std::string& stateid )
//{
//  lock_type lock( get_mutex() );
//  if ( stateid_list_.find( stateid ) != stateid_list_.end() )
//  {
//    CORE_THROW_LOGICERROR( std::string("Trying to add stateid '") + stateid +
//      std::string("' that already exists") );
//  }
//  stateid_list_.insert( stateid );
//}
//
//void StateEngine::remove_stateid( const std::string& stateid )
//{
//  lock_type lock( get_mutex() );
//  stateid_list_.erase( stateid );
//}
//
//bool StateEngine::is_stateid( const std::string& stateid )
//{
//  lock_type lock( get_mutex() );
//  return ( stateid_list_.find( stateid ) != stateid_list_.end() );
//}
//
//std::string StateEngine::create_stateid( std::string baseid )
//{
//  // Check
//  std::string::size_type loc = baseid.find_last_of( '_' );
//  if ( loc != std::string::npos )
//  {
//    // there is an under score in the name
//    // check whether the last part is a name
//    bool is_number = true;
//    if ( loc == baseid.size()-1 ) is_number = false;
//  
//    for ( std::string::size_type j = loc + 1; j < baseid.size(); j++ )
//    {
//      if ( baseid[j] < '0' || baseid[j] > '9' ) is_number = false;
//    }
//    
//    if ( is_number ) baseid = baseid.substr( 0, loc );
//  }
//
//  lock_type lock( get_mutex() );
//  
//  int number = 1;
//  std::string new_stateid;
//  
//  do
//  {
//    new_stateid = baseid + std::string( "_" ) + ExportToString( number );
//    number++;
//  }
//  while ( stateid_list_.find( new_stateid ) != stateid_list_.end() );
//
//  return new_stateid;
//}

std::string StateEngine::register_state_handler( const std::string &type_str, 
  Core::StateHandler* state_handler, bool auto_id )
{
  lock_type lock( this->get_mutex() );

  std::string handler_id;
  if ( auto_id )
  {
    Core::AtomicCounterHandle state_handler_counter;
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
    }

    handler_id = type_str + Core::ExportToString( ( *state_handler_counter )++ );
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

} // end namespace Core
