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
#include <queue>

// TinyXML includes
#include <tinyxml.h>

// Core includes
#include <Core/State/StateHandler.h>
#include <Core/State/StateEngine.h>
#include <Core/State/StateIO.h>
#include <Core/Application/Application.h>
#include <Core/Utils/Log.h>

namespace Core
{

typedef std::map< std::string, StateBaseHandle > state_map_type;
const std::string STATE_ELEMENT_NAME( "State" );

class StateHandlerPrivate
{
public:
  // The id of this state handler
  std::string statehandler_id_;

  // The base of the state handler id
  std::string statehandler_id_base_;

  // The number at the end of the state handler id
  size_t statehandler_id_number_;

  // The database with the actual states 
  state_map_type state_map_;

  // Check whether this state handler is still valid ( invalidate will unset this )
  bool valid_;
  
  // Keep track of which state was switched on
  bool signals_enabled_;

  // Keep track of whether the statehandler is being initialized
  bool initializing_;

  // Keep track whether mark as project data has been set
  bool mark_as_project_data_;

  // Do not save the id number in the xml files
  bool do_not_save_id_number_;

  // The version of the data that was loaded
  int loaded_version_;
};


StateHandler::StateHandler( const std::string& type_str, bool auto_id )
{
  // Initialize the private class
  this->private_ = new StateHandlerPrivate;

  // Create a new state handler id
  this->private_->statehandler_id_ = StateEngine::Instance()->
    register_state_handler( type_str, this, auto_id );
    
  // Get the current instantiation number of the state handler
  this->private_->statehandler_id_number_ = 0;
  std::string::size_type loc = this->private_->statehandler_id_.size();

  while (  loc > 0 && this->private_->statehandler_id_[ loc - 1 ] >= '0' && 
      this->private_->statehandler_id_[ loc - 1 ] <= '9' ) loc--;
  
  if ( loc >= this->private_->statehandler_id_.size() )
  {
    this->private_->statehandler_id_number_ = 0;
    this->private_->statehandler_id_base_ = this->private_->statehandler_id_;
  }
  else
  {
    ImportFromString( this->private_->statehandler_id_.substr( loc ), 
      this->private_->statehandler_id_number_ );
    this->private_->statehandler_id_base_ = 
      this->private_->statehandler_id_.substr( 0, loc - 1 );
  }
  
  // Set defaults for all the other fields in the private class
  this->private_->valid_ = true;
  this->private_->signals_enabled_ = true;
  this->private_->initializing_ = false;
  this->private_->mark_as_project_data_ = false;
  this->private_->do_not_save_id_number_ = false;
  this->private_->loaded_version_ = -1;
}

StateHandler::~StateHandler()
{
  this->disconnect_all();
  if( this->private_->valid_ )
  {
    StateEngine::Instance()->remove_state_handler( this->private_->statehandler_id_ );
  }
  delete this->private_;
}

bool StateHandler::add_statebase( StateBaseHandle state )
{
  // Step (1): Get unique state id
  std::string stateid = state->get_stateid();

  // Step (2): Link with statehandler
  this->add_connection( state->state_changed_signal_.connect( boost::bind(
      &StateHandler::handle_state_changed, this ) ) );

  // Step (3): Add the state to the map
  this->private_->state_map_[ stateid ] = state;

  // Step (4): copy current state to state variable
  state->enable_signals( this->private_->signals_enabled_ );
  state->set_initializing( this->private_->initializing_ );

  if ( this->private_->mark_as_project_data_ )
  {
    state->set_is_project_data( true );
  }

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

const std::string& StateHandler::get_statehandler_id_base() const
{
  return ( this->private_->statehandler_id_base_ );
}


size_t StateHandler::get_statehandler_id_number() const
{
  return ( this->private_->statehandler_id_number_ );
}

int StateHandler::get_session_priority()
{
  return -1;
}

int StateHandler::get_version()
{
  // Default version of any state handler
  return 1;
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

typedef std::pair< std::string, StateBaseHandle > StateEntry;

bool operator<( const StateEntry& left, const StateEntry& right )
{
  return left.second->get_session_priority() < right.second->get_session_priority();
}

bool StateHandler::load_states( const StateIO& state_io )
{
  // Get the XML element corresponding to this state handler
  
  const TiXmlElement* sh_element = 0;
  if ( this->private_->do_not_save_id_number_ )
  {
    sh_element = state_io.get_current_element()->
      FirstChildElement( this->get_statehandler_id_base().c_str() );

    if ( sh_element == 0 )
    {
      // Nothing to load, treat it as successful
      return true;
    }
  }
  else
  {
    sh_element = state_io.get_current_element()->
      FirstChildElement( this->get_statehandler_id().c_str() ); 
    if ( sh_element == 0 )
    {
      // Nothing to load, treat it as successful
      return true;
    }

  }
   
  bool success;

  state_io.push_current_element();
  state_io.set_current_element( sh_element );
  success = this->pre_load_states( state_io );
  state_io.pop_current_element();

  // Only continue if pre_load_states succeeded.
  if ( !success ) return false;

  // Query the version number in the loaded XML file.
  // NOTE: If the call fails, loaded_verison will not be changed, and thus is the same
  // as the current version number.
  int loaded_version = static_cast< int >( this->get_version() );
  sh_element->QueryIntAttribute( "version", &loaded_version );
  this->set_loaded_version( loaded_version );
  
  // Build a priority queue of all the states sorted in the descending order of
  // their session priority. Only  state variables with priority other than 
  // StateBase::DO_NOT_LOAD_E will be loaded.
  std::priority_queue< StateEntry > state_queue;
  state_map_type::iterator it = this->private_->state_map_.begin();
  state_map_type::iterator it_end = this->private_->state_map_.end();
  while ( it != it_end )
  {
    if ( ( *it ).second->get_session_priority() != StateBase::DO_NOT_LOAD_E )
    {
      state_queue.push( *it );
    }
    ++it;
  }

  // Build a map of state IDs and value strings by reading all the state elements.
  std::map< std::string, std::string > state_value_str_map;
  const TiXmlElement* state_element = 
    sh_element->FirstChildElement( STATE_ELEMENT_NAME.c_str() );

  while ( success && state_element != 0 )
  {
    const char* stateid = state_element->Attribute( "id" );
    if ( stateid == 0 )
    {
      CORE_LOG_ERROR( "Invalid state record with no id" );
      success = false;
    }
    else
    {
      const char* state_value_str = state_element->GetText();
      if ( state_value_str == 0 )
      {
        state_value_str = "";
      }
      
      state_value_str_map[ this->get_statehandler_id() + "::" + stateid ] = state_value_str;      
    }

    state_element = state_element->NextSiblingElement( STATE_ELEMENT_NAME );
  }
  
  if ( !success )
  {
    return false;
  }

  // TODO:
  // We need a virtual function here, that allows the values in state_value_str_map to be 
  // translated into the latest version
  // --JS

  // Import the state values in the correct order.
  while ( !state_queue.empty() )
  {
    StateEntry state_entry = state_queue.top();
    state_queue.pop();
    std::map< std::string, std::string >::iterator state_it =
      state_value_str_map.find( state_entry.first );
    if ( state_it != state_value_str_map.end() )
    {
      if ( !state_entry.second->import_from_string( ( *state_it ).second ) )
      {
        std::string error = std::string( "Failed to import '" ) + state_entry.first + "'.";
        CORE_LOG_ERROR( error );
        success = false;
      }
    }
  }
  
  // If the loading was successful, run post loading process.
  if ( success )
  {
    state_io.push_current_element();
    state_io.set_current_element( sh_element );
    success &= this->post_load_states( state_io );
    state_io.pop_current_element();
  }
  
  return success;
}

bool StateHandler::pre_load_states( const StateIO& state_io )
{
  // Do nothing.
  return true;
}

bool StateHandler::post_load_states( const StateIO& state_io )
{
  // Do nothing.
  return true;
}

bool StateHandler::pre_save_states( StateIO& state_io )
{
  // Do nothing.
  return true;
}

bool StateHandler::post_save_states( StateIO& state_io )
{
  // Do nothing.
  return true;
}

void StateHandler::handle_state_changed()
{
  // Trigger the signal in the state engine
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
    StateEngine::lock_type lock( StateEngine::GetMutex() );
    if( !this->private_->valid_ )
    {
      return;
    }
    this->private_->valid_ = false;
    StateEngine::Instance()->remove_state_handler( this->private_->statehandler_id_ );
  }

  state_map_type::iterator it_end = this->private_->state_map_.end();
  state_map_type::iterator it = this->private_->state_map_.begin();
  while ( it != it_end )
  {
    ( *it ).second->invalidate();
    ++it;
  }
    
  this->clean_up();

}

void StateHandler::enable_signals( bool enabled )
{
  state_map_type::iterator it_end = this->private_->state_map_.end();
  state_map_type::iterator it = this->private_->state_map_.begin();
  while ( it != it_end )
  {
    ( *it ).second->enable_signals( enabled );
    it++;
  }

  this->private_->signals_enabled_ = enabled;
}


void StateHandler::set_initializing( bool initializing )
{
  state_map_type::iterator it_end = this->private_->state_map_.end();
  state_map_type::iterator it = this->private_->state_map_.begin();
  while ( it != it_end )
  {
    ( *it ).second->set_initializing( initializing );
    it++;
  }
  
  this->private_->initializing_ = initializing;
}

bool StateHandler::is_valid()
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );
  return this->private_->valid_;
}

