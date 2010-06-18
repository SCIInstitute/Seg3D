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

#ifndef CORE_STATE_STATEHANDLER_H
#define CORE_STATE_STATEHANDLER_H

// STL includes
#include <vector>
#include <string>

// Boost includes
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// State includes
#include <Core/State/StateEngine.h>
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

class StateHandler : public Core::ConnectionHandler
{

  // -- constructor/destructor --
public:
  StateHandler( const std::string& type_str, bool auto_id, int save_priority = -1 );
  virtual ~StateHandler();

public:
  // ADD_STATE:
  // Add a local state variable with default value
  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const T& default_value )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_value ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  // Add a local state variable with default value and min and max
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
  // Add a local state variable without default value
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state )
  {
    state = HANDLE( new typename HANDLE::element_type(
      this->create_state_id( key ) ) );
      
    return this->add_statebase( state );
  }

  // ADD_STATE:
  // Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::string& option_list )
  { 
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_option, option_list ) );
    return this->add_statebase( state );
  }

  // ADD_STATE:
  // Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::vector< std::string > option_list )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_option, option_list ) );
    return this->add_statebase( state );
  }

    // ADD_STATE:
  // Add a local state variable with option list
  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::vector< OptionLabelPair > option_list )
  {
    state = HANDLE( new typename HANDLE::element_type( 
      this->create_state_id( key ), default_option, option_list ) );
    return this->add_statebase( state );
  }


  // ADD_STATE:
  // Add a local state variable without default value
  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const std::vector< T >& default_value )
  {
    state = HANDLE( new typename HANDLE::element_type(
      this->create_state_id( key ), default_value ) );
      
    return this->add_statebase( state );
  }

public:
  // POPULATE_SESSION_STATES:
  // This function will save all the state values of this StateHandler to the list of states
  // that is stored in the StateEngine
  bool populate_session_states();

  // LOAD_STATES:
  // This function will load all the state values of this StateHandler from the list of states
  // that it is passed
  bool load_states( std::vector< std::string >& states_vector );

  // IMPORT_STATES:
  // This function is called on StateHandlers that need to have its states loaded from a particular
  // location that is seperate from the session states
  bool import_states( boost::filesystem::path path, const std::string& name );

  // EXPORT_STATES:
  // This function is called on StateHandlers that need to have its states saved to a particular
  // location that is seperate from the session states
  bool export_states( boost::filesystem::path path, const std::string& name );


protected:
  // PRE_LOAD_STATES:
  // This virtual function can be implemented in the StateHandlers and will be called before its
  // states are loaded.  If it doesn't succeed it needs to return false.
  virtual bool pre_load_states();

  // POST_LOAD_STATES:
  // This virtual function can be implemented in the StateHandlers and will be called after its
  // states are loaded.  If it doesn't succeed it needs to return false.
  virtual bool post_load_states();

  // PRE_SAVE_STATES:
  // This virtual function can be implemented in the StateHandlers and will be called before its
  // states are saved.  If it doesn't succeed it needs to return false.
  virtual bool pre_save_states();

  // POST_SAVE_STATES:
  // This virtual function can be implemented in the StateHandlers and will be called after its
  // states are saved.  If it doesn't succeed it needs to return false.
  virtual bool post_save_states();
  

protected:
  // STATE_CHANGED:
  // This function is called when any of the state variables are changed
  virtual void state_changed();

public:
  // GET_STATEHANDLER_ID:
  // The id of the handler that will be the prefix of the state variables
  const std::string& get_statehandler_id() const;

  // GET_STATEHANDLER_ID_NUMBER:
  // The id number of the handler that will be at the end of the prefix
  size_t get_statehandler_id_number() const;

  // GET_SAVE_TO_DISK:
  // This function returns a bool indicating whether this statehandler should be save to disk
  int get_save_priority();


private:
  // HANDLE_STATE_CHANGED:
  // This function is called whenever a state registered with this statehandler is changed
  void handle_state_changed();

  // CREATE_STATE_ID:
  // Create the full state id
  std::string create_state_id( const std::string& key ) const;

private:
  friend class StateEngine;

  // ADD_STATEBASE:
  // Function that adds the state variable to the database
  bool add_statebase( StateBaseHandle state );

  // GET_STATE (CALLED BY STATEENGINE):
  // Get the state variable
  bool get_state( const std::string& state_id, StateBaseHandle& state );
  
  // GET_STATE (CALLED BY STATEENGINE):
  // Get the state variable
  bool get_state( const size_t idx, StateBaseHandle& state );
  
  // NUMBER_OF_STATES:
  // Get the number of state variables stored in this statehandler
  size_t number_of_states() const;

  StateHandlerPrivate* private_;

};

} // end namespace Core

#endif
