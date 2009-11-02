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

#include <Interface/QtInterface/QtInterfaceInternal.h>

namespace Seg3D {

// CLASS QtInterfaceUserQEvent:

QtInterfaceUserQEvent::QtInterfaceUserQEvent(
                  ApplicationEventHandle& application_event_handle) : 
  QEvent(QEvent::User),
  application_event_handle_(application_event_handle)
{
}

QtInterfaceUserQEvent::~QtInterfaceUserQEvent()
{
}

// CLASS QtInterfaceUserEventFilter:

bool
QtInterfaceUserEventFilter::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::User)
  {
    QtInterfaceUserQEvent *user_event = 
      dynamic_cast<QtInterfaceUserQEvent *>(event);
 
    // Run the code that was send to us
    user_event->application_event_handle_->handle_event();
    
    return (false);
  }
  else
  {
    return (QObject::eventFilter(obj,event));
  }
}

// CLASS QtInterfaceContext:
// This class needs to be constructed from the main Qt thread

QtInterfaceContext::QtInterfaceContext(QApplication* qt_application) :
  qt_application_(qt_application),
  application_thread_id_(boost::this_thread::get_id())
{
}

QtInterfaceContext::~QtInterfaceContext()
{
}


void
QtInterfaceContext::obtain()
{
}

void
QtInterfaceContext::release()
{
}

void
QtInterfaceContext::post_application_event(ApplicationEventHandle& event)
{
  // Generate a new event message for QT
  // This is an ordinary pointer as QT will
  // take ownership of it and delete it.
  
  QtInterfaceUserQEvent* qevent = new QtInterfaceUserQEvent(event);

  // Insert call into the QT event loop
  QApplication::postEvent(qt_application_,qevent);
}


void
QtInterfaceContext::post_and_wait_application_event(ApplicationEventHandle& event)
{
  // Need a synchronization class to confirm the event is done
  ApplicationEventSyncHandle sync = 
    ApplicationEventSyncHandle(new ApplicationEventSync);
  
  // Add event to event class so the responder is doing the other part
  // of the synchronization
  event->sync_handle() = sync;
  
  // Generate a new event message for QT:
  // This is an ordinary pointer as QT will take ownership of it and delete it.
  QtInterfaceUserQEvent* qevent = new QtInterfaceUserQEvent(event);
  
  // Handshaking for communication with QT
  boost::unique_lock<boost::mutex> lock(sync->lock_);

  // Insert call into the QT event loop
  QApplication::postEvent(qt_application_,qevent);
  
  // Wait for QT to handle the event
  sync->condition_.wait(lock);
}


void
QtInterfaceContext::process_events()
{
  QApplication::processEvents();
}

bool
QtInterfaceContext::is_application_thread() const
{
  return (boost::this_thread::get_id() == application_thread_id_); 
}

} // end namespace Seg3D

