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

#ifndef CORE_EVENTHANDLER_DEFAULTEVENTHANDLERCONTEXT_H
#define CORE_EVENTHANDLER_DEFAULTEVENTHANDLERCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>

// Include Base class
#include <Core/EventHandler/EventHandlerContext.h>

namespace Core
{

// Forward Declaration
class DefaultEventHandlerContext;

// Internals of the default event handler
class DefaultEventHandlerContextPrivate;
typedef boost::shared_ptr<DefaultEventHandlerContextPrivate> 
  DefaultEventHandlerContextPrivateHandle;

class DefaultEventHandlerContext : public EventHandlerContext
{

public:

  // Constuctor and destructor
  DefaultEventHandlerContext();
  virtual ~DefaultEventHandlerContext();

  // POST_EVENT:
  /// Post an event onto the event handler stack. This one
  /// returns immediately after posting the event, and does
  /// not wait for the process to finish the event.

  virtual void post_event( EventHandle& event );

  // POST_AND_WAIT_APPLICATION_EVENT:
  /// Post an event onto the event handler stack. This one
  /// returns after the thread signals that the event has been
  /// executed. The function does the full hand shaking for
  /// the synchronization.

  virtual void post_and_wait_event( EventHandle& event );

  // PROCESS_EVENTS:
  /// process the events that are queued in the event handler mailbox

  virtual bool process_events();

  // WAIT_AND_PROCESS_EVENTS:
  /// process the events that are queued in the event handler mailbox

  virtual bool wait_and_process_events();

  // IS_EVENTHANDLER_THREAD:
  /// Check whether we are running on the thread that handles the events
  /// This function is needed to avoid to post and execute things in an infinite
  /// loop.

  virtual bool is_eventhandler_thread() const;

  // START_EVENTHANDLER:
  /// Start the eventhandler thread and start processing events
  virtual bool start_eventhandler( EventHandler* eventhandler );

  // EVENTHANDLER_STARTED:
  /// Check whether the eventhandler is running
  virtual bool eventhandler_started();

  // TERMINATE_EVENTHANDLER:
  /// Terminate the eventhandler
  virtual void terminate_eventhandler();

  // -- internals of this class --
private:

  // Handle to the internals of this class
  DefaultEventHandlerContextPrivateHandle private_;
};

} // end namespace Core

#endif
