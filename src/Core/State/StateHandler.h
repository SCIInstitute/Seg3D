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

#ifndef CORE_STATE_STATEHANDLER_H
#define CORE_STATE_STATEHANDLER_H

// STL includes
#include <vector>
#include <string>

// Boost includes
#include <boost/utility.hpp>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// State includes
#include <Core/State/StateRangedValue.h>
#include <Core/State/StateOption.h>
#include <Core/State/StateLabeledOption.h>
#include <Core/State/StateValue.h>
#include <Core/State/StateVector.h>
#include <Core/State/StateView2D.h>
#include <Core/State/StateView3D.h>

namespace Core
{

class StateHandler;
class StateHandlerPrivate;
class StateIO;

class StateHandler : protected ConnectionHandler
{

  // -- constructor/destructor --
public:
  StateHandler( const std::string& type_str, bool auto_id );
  virtual ~StateHandler();

public:
  // ADD_STATE:
  /// Add a local state variable with default value
  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const T& default_value )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_value ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable with default value and min and max
  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const T& default_value,
      const T& min_value, const T& max_value, const T& step )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_value, 
      min_value, max_value, step ) );
      
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable without default value
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state )
  {
    state = HANDLE( new typename HANDLE::element_type(
      this->create_state_id( key ) ) );
      
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::string& option_list )
  { 
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_option, option_list ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::vector< std::string > option_list )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_option, option_list ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::vector< OptionLabelPair > option_list )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_option, option_list ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, 
    const std::vector< std::string >& default_options,
    const std::vector< OptionLabelPair > option_list )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_options, option_list ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, 
    const std::vector< std::string >& default_options,
    const std::string& option_list )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_options, option_list ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  /// Add a local state variable without default value
  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const std::vector< T >& default_value )
  {
    state = HANDLE( new typename HANDLE::element_type(
      this->create_state_id( key ), default_value ) );
      
    return this->add_statebase( state );
  }

public:
  // INVALIDATE:
  /// this function is called when you need to delete something from the state engine, but may have 
  // lingering handles that need to be cleaned up
  
  /// NOTE: This is needed to ensure that all states are removed from the state engine while the
  /// UI can still have lingering handles to the object. This situation occurs because of the
  /// Qt and Application thread to be separate and the messaging is synchronous without any
  // confirmation when things have been updated.
  void invalidate();

  // IS_VALID:
  /// returns true if the function hasn't been invalidated
  bool is_valid();

  // MARK_AS_PROJECT_DATA:
  /// Mask the states that are added to this state handler as project data
  void mark_as_project_data();
  
  // DO_NOT_SAVE_ID_NUMBER
  /// The id number of the statehandler will not be saved
  void do_not_save_id_number();

protected:

  // CLEAN_UP:
  /// This function is called by invalidate to clean up stuff in the statehandler subclasses
  virtual void clean_up();

  // ENABLE_SIGNALS:
  /// This function enables/disables signals in the state variables
  void enable_signals( bool enabled );
  
public:
  // SET_INITIALIZING:
  /// This function denotes whether a state handler is initializing or not. During the initializing
  /// phase signals and thread checking are turned off
  void set_initializing( bool initializing );

  // TODO:
  // These should be private and only the StateEngine should have access as a friend of this class
  // --JGS

  // LOAD_STATES:
  /// Load the states into the StateIO variable
  bool load_states( const StateIO& state_io );

  // SAVE_STATES:
  /// Save the states into the StateIO variable
  bool save_states( StateIO& state_io );

protected:
  // PRE_LOAD_STATES:
  /// This virtual function can be implemented in the StateHandlers and will be called before its
  /// states are loaded.  If it doesn't succeed it needs to return false.
  virtual bool pre_load_states( const StateIO& state_io );

  // POST_LOAD_STATES:
  /// This virtual function can be implemented in the StateHandlers and will be called after its
  /// states are loaded.  If it doesn't succeed it needs to return false.
  virtual bool post_load_states( const StateIO& state_io );

  // PRE_SAVE_STATES:
  /// This virtual function can be implemented in the StateHandlers and will be called before its
  /// states are saved.  If it doesn't succeed it needs to return false.
  virtual bool pre_save_states( StateIO& state_io );

  // POST_SAVE_STATES:
  /// This virtual function can be implemented in the StateHandlers and will be called after its
  /// states are saved.  If it doesn't succeed it needs to return false.
  virtual bool post_save_states( StateIO& state_io );
  
  // STATE_CHANGED:
  /// This function is called when any of the state variables are changed and can be overloaded
  /// to implement a general function that needs to be called each time the state is updated.
  virtual void state_changed();

  // GET_SESSION_PRIORITY:
  /// Returns the session priority of the state handler. State handlers with higher priorities
  /// gets loaded earlier than those with lower priorities.
  /// By default this functions returns -1, which means that the state handler won't be
  /// saved/loaded by the state engine.
  virtual int get_session_priority();

  // GET_VERSION:
  /// Get the version number of this class, this one is used for versioning of session files
  virtual int get_version();
  
  // GET_LOADED_VERSION:
  /// Get the version number of the data that was loaded
  int get_loaded_version();
    
  // SET_LOADED_VERSION:
  /// Set the version number of the data that was loaded
  void set_loaded_version( int loaded_version );

public:
  // GET_STATEHANDLER_ID:
  /// The id of the handler that will be the prefix of the state variables
  const std::string& get_statehandler_id() const;

  // GET_STATEHANDLER_ID_BASE:
  /// The id of the handler that will be the prefix of the state variables
  const std::string& get_statehandler_id_base() const;

  // GET_STATEHANDLER_ID_NUMBER:
  /// The id number of the handler that will be at the end of the prefix
  size_t get_statehandler_id_number() const;

private:
  friend class StateEngine;

  // HANDLE_STATE_CHANGED:
  /// This function is called whenever a state registered with this statehandler is changed
  void handle_state_changed();

  // CREATE_STATE_ID:
  /// Create the full state id
  std::string create_state_id( const std::string& key ) const;

  // ADD_STATEBASE:
  /// Function that adds the state variable to the database, the functions add_state call
  /// this function to finalize the process of adding the state to the state engine
  bool add_statebase( StateBaseHandle state );

  // GET_STATE (CALLED BY STATEENGINE):
  /// Get the state variable
  bool get_state( const std::string& state_id, StateBaseHandle& state );
  
  // GET_STATE (CALLED BY STATEENGINE):
  /// Get the state variable
  bool get_state( const size_t idx, StateBaseHandle& state );
  
  // NUMBER_OF_STATES (CALLED BY STATEENGINE):
  /// Get the number of state variables stored in this statehandler
  size_t number_of_states() const;

  /// Internal implementation
  StateHandlerPrivate* private_;
};

} // end namespace Core

#endif
