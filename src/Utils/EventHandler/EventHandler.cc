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

#include <Utils/Core/Log.h>

#include <Utils/EventHandler/EventHandler.h>
#include <Utils/EventHandler/DefaultEventHandlerContext.h>

namespace Utils {

EventHandler::EventHandler() 
{
  // Code for installing a default message handling queue
  eventhandler_context_ = EventHandlerContextHandle(new DefaultEventHandlerContext);
}

EventHandler::~EventHandler()
{
  eventhandler_context_->terminate_eventhandler();
}

void
EventHandler::run_eventhandler()
{
  while (!(wait_and_process_events()));
}

void
EventHandler::install_eventhandler_context(EventHandlerContextHandle& context)
{  
  // install the new context atomically
  eventhandler_context_ = context;
}

void 
EventHandler::post_event(boost::function<void ()> function)
{
  EventHandle event = EventHandle(new EventT<boost::function<void ()> >(function));
  eventhandler_context_->post_event(event);
}

void 
EventHandler::post_and_wait_event(boost::function<void ()> function)
{
  if (is_eventhandler_thread())
  {
    function();
  }
  else
  {
    EventHandle event = EventHandle(new EventT<boost::function<void ()> >(function));
    eventhandler_context_->post_and_wait_event(event);
  }
}

// TERMINATE_EVENTHANDLER

void TerminateEventHandlerThread(EventHandlerHandle handle)
{
  handle->terminate_eventhandler();
}

void 
TerminateEventHandler(EventHandlerHandle& handle)
{
  boost::thread termination_thread(&TerminateEventHandlerThread,handle);
  handle.reset(); 
}


} // end namespace Core
