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

#ifndef APPLICATION_APPLICATION_APPLICATIONCONTEXT_H
#define APPLICATION_APPLICATION_APPLICATIONCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <Application/Application/ApplicationEvent.h>

namespace Seg3D {

// CLASS APPLICATIONCONTEXT:
// This class isolates the calls that need to be done into the GUI system in
// terms of communication. Since the application is multi threaded, we need
// to push events from asynchronous threads back onto the main thread. As GUI systems
// tend to be event driven, these asynchronous events need to be tunneled 
// through the main GUI interface system, if the UI and the main application
// share a single thread.

class ApplicationContext {
  
  public:
    
    // Constuctor and destructor for class that can be overloaded
    ApplicationContext() {}
    virtual ~ApplicationContext() {}
  
    // OBTAIN:
    // This function is called when the context is installed

    virtual void obtain() = 0;

    // RELEASE:
    // This function is called when the application is destroyed or
    // another context is installed
    
    virtual void release() = 0;
  
    // POST_APPLICATION_EVENT:
    // Post an application event onto the application event stack. This one
    // returns immediately after posting the event, and does not wait for the
    // process to finish the event.

    virtual void post_application_event(ApplicationEventHandle& event) = 0; 

    // POST_AND_WAIT_APPLICATION_EVENT:
    // Post an application event onto the application event stack. This one
    // returns after the application thread signals that the event has been
    // executed. The function does the full hand shaking for the synchronization.

    virtual void post_and_wait_application_event(ApplicationEventHandle& event) = 0;

    // PROCESS_EVENT:
    // process the events that are queued in the application mailbox
    virtual void process_events() = 0;

    // IS_APPLICATION_THREAD:
    // Check whether we are running on the therad that handles the events
    // This function is needed to avoid to post and execute things in an infinite
    // loop.
    virtual bool is_application_thread() const = 0;
};

typedef boost::shared_ptr<ApplicationContext> ApplicationContextHandle;

} // end namespace Seg3D

#endif
