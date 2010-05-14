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

#ifndef CORE_STATE_STATEENGINE_H
#define CORE_STATE_STATEENGINE_H

// STL includes
#include <map>
#include <set>

// boost includes
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>

#include <Core/State/StateBase.h>

namespace Core
{

// CLASS STATEENGINE

// Forward decclaration
class StateEngine;

// Class definition
class StateEngine : public Core::RecursiveLockable
{
  CORE_SINGLETON( StateEngine );
  
  // -- Constructor/destructor --
private:
  StateEngine();
  virtual ~StateEngine();

  // -- Interface for accessing state variables --
public:

  // ADD_STATE:
  // Add the base of a state variable to the central state Engine in
  // StateEngine.
  bool add_state( const std::string& stateid, StateBaseHandle& state );

  // GET_STATE:
  // Get pointer to the state variable based on the unique state tag
  bool get_state( const std::string& stateid, StateBaseHandle& state );

  // REMOVE_STATE:
  // Remove all the state variables that derive from the tag and the tag
  // itself. So remove ToolManager::PaintTool will remove as well derived
  // values like ToolManager::PaintTool::Brushsize.
  void remove_state( const std::string& stateid );

  // NUM_STATES:
  // The number of state variables in the system
  size_t num_states();
  
  // GET_STATE:
  // Get the state variable by index
  bool get_state( const size_t idx, StateBaseHandle& state);

  // -- Interface for accounting stateids --
public:
  // ADD_STATEID:
  // Add a new id to the list
  void add_stateid( const std::string& stateid );

  // REMOVE_STATEID:
  // Remove an id from the list
  void remove_stateid( const std::string& stateid );

  // IS_STATEID:
  // Check whether an id exists
  bool is_stateid( const std::string& stateid );

  // CREATE_STATEID:
  // Create a new id based on basename but with an unique extension padded to
  // the end
  std::string create_stateid( std::string basename );

  // -- Signals --
public:
  typedef boost::signals2::signal<void ()> state_changed_signal_type;
  state_changed_signal_type state_changed_signal_;

  // -- state database --
private:
  typedef std::set< std::string > stateid_list_type;
  typedef boost::unordered_map< std::string, StateBaseHandle > state_map_type;
  typedef boost::unordered_map< std::string, std::string > statealias_map_type;
  typedef std::vector< StateBaseHandle > state_list_type;

  // Map containing pointers to the State variables in the class under control
  // by the StateEngine
  state_map_type state_map_;

  // Maintain an organized vector of states as well, so the order is established for
  // writing the states to disk
  state_list_type state_list_;

  // The list of IDs that are in use
  stateid_list_type stateid_list_;

  // -- Static convenience functions --

public:

  // GETMUTEX
  // Get the mutex of the state engine
  static mutex_type& GetMutex()
  {
    return Instance()->get_mutex();
  }
  
  // CREATESTATEID
  // Create an unique stateid from a baseid
  static std::string CreateStateID(const std::string& baseid )
  {
    return Instance()->create_stateid( baseid );
  }

};

} // end namespace Core

#endif
