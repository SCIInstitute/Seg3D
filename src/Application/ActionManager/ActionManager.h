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

#ifndef APPLICATION_ACTIONMANAGER_ACTIONMANAGER_H
#define APPLICATION_ACTIONMANAGER_ACTIONMANAGER_H

// Boost includes for singleton
#include <boost/thread/mutex.hpp>
#include <boost/utility.hpp>

namespace Seg3D {

// Forward declarations
class ActionManagerDispatcher;
class ActionManagerState;
class ActionManagerHandler;
class ActionManager;

class ActionManager : public boost::noncopyable {

// -- Constructor
  public:
    ActionManager();


// -- Dispatcher/State/Handler interface
  public:

    ActionManagerDispatcher* dispatcher() const { return dispatcher_; }
    ActionManagerState*      state() const      { return state_; }
    ActionManagerHandler*    handler() const    { return handler_; }

  private:
    ActionManagerDispatcher* dispatcher_;
    ActionManagerState*      state_;
    ActionManagerHandler*    handler_;

// -- Singleton interface --
  public:
  
    // INSTANCE:
    // Get the singleton pointer to the application
    
    static ActionManager* instance();

  private:
  
    // Mutex protecting the singleton interface
    static boost::mutex     instance_mutex_;
    // Initialized or not?
    static bool             initialized_;
    // Pointer that contains the singleton interface to this class
    static ActionManager*   instance_;
};

} // namespace Seg3D

// Include these here to prevent circular includes

#include <Application/ActionManager/ActionManagerDispatcher.h>
#include <Application/ActionManager/ActionManagerState.h>
#include <Application/ActionManager/ActionManagerHandler.h>

#endif
