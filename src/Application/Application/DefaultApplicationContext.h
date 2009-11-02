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

#ifndef APPLICATION_APPLICATION_DEFAULTAPPLICATIONCONTEXT_H
#define APPLICATION_APPLICATION_DEFAULTAPPLICATIONCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <Application/Application/ApplicationContext.h>

// STL includes
#include <queue>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D {

class DefaultApplicationContext : public ApplicationContext {

  public:
    
    // Constuctor and destructor
    DefaultApplicationContext();
    virtual ~DefaultApplicationContext();

    // OBTAIN:
    // This function is called when the context is installed
    
    virtual void obtain();
    
    // RELEASE:
    // This function is called when the application is destroyed or
    // another context is installed
        
    virtual void release();
  
    // POST_APPLICATION_EVENT:
    // Post an application event onto the application event stack. This one
    // returns immediately after posting the event, and does not wait for the
    // process to finish the event.
            
    virtual void post_application_event(ApplicationEventHandle& event);

    // POST_AND_WAIT_APPLICATION_EVENT:
    // Post an application event onto the application event stack. This one
    // returns after the application thread signals that the event has been
    // executed. The function does the full hand shaking for the synchronization.
    
    virtual void post_and_wait_application_event(ApplicationEventHandle& event);

    // PROCESS_EVENT:
    // process the events that are queued in the application mailbox
    
    virtual void process_events();
    
    // IS_APPLICATION_THREAD:
    // Check whether we are running on the therad that handles the events
    // This function is needed to avoid to post and execute things in an infinite
    // loop.
        
    virtual bool is_application_thread() const;
    
  private:
  
    // Queue type that is used to store the events
    typedef std::queue<ApplicationEventHandle> application_queue_type;
    
    // Application thread id
    boost::thread::id       application_thread_id_;

    // Mutex protecting the queue
    boost::mutex            application_queue_mutex_;    

    // The event queue
    application_queue_type  application_queue_;
};

} // end namespace Seg3D

#endif
