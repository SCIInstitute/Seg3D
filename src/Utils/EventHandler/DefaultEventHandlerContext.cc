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

#include <Utils/EventHandler/DefaultEventHandlerContext.h>
#include <Utils/EventHandler/EventHandler.h>

namespace Utils {

DefaultEventHandlerContext::DefaultEventHandlerContext() :
  done_(false)
{
}

DefaultEventHandlerContext::~DefaultEventHandlerContext()
{
}

void
DefaultEventHandlerContext::post_event(EventHandle& event)
{
  boost::unique_lock<boost::mutex> lock(event_queue_mutex_);
  event_queue_.push(event);
}  

void
DefaultEventHandlerContext::post_and_wait_event(EventHandle& event)
{
  // Need a synchronization class to confirm the event is done
  EventSyncHandle sync = EventSyncHandle(new EventSync);
  // Add event to event class so the responder is doing the other part
  // of the synchronization
  event->sync_handle() = sync;

  // Need to lock this lock, so the handshaking does not happen until this
  // thread waits
  boost::unique_lock<boost::mutex> lock(sync->lock_);

  {
    // Need to lock queue before inserting message
    boost::unique_lock<boost::mutex> lock(event_queue_mutex_);
    // Adding event to queue
    event_queue_.push(event);
    event_queue_new_event_.notify_one();
  }
  
  // wait for application to handle the event
  sync->condition_.wait(lock);
}  


bool
DefaultEventHandlerContext::process_events()
{
  // Only run on the application thread
  if (boost::this_thread::get_id() != eventhandler_thread_.get_id()) 
  {
    SCI_THROW_LOGICERROR("process_events was called from a thread that is processing the events");
  }
  
  // lock the queue, so it is not changed while we are taking events of the
  // the list.
  boost::unique_lock<boost::mutex> lock(event_queue_mutex_);
  
  while (!event_queue_.empty())
  {
    // get the next event
    EventHandle event_handle;
    event_handle.swap(event_queue_.front());
    // pop the handled event from the list
    event_queue_.pop();

    // unlock so the call back can add another event on the list
    lock.unlock();
    // run the call back
    event_handle->handle_event();
    // lock again so we can change the queue
    lock.lock();
  }
  
  return (done_);
}


bool
DefaultEventHandlerContext::wait_and_process_events()
{
  // Only run on the eventhandler thread
  if (boost::this_thread::get_id() != eventhandler_thread_.get_id()) 
  {
    SCI_THROW_LOGICERROR("wait_and_process_events was called from a thread that is processing the events");
  }
  
  // lock the queue, so it is not changed while we are taking events of the
  // the list.
  boost::unique_lock<boost::mutex> lock(event_queue_mutex_);
  
  // wait for an event to come if the event queue is empty
  if (event_queue_.empty()) event_queue_new_event_.wait(lock);  
  
  while (!event_queue_.empty())
  {
    // get the next event
    EventHandle event_handle;
    event_handle.swap(event_queue_.front());
    // pop the handled event from the list
    event_queue_.pop();

    // unlock so the call back can add another event on the list
    lock.unlock();
    // run the call back
    event_handle->handle_event();
    // lock again so we can change the queue
    lock.lock();
  }

  return (done_);
}

bool
DefaultEventHandlerContext::is_eventhandler_thread() const
{
  return (boost::this_thread::get_id() == eventhandler_thread_.get_id()); 
}

bool
DefaultEventHandlerContext::start_eventhandler(EventHandler* eventhandler)
{
  // Generate a new thread that will run the eventhandler
  // It needs a pointer to the run_eventhandler() and will
  // use that as callable
  eventhandler_thread_ = boost::thread(
                    boost::bind(&EventHandler::run_eventhandler,eventhandler));
}

void
DefaultEventHandlerContext::terminate_eventhandler()
{
  // Signal that we are done handling events
  {
    // Lock the state of the eventhandler
    boost::unique_lock<boost::mutex> lock(event_queue_mutex_);

    // If it is already done, exit the function as this
    // function has already been exeecuted
    if (done_ == true) return;

    // Mark the eventhandler as done
    done_ = true;  
    
    // Notify the thread waiting for input that it can stop waiting
    event_queue_new_event_.notify_one(); 
  }
  
  // Join the thread back into the main application thread
  eventhandler_thread_.join();  
}

} // end namespace Utils
