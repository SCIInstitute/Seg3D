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

#ifndef INTERFACE_QTINTERFACE_QTINTERFACEINTERNAL_H
#define INTERFACE_QTINTERFACE_QTINTERFACEINTERNAL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Qt includes
#include <QEvent>
#include <QObject>
#include <QApplication>

// Includes from application layer
#include <Application/Application/ApplicationEvent.h>
#include <Application/Application/ApplicationContext.h>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>


namespace Seg3D {

// INTERFACEINTERNAL:

// As QT takes ownership of events, these classes wraps around our
// functor call back classes and hide all the QT handling behind
// a cleaner interface.

// CLASS QtInterfaceUserQEvent:
// The user event class that is send to QT, it wraps around the 
// ApplicationEvent class, and thence only needs a handle to that
// class.

class QtInterfaceUserQEvent : public QEvent {

  public:
    // constructor for a wrapper around the ApplicationEvent class
    QtInterfaceUserQEvent(ApplicationEventHandle& application_event_handle);
 
    virtual ~QtInterfaceUserQEvent();

    // As QT takes on ownership of the QEvent, we use smart
    // pointers to share resources between the different threads
    // As long as one of the threads has a handle the object will
    // persist.
    ApplicationEventHandle  application_event_handle_;
};

// CLASS QtInterfaceUserEventFilter:
// This class is the filter that needs to be installed into the main QT
// event handler. 

class QtInterfaceUserEventFilter : public QObject {
    Q_OBJECT
  
  public:
    QtInterfaceUserEventFilter(QObject* parent = 0) :
      QObject(parent) {}
      
  protected:
    bool eventFilter(QObject* obj, QEvent* event);
};


// CLASS QtInterfaceContext:
// This class wraps the functionality that the Application class needs in an
// object, so there is a clean interface between Interface and Application
// layer.

class QtInterfaceContext : public ApplicationContext {

 public:
    
    // Constuctor and destructor
    QtInterfaceContext(QApplication* qt_application);
    virtual ~QtInterfaceContext();

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
    
    // A pointer to the main Qt application class
    QApplication*           qt_application_;
    
    // Thread id from the thead that is running Qt
    boost::thread::id       application_thread_id_;
    
};

} // end namespace Seg3D

#endif
