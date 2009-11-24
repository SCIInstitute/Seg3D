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

#ifndef APPLICATION_STATE_STATEMANAGER_H
#define APPLICATION_STATE_STATEMANAGER_H

// STL includes
#include <map>

// boost includes
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>
#include <Utils/Singleton/Singleton.h>

#include <Application/State/StateBase.h>

namespace Seg3D {

class StateManager;

class StateManager : public boost::noncopyable {

// -- Constructor/destructor --
  public:
    StateManager();
    
// -- Interface for accessing state variables --
  public:
  
    // ADD_STATE:
    // Add the base of a state variable to the central state database in
    // StateManager. 
    bool add_state(const std::string& stateid, StateBaseHandle& state);
  
    // GET_STATE:
    // Get pointer to the state variable based on the unique state tag
    bool get_state(const std::string& stateid, StateBaseHandle& state);
    
    // REMOVE_STATE:
    // Remove all the state variables that derive from the tag and the tag 
    // itself. So remove ToolManager::PaintTool will remove as well derived
    // values like ToolManager::PaintTool::Brushsize.
    void remove_state(const std::string& stateid);
    
  private:
    typedef boost::unordered_map<std::string,StateBaseHandle> state_map_type;

    // Map containing pointers to the State variables in the class under control
    // by the StateManager
    state_map_type state_map_;
    
    // Mutex protecting the pointer map
    boost::mutex state_map_lock_;

// -- Singleton interface --
  public:
    static StateManager* instance() { instance_.instance(); }

  private:
    static Utils::Singleton<StateManager> instance_;
};

} // end namespace Seg3D

#endif
