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

#ifndef CORE_STATE_STATEENGINE_H
#define CORE_STATE_STATEENGINE_H

// Boost includes
#include <boost/noncopyable.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateBase.h>
#include <Core/Utils/Singleton.h>

namespace Core
{

// CLASS STATEENGINE

// Forward declaration
class StateEngine;
class StateEnginePrivate;
class StateHandler;
class StateIO;

// Class definition
class StateEngine : public boost::noncopyable
{
  CORE_SINGLETON( StateEngine );

  // -- Constructor/destructor --
private:
  StateEngine();
  virtual ~StateEngine();

  // -- Interface for accessing state variables --
public:
  // GET_STATE:
  /// Get pointer to the state variable based on the unique state tag
  bool get_state( const std::string& stateid, StateBaseHandle& state );

  // GET_STATE:
  /// Get the state variable by index
  bool get_state( const size_t idx, StateBaseHandle& state);

  // NUMBER_OF_STATES:
  /// The number of state variables in the system
  size_t number_of_states();
  
  // GET_NEXT_STATEHANDLER_COUNT:
  /// Get the count for the next state handler
  size_t get_next_statehandler_count( const std::string& stateid );
  
  // SET_NEXT_STATEHANDLER_COUNT:
  /// Reset the count to a certain number
  /// NOTE: This functionality is intended for redo
  void set_next_statehandler_count( const std::string& stateid, size_t count);

public:
  // LOAD_STATES:
  /// This function finds the StateHandlers that are saved to file by default and then
  /// sets their state variables from the values that have been loaded into session_states_
  bool load_states( const StateIO& state_io );

  bool save_states( StateIO& state_io );

  // -- mutex and lock --
public:
  typedef Application::mutex_type mutex_type;
  typedef Application::lock_type lock_type;

  mutex_type& get_mutex() const;
  
  // -- Interface for accounting stateids --
private:
  friend class StateHandler;
  
  //
  std::string register_state_handler( const std::string& type_str, 
    StateHandler* state_handler, bool auto_id );

  void remove_state_handler( const std::string& handler_id );
  
  // -- Signals --
public:
  // STATE_CHANGED_SIGNAL:
  /// This signal is triggered when a state in the state engine is changed
  typedef boost::signals2::signal<void ()> state_changed_signal_type;
  state_changed_signal_type state_changed_signal_;

  // PRE_LOAD_STATES_SIGNAL_:
  /// Triggered at the beginning of load_states function.
  boost::signals2::signal< void () > pre_load_states_signal_;

  // POST_LOAD_STATES_SIGNAL_:
  /// Triggered at the end of load_states function.
  boost::signals2::signal< void () > post_load_states_signal_;

  // -- Implementation details --
private:
  StateEnginePrivate* private_;

  // -- Static convenience functions --
public:

  // GETMUTEX
  /// Get the mutex of the state engine
  /// NOTE: Do not lock the StateEngine while RenderResources is locked as this will cause deadlock
  static mutex_type& GetMutex()
  {
    return Instance()->get_mutex();
  }
  
};

} // end namespace Core

#endif
