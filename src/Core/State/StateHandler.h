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

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// State includes
#include <Core/State/StateEngine.h>
#include <Core/State/StateRangedValue.h>
#include <Core/State/StateOption.h>
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
  StateHandler( const std::string& type_str, bool auto_id );
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
  // Add a local state variable without default value
  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const std::vector< T >& default_value )
  {
    state = HANDLE( new typename HANDLE::element_type(
      this->create_state_id( key ), default_value ) );
      
    return this->add_statebase( state );
  }

protected:
  // STATE_CHANGED:
  // This function is called when any of the state variables are changed
  virtual void state_changed();

public:
  // GET_STATEHANDLER_ID:
  // The id of the handler that will be the prefix of the state variables
  const std::string& get_statehandler_id() const;

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
