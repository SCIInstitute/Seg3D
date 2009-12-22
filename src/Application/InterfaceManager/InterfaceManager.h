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

#ifndef APPLICATION_INTERFACEMANAGER_INTERFACEMANAGER_H
#define APPLICATION_INTERFACEMANAGER_INTERFACEMANAGER_H

// STL includes
#include <string>
#include <map>
#include <set>

// Boost includes

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>

// Application includes
#include <Application/State/StateHandler.h>

namespace Seg3D {

class InterfaceManager : public StateHandler {

// -- constructor/destructor --
  public:
    InterfaceManager();

// -- Dispatcher functions for User Interface -- 
  public:
    
    // -- Open an interface window --
    void dispatch_show_window(const std::string& windowid) const;
 
    // -- Close an interface window --
    void dispatch_close_window(const std::string& windowid) const;

// -- Signals --
  public:
    typedef boost::signals2::signal<void (std::string)> window_signal_type;
    
    // OPEN_WINDOW_SIGNAL:
    // This signal is triggered after a window needs to be opened
    window_signal_type show_window_signal;
    
    // CLOSE_WINDOW_SIGNAL:
    // This signal is triggered after a window needs to be closed
    window_signal_type close_window_signal;

// -- Access to toollist --
  public:
    typedef std::set<std::string> windowid_list_type;
  
    // ADD_WINDOWID:
    // Mark a windowid as available
    void add_windowid(const std::string& windowid); // << THREAD-SAFE
    
    // IS_WINDOWIS:
    // Check whether window id is valid
    bool is_windowid(const std::string& windowid); // << THREAD-SAFE
  
  private:
    boost::mutex       windowid_list_mutex_;
    windowid_list_type windowid_list_;
    
// -- Singleton interface --
  public:
    static InterfaceManager* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<InterfaceManager> instance_;
};

} // namespace Seg3D

#endif
