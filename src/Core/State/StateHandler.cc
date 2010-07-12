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
#include <boost/lexical_cast.hpp>

// TinyXML includes
#include <Externals/tinyxml/tinyxml.h>

#include <Core/State/StateHandler.h>
#include <Core/State/StateEngine.h>
#include <Core/State/StateIO.h>
#include <Core/Application/Application.h>

namespace Core
{

typedef std::map< std::string, StateBaseHandle > state_map_type;

class StateHandlerPrivate
{
public:
  // The id of this state handler
  std::string statehandler_id_;

  // The number at the end of the state handler id
  size_t statehandler_id_number_;

  int save_priority_;

  // The database with the actual states 
  state_map_type state_map_;

  size_t version_number_;

  bool valid_;

  boost::mutex mutex_;
};


StateHandler::StateHandler( const std::string& type_str, size_t version_number, bool auto_id,  int save_priority )
{
  this->private_ = new StateHandlerPrivate;
  this->private_->version_number_ = version_number;
  this->private_->statehandler_id_ = StateEngine::Instance()->
    register_state_handler( type_str, this, auto_id );
    
  this->private_->statehandler_id_number_ = 0;
  std::string::size_type loc = this->private_->statehandler_id_.size();
  while (  loc > 0 && this->private_->statehandler_id_[ loc - 1 ] >= '0' && 
      this->private_->statehandler_id_[ loc - 1 ] <= '9' ) loc--;
  
  if ( loc >= this->private_->statehandler_id_.size() )
  {
    this->private_->statehandler_id_number_ = 0;
  }
  else
  {
    ImportFromString( this->private_->statehandler_id_.substr( loc ), 
      this->private_->statehandler_id_number_ );
  }
  
  this->private_->save_priority_ = save_priority;

  this->private_->valid_ = true;
}

StateHandler::~StateHandler()
{
  this->disconnect_all();
  if( this->is_valid() )
  {
    StateEngine::Instance()->remove_state_handler( this->private_->statehandler_id_ );
  }
  delete this->private_;
}
    

bool StateHandler::add_statebase( StateBaseHandle state )
{
  // Step (1): Get unique state id
  std::string stateid = state->stateid();

  // Step (2): Link with statehandler
  this->add_connection( state->state_changed_signal_.connect( boost::bind(
      &StateHandler::handle_state_changed, this ) ) );

  // Step (3): Add the state to the map
  this->private_->state_map_[ stateid ] = state;

  return true;
}

void StateHandler::state_changed()
{
  // default function is to do nothing
}

const std::string& StateHandler::get_statehandler_id() const
{
  return ( this->private_->statehandler_id_ );
}

size_t StateHandler::get_statehandler_id_number() const
{
  return ( this->private_->statehandler_id_number_ );
}

int StateHandler::get_save_priority()
{
  return this->private_->save_priority_;
}


std::string StateHandler::create_state_id( const std::string& key ) const
{
  return this->get_statehandler_id() + "::" + key;
}

bool StateHandler::get_state( const std::string& state_id, StateBaseHandle& state )
{
  state_map_type::iterator it = this->private_->state_map_.find( state_id );
  if ( it != this->private_->state_map_.end() )
  {
    state = ( *it ).second;
    return true;
  }

  state.reset();
  return false;
}

bool StateHandler::get_state( const size_t idx, StateBaseHandle& state )
{
  state_map_type::iterator it = this->private_->state_map_.begin();
  
  if ( idx >= this->private_->state_map_.size() ) return false;
  
  std::advance( it, idx );
  
  state = ( *it ).second;
  return true;
}

bool StateHandler::populate_session_states()
{
  if( !pre_save_states() )
    return false;

  state_map_type::iterator it = this->private_->state_map_.begin();
  state_map_type::iterator it_end = this->private_->state_map_.end();

  std::vector< std::string > states;
  StateEngine::Instance()->get_session_states( states );

  // Like in XML which we are mimicking we surround the StateHandler's state variables
  // with its statehandler_id


  states.push_back( this->private_->statehandler_id_ );

  states.push_back( this->private_->statehandler_id_ +
    "*version*" + boost::lexical_cast< std::string >( this->private_->version_number_) );

  while ( it != it_end )
  {

    states.push_back( ( *it ).second->stateid() + "*"
      + ( *it ).second->export_to_string() );
    ++it;
  }
  
  // Like in XML which we are mimicking we surround the StateHandler's state variables
  // with its statehandler_id
  states.push_back( this->private_->statehandler_id_ );
  
  StateEngine::Instance()->set_session_states( states );

  return post_save_states();
}


bool StateHandler::load_states( std::vector< std::string >& states_vector )
{
  
  if( !pre_load_states() ) return false;

  size_t loaded_version = 0;

  for( int i = 0; i < static_cast< int >( states_vector.size() ); ++i )
  {
    if( states_vector[ i ] == this->private_->statehandler_id_ )
    { 
      i++;
      if( ( SplitString( states_vector[ i ], "*" ) )[ 1 ] == "version" )
      {
        loaded_version = boost::lexical_cast< size_t > 
          ( ( SplitString( states_vector[ i ], "*" ) )[ 2 ] );
        i++;
      }
      
      std::vector< std::string > state_value_as_string_vector; 
      while( states_vector[ i ] != this->private_->statehandler_id_ )
      {
        if( ( loaded_version == 0 ) || ( loaded_version == this->private_->version_number_ ) )
        {
          state_value_as_string_vector = 
            SplitString( states_vector[ i ], "*" );
          if( ( state_value_as_string_vector[ 0 ] != "" ) )//&& ( state_value_as_string_vector[ 1 ] != "" ) )
          {
            state_map_type::iterator it = this->private_->state_map_.find( state_value_as_string_vector[ 0 ] );
            if ( it != this->private_->state_map_.end() )
            {
              ( *it ).second->import_from_string( state_value_as_string_vector[ 1 ] );
            }           
          }
          else
          {
            return false;
          }
        }
        else
        {
          // TODO: version translation is called here.
        }
        i++;
      }
    }
  }
  return post_load_states();
}

bool StateHandler::import_states( boost::filesystem::path path, const std::string& name )
{
  std::vector< std::string > state_values;
  if( Core::StateIO::import_from_file( ( path / ( name ) ), state_values ) )
    return this->load_states( state_values );
  else
    return false;
}

bool StateHandler::export_states( boost::filesystem::path path, const std::string& name )
{

  if( !pre_save_states() )
    return false;

  state_map_type::iterator it = this->private_->state_map_.begin();
  state_map_type::iterator it_end = this->private_->state_map_.end();
  std::vector< std::string > state_values;
  
  state_values.push_back( this->private_->statehandler_id_ );
  state_values.push_back( this->private_->statehandler_id_ +
    "*version*" + boost::lexical_cast< std::string >( this->private_->version_number_) );
  while ( it != it_end )
  {
    state_values.push_back( ( *it ).second->stateid() + "*"
      + ( *it ).second->export_to_string() );
    ++it;
  }
  state_values.push_back( this->private_->statehandler_id_ );

  return Core::StateIO::export_to_file( ( path / ( name ) ), state_values );
}

bool StateHandler::pre_load_states()
{
  // Do nothing.
  return true;
}

bool StateHandler::post_load_states()
{
  // Do nothing.
  return true;
}

bool StateHandler::pre_save_states()
{
  // Do nothing.
  return true;
}

bool StateHandler::post_save_states()
{
  // Do nothing.
  return true;
}

void StateHandler::handle_state_changed()
{
  // Trigger the signal in the state engine
  CORE_LOG_DEBUG("Triggering state changed signal");

  StateEngine::Instance()->state_changed_signal_();
  
  // Call the local function of this state engine that handles the specifics of the derived
  // class when the state engine has changed
  state_changed();
}

size_t StateHandler::number_of_states() const
{
  return this->private_->state_map_.size();
}

void StateHandler::invalidate()
{
  {
    boost::mutex::scoped_lock lock( this->private_->mutex_ );
    if( !this->private_->valid_ )
    {
      return;
    }
    this->private_->valid_ = false;
  }

  state_map_type::iterator it_end = this->private_->state_map_.end();
  state_map_type::iterator it = this->private_->state_map_.begin();
  while ( it != it_end )
  {
    ( *it ).second->invalidate();
    it++;
  }
    
  this->clean_up();

  StateEngine::Instance()->remove_state_handler( this->private_->statehandler_id_ );
}

bool StateHandler::is_valid()
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  return this->private_->valid_;
}

void StateHandler::clean_up()
{
  // does nothing by default.
}




} // end namespace Core
