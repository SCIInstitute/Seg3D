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

#ifndef UTILS_EVENTHANDLER_EVENTHANDLER_H
#define UTILS_EVENTHANDLER_EVENTHANDLER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>

// For event handling
#include <Utils/EventHandler/Event.h>
#include <Utils/EventHandler/EventHandlerContext.h>

namespace Utils {


class EventHandler;
typedef boost::shared_ptr<EventHandler> EventHandlerHandle;

class EventHandler : public boost::noncopyable  {

// -- Constructor/Destructor --
  public:
    EventHandler();

    virtual ~EventHandler();

// -- Event handling --
  public:
    
    bool is_eventhandler_thread() const
    { 
      // use the private context class to answer this question
      return (eventhandler_context_->is_eventhandler_thread());
    }
    
    // POST_EVENT:
    // Post an event on the application event stack. If the program uses a GUI
    // these events are mixed in with the main event handler and are execute at
    // the same time.   
    
    void post_event(boost::function<void ()> function);
    
    // POST_AND_WAIT_EVENT:
    // This function is similar to post_event, but waits until the function 
    // has finished execution. Note in case the function is called from the
    // application thread itself it will immediately execute and not post 
    // any events on the event stack in order for the application not to
    // dead lock
    
    void post_and_wait_event(boost::function<void ()> function);

    // POST_EVENT_AND_GET_RESULT:
    // This function is similar to post_event, but waits until the function 
    // has finished execution and it also grabs the output.
        
    template<class FUNCTION>
    void post_event_and_get_result(FUNCTION function, 
                                   typename FUNCTION::result_type& result)
    {
      if (is_eventhandler_thread())
      {
        result = function();
      }
      else
      {
        EventHandle event = EventHandle(new EventRT<FUNCTION>(function));
        eventhandler_context_->post_and_wait_event(event);
        result = dynamic_cast<EventRT<FUNCTION> *>(event.get())->get_result();
      }
    }

// -- Processing events from within the event handler thread --
  protected:
    
    // PROCESS_EVENTS:
    // Process the events that are in the event handler queue.

    bool process_events()
    {
      // use the implementation of the application context
      return (eventhandler_context_->process_events());
    }

    // WAIT_AND_PROCESS_EVENTS:
    // Wait for events to come in and process the events

    bool wait_and_process_events()
    {
      // use the implementation of the application context
      return (eventhandler_context_->wait_and_process_events());
    }

// -- Thread interface --
  public:
  
    // START_EVENTHANDLER:
    // Start the eventhandler. This will launch the eventhandler
    // thread and runs the run_eventhandler() function.
  
    bool start_eventhandler()
    {
      return (eventhandler_context_->start_eventhandler(this));
    }
  
    // RUN_EVENTHANDLER:
    // The main functions that processes all the incoming events.
    virtual void run_eventhandler(); 
    
  private:
  
    // TERMINATE_EVENTHANDLER:
    // Terminate processing events and kill the thread that processes them.
    // This function is private and can only be called from
    // TerminateEventHandler(). This function launches a separate thread
    // that joins with the eventhandler thread, to allow for the remainder
    // events to be processed before the object is destructed.
    friend void TerminateEventHandlerThread(EventHandlerHandle handle);
    
    void terminate_eventhandler()
    {
      eventhandler_context_->terminate_eventhandler();
    }
    
// -- Context interface --
  public:
 
    // INSTALL_EVENTHANDLER_CONTEXT()
    // Setup the application context. The application context abstracts the 
    // interface piece of the GUI layer that is needed for communication with
    // the application thread.

    void install_eventhandler_context(EventHandlerContextHandle& context);
    
  private:  

    // EVENTHANDLERCONTEXT:
    // This is the internal representation of the handler.
    // This one can be overloaded, which is for instance needed
    // for the Qt thread.
    
    EventHandlerContextHandle eventhandler_context_; 
    
};

// This function terminates the object that uses the eventhandler
// We need an additional thread to terminate the eventhandler thread
// safely.
void TerminateEventHandler(EventHandlerHandle& eventhandlerhandle);

} // end namespace Utils

#endif
