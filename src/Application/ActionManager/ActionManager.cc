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


#include <Application/ActionManager/ActionManager.h>

#include <boost/signals2.hpp>

namespace Seg3D {

ActionManager::ActionManager() :
  dispatcher_(new ActionManagerDispatcher),
  state_(new ActionManagerState),
  handler_(new ActionManagerHandler)
{
  // Connect the dispatcher to the handler
  // This is the main connection that separates the input from the output part
  // of the application. 
  
  // This one needs to be disconnected to reroute all the action handling through
  // a different class. Hence at some point this connection should go to the 
  // handler.
  
  dispatcher_->post_action_signal_.connect(
    boost::bind(&ActionManagerHandler::handle_action_slot,handler_,_1));
}

ActionManager*
ActionManager::instance()
{
  // if no singleton was allocated, allocate it now
  if (!initialized_)   
  {
    //in case multiple threads try to allocate this one at once.
    {
      boost::unique_lock<boost::mutex> lock(instance_mutex_);
      // The first test was not locked and hence not thread safe
      // This one will do a thread-safe allocation of the interface
      // class
      if (instance_ == 0) instance_ = new ActionManager();
    }
    
    {
      // Enforce memory synchronization so the singleton is initialized
      // before we set initialized to true
      boost::unique_lock<boost::mutex> lock(instance_mutex_);
      initialized_ = true;
    }
  }
  return (instance_);
}

}
