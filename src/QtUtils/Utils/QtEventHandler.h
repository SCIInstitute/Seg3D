/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef QTUTILS_UTILS_QTEVENTHANDLER_H
#define QTUTILS_UTILS_QTEVENTHANDLER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#ifndef Q_MOC_RUN

// STL includes
#include <queue>

// Qt includes
#include <QApplication>
#include <QEvent>
#include <QObject>

// Includes from application layer
#include <Core/EventHandler/EventHandlerContext.h>
#include <Core/EventHandler/EventHandler.h>
#include <Core/EventHandler/Event.h>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#endif

namespace QtUtils
{

class QtEventHandlerContext;
class QtEventHandlerObject;

/// QTEVENTHANDLER:

/// As QT takes ownership of events, these classes wraps around our
/// functor call back classes and hide all the QT handling behind
/// a cleaner interface.

/// CLASS QtEventHandlerObject:
/// This is a helper class for QtEventHandlerContext. It installs an event filter and a timer
/// to the main Qt thread, which will trigger QtEventHandlerContext::process_events in
/// the Qt thread.

class QtEventHandlerObject : public QObject
{
Q_OBJECT

public:
  QtEventHandlerObject( QApplication* parent, QtEventHandlerContext* context );
  virtual bool eventFilter ( QObject* watched, QEvent* event );

protected:
  virtual void timerEvent ( QTimerEvent* event );

private Q_SLOTS:
  void cleanup();

private:
  QApplication* parent_;
  QtEventHandlerContext* event_handler_context_;
  int timer_id_;
};

/// CLASS QtInterfaceContext:
/// This class wraps the functionality that the Application class needs in an
/// object, so there is a clean interface between Interface and Application
/// layer.

class QtEventHandlerContext : public Core::EventHandlerContext
{

public:

  // Constructor and destructor
  QtEventHandlerContext( QApplication* qt_application );
  virtual ~QtEventHandlerContext();

  /// POST_EVENT:
  /// Post an event onto the event handler stack. This one
  /// returns immediately after posting the event, and does
  /// not wait for the process to finish the event.
  virtual void post_event( Core::EventHandle& event );

  /// POST_AND_WAIT_EVENT:
  /// Post an event onto the event handler stack. This one
  /// returns after the thread signals that the event has been
  /// executed. The function does the full hand shaking for
  /// the synchronization.
  virtual void post_and_wait_event( Core::EventHandle& event );

  /// PROCESS_EVENT:
  /// process the events that are queued in the event handler stack.
  virtual bool process_events();

  /// WAIT_AND_PROCESS_EVENTS:
  /// process the events that are queued in the event handler mailbox.
  virtual bool wait_and_process_events();

  /// IS_EVENTHANDLER_THREAD:
  /// Check whether we are running on the thread that handles the events
  /// This function is needed to avoid to post and execute things in an
  /// infinite loop.
  virtual bool is_eventhandler_thread() const;

  /// START_EVENTHANDLER:
  /// Start the eventhandler thread and start processing events
  virtual bool start_eventhandler( Core::EventHandler* eventhandler );
  
  /// EVENTHANDLER_STARTED:
  /// Check whether the eventhandler is running
  virtual bool eventhandler_started();

  /// TERMINATE_EVENTHANDLER:
  /// Terminate the eventhandler
  virtual void terminate_eventhandler();

  /// EMPTY_EVENT_QUEUE:
  /// Purge all the events in the queue.
  void empty_event_queue();

private:
  /// Keep track of when the event filter is installed
  bool eventhandler_started_;

  bool eventhandler_stopped_;

  /// A pointer to the main Qt application class
  QApplication* qapplication_;

  /// Thread id from the thread that is running Qt
  boost::thread::id interface_thread_id_;

  /// Event queue
  std::queue< Core::EventHandle > events_;

  /// Mutex for protecting the event queue
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;
  mutex_type mutex_;
};

} // end namespace QtUtils

#endif