void StateHandler::clean_up()
{
  // does nothing by default.
}

bool StateHandler::save_states( StateIO& state_io )
{
  TiXmlElement* sh_element;
  if ( this->private_->do_not_save_id_number_ )
  {
    sh_element = new TiXmlElement( this->get_statehandler_id_base() );
  }
  else
  {
    sh_element = new TiXmlElement( this->get_statehandler_id() );
  }

  state_io.get_current_element()->LinkEndChild( sh_element );
  sh_element->SetAttribute( "version", this->get_version() );
  
  state_io.push_current_element();
  state_io.set_current_element( sh_element );
  bool success = true;
  if( !this->pre_save_states( state_io ) )
  {
    std::string error = std::string( "Pre save states failed for " ) + 
      this->get_statehandler_id();
    CORE_LOG_ERROR( error );
    success = false;
  }
  
  state_map_type::iterator it = this->private_->state_map_.begin();
  state_map_type::iterator it_end = this->private_->state_map_.end();
  while ( it != it_end )
  {
    if ( ( *it ).second->get_session_priority() != StateBase::DO_NOT_LOAD_E )
    {
      TiXmlElement* state_element = new TiXmlElement( STATE_ELEMENT_NAME.c_str() );
      state_element->SetAttribute( "id", SplitString( ( *it ).first, "::" )[ 1 ].c_str() );
      state_element->LinkEndChild( new TiXmlText( ( *it ).second->export_to_string().c_str() ) );
      sh_element->LinkEndChild( state_element );
    }
    ++it;
  }

  if( !this->post_save_states( state_io ) )
  {
    std::string error = std::string( "Post save states failed for " ) + 
      this->get_statehandler_id();
    CORE_LOG_ERROR( error );
    success = false;
  }
  
  state_io.pop_current_element();
  return success;
}

void StateHandler::mark_as_project_data()
{
  this->private_->mark_as_project_data_ = true;
  
  state_map_type::iterator it_end = this->private_->state_map_.end();
  state_map_type::iterator it = this->private_->state_map_.begin();
  while ( it != it_end )
  {
    ( *it ).second->set_is_project_data( true );
    it++;
  }
}

void StateHandler::do_not_save_id_number()
{
  this->private_->do_not_save_id_number_ = true;
}

int StateHandler::get_loaded_version()
{
  return this->private_->loaded_version_;
}

void StateHandler::set_loaded_version( int loaded_version )
{
  this->private_->loaded_version_ = loaded_version;
}

} // end namespace Core
