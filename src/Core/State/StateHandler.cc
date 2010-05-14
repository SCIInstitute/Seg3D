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

#include <Core/State/StateHandler.h>
#include <Core/State/StateEngine.h>

namespace Core
{

typedef std::map< std::string, StateBaseHandle > state_map_type;

class StateHandlerPrivate
{
public:
  StateHandlerPrivate() {}
  ~StateHandlerPrivate() {}

  state_map_type state_map_;
};

StateHandler::StateHandler( const std::string& type_str, bool auto_id )
{
  this->private_ = new StateHandlerPrivate;
  this->stateid_prefix_ = StateEngine::Instance()->
    register_state_handler( type_str, this, auto_id );
}

StateHandler::~StateHandler()
{
  this->disconnect_all();
  StateEngine::Instance()->remove_state_handler( this->stateid_prefix_ );
  delete this->private_;
}
    

bool StateHandler::add_statebase( StateBaseHandle state )
{
  // Step (1): Get unique state id
  std::string stateid = state->stateid();

  // Step (2): Import previous setting from the current variable
  StateBaseHandle old_state;
  this->get_state( stateid, old_state );

  if ( old_state.get() )
  {
    // use the string representation as intermediate
    state->import_from_string( old_state->export_to_string() );
  }

  // Step (3): Link with statehandler
  this->add_connection( state->state_changed_signal_.connect( boost::bind(
      &StateHandler::handle_state_changed, this ) ) );

  // Step (4): Add the state to the map
  this->private_->state_map_[ stateid ] = state;

  return true;
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

void StateHandler::handle_state_changed()
{
  // Trigger the signal in the state engine
  CORE_LOG_DEBUG("Triggering state changed signal");
  StateEngine::Instance()->state_changed_signal_();
  
  // Call the local function of this state engine that handles the specifics of the derived
  // class when the state engine has changed
  state_changed();
}

} // end namespace Core
