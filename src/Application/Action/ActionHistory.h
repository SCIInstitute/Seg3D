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

#ifndef APPLICATION_ACTION_ACTIONHISTORY_H
#define APPLICATION_ACTION_ACTIONHISTORY_H

// STL includes
#include <iostream>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/signals2/signal.hpp>

#include <Application/Action/Action.h>
#include <Application/Action/ActionDispatcher.h>

namespace Seg3D {

// CLASS ACTIONHISTORY
// A record of the last actions issued in the program

// Forward Declaration
class ActionHistory;

// Class defintion
class ActionHistory : public boost::noncopyable {

// -- Constructor/Destructor --
  private:
    friend class Utils::Singleton<ActionHistory>;
    ActionHistory();
  
  public:
    virtual ~ActionHistory();
  
// -- History recording --

  public:
    // SET_MAX_HISTORY_SIZE:
    // Set the size of the action history buffer
    void set_max_history_size(size_t size);
    
    // MAX_HISTORY_SIZE:
    // Retrieve the size of this buffer
    size_t max_history_size();
    
    // HISTORY_SIZE:
    size_t history_size();
    
    // ACTION
    // Get the nth action in the buffer
    ActionHandle action(size_t index);

    // ACTIONRESULT
    // Get the nth action result in the buffer
    ActionResultHandle result(size_t index);
        
  private:
    typedef std::deque<std::pair<ActionHandle,ActionResultHandle> > action_history_type;

    boost::mutex        action_history_mutex_;
    action_history_type action_history_;
    size_t              action_history_max_size_;

    boost::signals2::connection dispatcher_connection_;

    void record_action(ActionHandle handle, ActionResultHandle result);

// -- History changed signal --
  public:
    typedef boost::signals2::signal<void ()> history_changed_signal_type;
    
    // HISTORY_CHANGED_SIGNAL:
    // Signal indicating that the history changed
    history_changed_signal_type history_changed_signal_;

// -- Singleton interface --
  public:
    
    static ActionHistory* Instance() { return instance_.instance(); } // << SINGLETON
    
  private:
    // Singleton internals
    static Utils::Singleton<ActionHistory> instance_;
};

} // end namespace Seg3D

#endif