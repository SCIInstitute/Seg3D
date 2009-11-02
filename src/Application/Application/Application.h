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

#ifndef APPLICATION_APPLICATION_APPLICATION_H
#define APPLICATION_APPLICATION_APPLICATION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Core includes
#include <Core/Utils/Log.h>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// For action handling
#include <Application/Application/Action.h>

// For event handling
#include <Application/Application/ApplicationEvent.h>
#include <Application/Application/ApplicationContext.h>


namespace Seg3D {

// CLASS APPLICATION:
// This is the main class for handling all the events and actions in the program
// -- Events --
// Events are the asynchronous events that take place in the program and use an
// event queue that is processed asynchronously from the event issuer
 

class Application : public boost::noncopyable  {

// -- Constructor/Destructor --
  public:
    Application();
  
// -- Action handling --
  public:
    // TYPEDEFS 
    // The type of the main action signal

    typedef boost::signals2::signal<void (ActionHandle)> post_action_signal_type;

    // POST_ACTION:
    // Post an action into the main signal stack of the application
    // All actions in the program are funneled through this signal stack
    // Allow queue determines whether actions that have to be post poned
    // because they cannot be executed at the time of issuing are allowed
    // to pass through the action post_signal. Generally interface posts
    // will set this flag to false, so issuing a filter before a layer is
    // ready will nullify the action, as it cannot be executed at that time
    // Script playbacks will allow queueing and hence they will need to have
    // allow_queue = true.

    bool post_action(ActionHandle action, bool allow_queue = false);        

    // POST_ACTION_SIGNAL:
    // This is the main signal stack for actions that are posted inside the
    // application. Any observer that wants to listen into the actions that
    // are being issued by th program needs to connect to this signal as all
    // GUI events, Application events, and Layer Data events are passed through
    // this single application signal.
    
    post_action_signal_type post_action_signal_;
    
    // DISPATCH_ACTION_SLOT:
    // This is the main action dispatcher. This function runs the actions and
    // cleans up the actions when they are done. It also invokes the provenance
    // recording.
    
    void dispatch_action_slot(ActionHandle action);

// -- Event handling --
  public:

    // IS_APPLICATION_THREAD:
    // This function checks whether the current thread is the application thread
    // This is important for slots and functions that can receive input from
    // asynchronous processes as the main signal/slot operations should only be
    // executed on the main application thread. If the code is not on the main
    // application thread, one should consider posting functor with the code that
    // needs to be executed through the application event handling mechanism.
    
    bool is_application_thread() const
    { 
      // use the private context class to answer this question
      return (application_context_->is_application_thread());
    }
    
    // POST_EVENT:
    // Post an event on the application event stack. If the program uses a GUI
    // these events are mixed in with the main event handler and are execute at
    // the same time. The option direct_evaluation_when_on_application_thread
    // controls whether the event can be executed immediately if we are already
    // on the application thread. This is the default behavior  
    
    template<class FUNCTOR>
    void post_event(FUNCTOR functor, bool direct_evaluation_when_on_application_thread = true)
    {
      if (is_application_thread() && direct_evaluation_when_on_application_thread) 
      {
        functor.operator();
      }
      else
      {
        ApplicationEventHandle event = 
          ApplicationEventHandle(new ApplicationEventT<FUNCTOR>(functor));
        application_context_->post_application_event(event);
      }
    }
    
    // POST_AND_WAIT_EVENT:
    // This function is similar to post_event, but waits until the function 
    // has finished execution. Note in case the function is called from the
    // application thread itself it will immediately execute and not post 
    // any events on the event stack in order for the application not to
    // dead lock
    
    template<class FUNCTOR>
    void post_and_wait_event(FUNCTOR functor)
    {
      if (is_application_thread())
      {
        functor.operator();
      }
      else
      {
        ApplicationEventHandle event = 
          ApplicationEventHandle(new ApplicationEventT<FUNCTOR>(functor));
        application_context_->post_and_wait_application_event(event);
      }
    }

    // POST_EVENT_AND_GET_RESULT:
    // This function is similar to post_event, but waits until the function 
    // has finished execution and it also grabs the output.
        
    template<class FUNCTOR>
    typename FUNCTOR::result_type
    post_event_and_get_result(FUNCTOR functor)
    {
      if (is_application_thread())
      {
        return (functor());
      }
      else
      {
        ApplicationEventHandle event = 
          ApplicationEventHandle(new ApplicationEventRT<FUNCTOR>(functor));
        application_context_->post_and_wait_application_event(event);
        return (dynamic_cast<ApplicationEventRT<FUNCTOR> *>(event.get())->get_result());
      }
    }
    
    // PROCESS_EVENTS:
    // Process the events that are in the applications event queue.
    // This function is initended for the case when there is no GUI driving the
    // program.

    void process_events();
    
// -- Context interface --
  public:
 
    // INSTALL_CONTEXT()
    // Setup the application context. The application context abstracts the 
    // interface piece of the GUI layer that is needed for communication with
    // the application thread.

    void install_context(ApplicationContextHandle& context);
    
  private:  

    // APPLICATIONCONTEXT
    // The application context handles GUI specific function calls that need to
    // be launched from the application layer. A context is installed into the 
    // application to deal with these issues.
    // The context deals with:
    // - merging event stacks of the application and the interface
    // - posting events on the application stack
    
    ApplicationContextHandle application_context_; 

// -- Singleton interface --
  public:
  
    // INSTANCE:
    // Get the singleton pointer to the application
    
    static Application* instance();

  private:
  
    // Mutex protecting the singleton interface
    static boost::mutex   application_mutex_;
    // Initialized or not?
    static bool initialized_;
    // Pointer that contains the singleton interface to this class
    static Application*   application_;
    
};

} // end namespace Seg3D

#endif
