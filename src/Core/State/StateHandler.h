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
    // Step (1): Generate a new unique ID for this state
    std::string stateid = stateid_prefix_ + std::string( "::" ) + key;
  
    // Step (2): Generate the state variable
    state = HANDLE( new typename HANDLE::element_type( stateid, default_value ) );

    // Step (3): Now handle the common part for each add_state function
    return ( add_statebase( StateBaseHandle( state ) ) );
  }

  // ADD_STATE:
  // Add a local state variable with default value and min and max

  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const T& default_value,
      const T& min_value, const T& max_value, const T& step )
  {
    // Step (1): Generate a new unique ID for this state
    std::string stateid = stateid_prefix_ + std::string( "::" ) + key;
    
    // Step (2): Generate the state variable
    state = HANDLE( new typename HANDLE::element_type( stateid, default_value, 
      min_value, max_value, step ) );

    // Step (3): Now handle the common part for each add_state function
    return ( add_statebase( StateBaseHandle( state ) ) );
  }

  // ADD_STATE:
  // Add a local state variable without default value

  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state )
  {
    // Step (1): Generate a new unique ID for this state
    std::string stateid = stateid_prefix_ + std::string( "::" ) + key;
    
    // Step (2): Generate the state variable
    state = HANDLE( new typename HANDLE::element_type( stateid ) );

    // Step (3): Now handle the common part for each add_state function
    return ( add_statebase( StateBaseHandle( state ) ) );
  }

  // ADD_STATE:
  // Add a local state variable with option list

  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::string& option_list )
  {   
    // Step (1): Generate a new unique ID for this state
    std::string stateid = stateid_prefix_ + std::string( "::" ) + key;
  
    // Step (2): Generate the state variable
    state = HANDLE( new typename HANDLE::element_type( stateid, default_option, option_list ) );

    // Step (3): Now handle the common part for each add_state function
    return ( add_statebase( StateBaseHandle( state ) ) );
  }

  // ADD_STATE:
  // Add a local state variable with option list

  template< class HANDLE >
  bool add_state( const std::string& key, HANDLE& state, const std::string& default_option,
      const std::vector< std::string > option_list )
  {
    // Step (1): Generate a new unique ID for this state
    std::string stateid = stateid_prefix_ + std::string( "::" ) + key;
      
    // Step (2): Generate the state variable
    state = HANDLE( new typename HANDLE::element_type( stateid, default_option, option_list ) );

    // Step (3): Now handle the common part for each add_state function
    return ( add_statebase( StateBaseHandle( state ) ) );
  }

  // ADD_STATE:
  // Add a local state variable without default value
  
  template< class HANDLE, class T >
  bool add_state( const std::string& key, HANDLE& state, const std::vector< T >& default_value )
  {
    // Step (1): Generate a new unique ID for this state
    std::string stateid = stateid_prefix_ + std::string( "::" ) + key;
    
    // Step (2): Generate the state variable
    state = HANDLE( new typename HANDLE::element_type( stateid, default_value ) );

    // Step (3): Now handle the common part for each add_state function
    return ( add_statebase( StateBaseHandle( state ) ) );

  }

protected:
  // STATE_CHANGED:
  // This function is called when any of the state variables are changed
  
  virtual void state_changed()
  {
    // default function is to do nothing
  }

  const std::string& stateid() const
  {
    return ( stateid_prefix_ );
  }
  

private:
  // HANDLE_STATE_CHANGED:
  // This function is called whenever a state registered with this statehandler is changed
  void handle_state_changed();

private:

  friend class StateEngine;

  // Function that adds the state variable to the database
  bool add_statebase( StateBaseHandle state );

  bool get_state( const std::string& state_id, StateBaseHandle& state );

  // Prefix for all state variables of this class
  std::string stateid_prefix_;

  StateHandlerPrivate* private_;
};

} // end namespace Core

#endif
