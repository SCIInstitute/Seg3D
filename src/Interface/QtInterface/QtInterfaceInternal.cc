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

#include <Utils/Core/Exception.h>

#include <Interface/QtInterface/QtInterfaceInternal.h>

namespace Seg3D {

// CLASS QtUserEvent:

QtUserEvent::QtUserEvent(Utils::EventHandle& event_handle) : 
  QEvent(QEvent::User),
  event_handle_(event_handle)
{
}

QtUserEvent::~QtUserEvent()
{
}


// CLASS QtEventFilter:

bool
QtEventFilter::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::User)
  {
    // Cast to the right type
    QtUserEvent *user_event = 
      dynamic_cast<QtUserEvent *>(event);
 
    // Run the code that was send to us
    user_event->event_handle_->handle_event();
    
    return (false);
  }
  else
  {
    return (QObject::eventFilter(obj,event));
  }
}

// CLASS QtInterfaceContext:
// This class needs to be constructed from the main Qt thread

QtEventHandlerContext::QtEventHandlerContext(QApplication* qapplication) :
  qapplication_(qapplication),
  interface_thread_id_(boost::this_thread::get_id())
{
}

QtEventHandlerContext::~QtEventHandlerContext()
{
}

void
QtEventHandlerContext::post_event(Utils::EventHandle& event)
{
  // Generate a new event message for QT
  // This is an ordinary pointer as QT will
  // take ownership of it and delete it.
  
  QtUserEvent* qevent = new QtUserEvent(event);

  // Insert call into the QT event loop
  QApplication::postEvent(qapplication_,qevent);
}


void
QtEventHandlerContext::post_and_wait_event(Utils::EventHandle& event)
{
  // Need a synchronization class to confirm the event is done
  Utils::EventSyncHandle sync = Utils::EventSyncHandle(new Utils::EventSync);
  
  // Add event to event class so the responder is doing the other part
  // of the synchronization
  event->sync_handle() = sync;
  
  // Generate a new event message for QT:
  // This is an ordinary pointer as QT will take ownership of it and delete it.
  QtUserEvent* qevent = new QtUserEvent(event);
  
  // Handshaking for communication with QT
  boost::unique_lock<boost::mutex> lock(sync->lock_);

  // Insert call into the QT event loop
  QApplication::postEvent(qapplication_,qevent);
  
  // Wait for QT to handle the event
  sync->condition_.wait(lock);
}

bool
QtEventHandlerContext::process_events()
{
  QApplication::processEvents();
  return (false);
}

bool
QtEventHandlerContext::wait_and_process_events()
{
  SCI_THROW_LOGICERROR("Cannot wait on the Qt thread");
}

bool
QtEventHandlerContext::start_eventhandler(Utils::EventHandler* eventhandler)
{
  // Allow Qt to intercept the actions in its main 
  QtEventFilter* qt_event_filter = new QtEventFilter(qapplication_); 
  qapplication_->installEventFilter(qt_event_filter);

  return (true);
}

void
QtEventHandlerContext::terminate_eventhandler()
{
  SCI_THROW_LOGICERROR("Cannot terminate the Qt thread");
}

bool
QtEventHandlerContext::is_eventhandler_thread() const
{
  return (boost::this_thread::get_id() == interface_thread_id_); 
}

} // end namespace Seg3D

