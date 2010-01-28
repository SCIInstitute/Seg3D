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

#ifndef APPLICATION_STATE_STATEENGINE_H
#define APPLICATION_STATE_STATEENGINE_H

// STL includes
#include <map>

// boost includes
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>
#include <Utils/Singleton/Singleton.h>

#include <Application/State/StateBase.h>

namespace Seg3D {

class StateEngine;

class StateEngine : public boost::noncopyable {

// -- Constructor/destructor --
  public:
    StateEngine();
    virtual ~StateEngine();
    
// -- Interface for accessing state variables --
  public:
  
    // ADD_STATE:
    // Add the base of a state variable to the central state Engine in
    // StateEngine. 
    bool add_state(const std::string& stateid, StateBaseHandle& state);
  
    // GET_STATE:
    // Get pointer to the state variable based on the unique state tag
    bool get_state(const std::string& stateid, StateBaseHandle& state);
    
    // REMOVE_STATE:
    // Remove all the state variables that derive from the tag and the tag 
    // itself. So remove ToolManager::PaintTool will remove as well derived
    // values like ToolManager::PaintTool::Brushsize.
    void remove_state(const std::string& stateid);
    
// -- state engine locking interface --
  public:
    // Mutex protecting the StateEngine
    typedef boost::recursive_mutex          mutex_type;
    typedef boost::unique_lock<mutex_type>  lock_type;

    // GET_MUTEX:
    // Get the mutex that controls whether changes can be made to the
    // state engine.
    inline mutex_type& get_mutex() { return state_engine_lock_; }
    
    // LOCK:
    // Lock the mutex that controls whether changes can be made to the
    // state engine
    inline void lock() { state_engine_lock_.lock(); }

    // UNLOCK:
    // Lock the mutex that controls whether changes can be made to the
    // state engine
    inline void unlock() { state_engine_lock_.unlock(); }
    
  private:
    // Lock that controls whether changes can be made to the state engine
    mutex_type state_engine_lock_;

// -- state database --
  private:
    typedef boost::unordered_map<std::string,StateBaseHandle> state_map_type;

    // Map containing pointers to the State variables in the class under control
    // by the StateEngine
    state_map_type state_map_;
    
// -- Singleton interface --
  public:
    static StateEngine* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<StateEngine> instance_;
    
// -- Static convenience functions --

  public:
    // LOCK
    // Prohibit changes to be made in the state of the program
    static void Lock() { Instance()->lock(); }
    
    // UNLOCK
    // Allow changes to be made in the state of the program
    static void Unlock() { Instance()->unlock(); }
};

} // end namespace Seg3D

#endif
