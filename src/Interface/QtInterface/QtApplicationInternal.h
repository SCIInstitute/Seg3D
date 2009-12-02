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

#ifndef INTERFACE_QTINTERFACE_QTAPPLICATIONINTERNAL_H
#define INTERFACE_QTINTERFACE_QTAPPLICATIONINTERNAL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Qt includes
#include <QtGui>

// Includes from application layer
#include <Utils/EventHandler/EventHandlerContext.h>
#include <Utils/EventHandler/EventHandler.h>
#include <Utils/EventHandler/Event.h>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace Seg3D {

// QTAPPLICATIONINTERNAL:

// As QT takes ownership of events, these classes wraps around our
// functor call back classes and hide all the QT handling behind
// a cleaner interface.

// CLASS QtUserQEvent:
// The user event class that is send to QT, it wraps around the 
// ApplicationEvent class, and thence only needs a handle to that
// class.

class QtUserEvent : public QEvent {

  public:
    // constructor for a wrapper around the ApplicationEvent class
    QtUserEvent(Utils::EventHandle& event_handle);
 
    virtual ~QtUserEvent();

    // As QT takes on ownership of the QEvent, we use smart
    // pointers to share resources between the different threads
    // As long as one of the threads has a handle the object will
    // persist.
    Utils::EventHandle  event_handle_;
};

// CLASS QtEventFilter:
// This class is the filter that needs to be installed into the main QT
// event handler. 

class QtEventFilter : public QObject {
    Q_OBJECT
  
  public:
    QtEventFilter(QObject* parent = 0) :
      QObject(parent) {}
      
  protected:
    bool eventFilter(QObject* obj, QEvent* event);
};


// CLASS QtInterfaceContext:
// This class wraps the functionality that the Application class needs in an
// object, so there is a clean interface between Interface and Application
// layer.

class QtEventHandlerContext : public Utils::EventHandlerContext {

 public:
    
    // Constuctor and destructor
    QtEventHandlerContext(QApplication* qt_application);
    virtual ~QtEventHandlerContext();
  
    // POST_EVENT:
    // Post an event onto the event handler stack. This one
    // returns immediately after posting the event, and does 
    // not wait for the process to finish the event.            
    virtual void post_event(Utils::EventHandle& event);

    // POST_AND_WAIT_EVENT:
    // Post an event onto the event handler stack. This one
    // returns after the thread signals that the event has been
    // executed. The function does the full hand shaking for 
    // the synchronization.
    virtual void post_and_wait_event(Utils::EventHandle& event);

    // PROCESS_EVENT:
    // process the events that are queued in the event handler stack.
    virtual bool process_events();

    // WAIT_AND_PROCESS_EVENTS:
    // process the events that are queued in the event handler mailbox.   
    virtual bool wait_and_process_events();
        
    // IS_EVENTHANDLER_THREAD:
    // Check whether we are running on the thread that handles the events
    // This function is needed to avoid to post and execute things in an 
    // infinite loop.
    virtual bool is_eventhandler_thread() const;
    
    // START_EVENTHANDLER:
    // Start the eventhandler thread and start processing events
    virtual bool start_eventhandler(Utils::EventHandler* eventhandler);

    // TERMINATE_EVENTHANDLER:
    // Terminate the eventhandler
    virtual void terminate_eventhandler();
            
  private:
    
    // A pointer to the main Qt application class
    QApplication*           qapplication_;
    
    // Thread id from the thead that is running Qt
    boost::thread::id       interface_thread_id_;
};

} // end namespace Seg3D

#endif
