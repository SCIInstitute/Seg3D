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

#include <Application/Application/DefaultApplicationContext.h>

namespace Seg3D {

DefaultApplicationContext::DefaultApplicationContext()
{
}

DefaultApplicationContext::~DefaultApplicationContext()
{
}

void
DefaultApplicationContext::obtain()
{
  application_thread_id_ = boost::this_thread::get_id();
}


void
DefaultApplicationContext::release()
{
  // Clear the current application 
  if (boost::this_thread::get_id() != application_thread_id_) 
  {
    application_queue_mutex_.lock();
    while (!application_queue_.empty()) application_queue_.pop();
    application_queue_mutex_.unlock();
  }
  else
  {
    process_events();
  } 
}

void
DefaultApplicationContext::post_application_event(ApplicationEventHandle& event)
{
  // Need to lock queue before inserting message
  application_queue_mutex_.lock();
  // Adding event to queue
  application_queue_.push(event);
  application_queue_mutex_.unlock();
}  

void
DefaultApplicationContext::post_and_wait_application_event(ApplicationEventHandle& event)
{
  // Need a synchronization class to confirm the event is done
  ApplicationEventSyncHandle sync = 
    ApplicationEventSyncHandle(new ApplicationEventSync);
  // Add event to event class so the responder is doing the other part
  // of the synchronization
  event->sync_handle() = sync;

  // Need to lock this lock, so the handshaking does not happen until this
  // thread waits
  boost::unique_lock<boost::mutex> lock(sync->lock_);

  // Need to lock queue before inserting message
  application_queue_mutex_.lock();
  // Adding event to queue
  application_queue_.push(event);
  application_queue_mutex_.unlock();
  
  // wait for application to handle the event
  sync->condition_.wait(lock);
}  

void
DefaultApplicationContext::process_events()
{
  // Only run on the application thread
  if (boost::this_thread::get_id() != application_thread_id_) 
  {
    // TODO: need to assert here, the event processing can only be
    // on the thread that was designated for event handling
    return;
  }
  
  // lock the queue, so it is not changed while we are taking events of the
  // the list.
  application_queue_mutex_.lock();
  while (! application_queue_.empty())
  {
    // get the next event
    ApplicationEventHandle event_handle = application_queue_.front();
    // pop the handled event from the list
    application_queue_.pop();

    // unlock so the call back can add another event on the list
    application_queue_mutex_.unlock();
    // run the call back
    event_handle->handle_event();
    // lock again so we can change the queue
    application_queue_mutex_.lock();
  }
  
  // done with application_queue
  application_queue_mutex_.unlock();
}

bool
DefaultApplicationContext::is_application_thread() const
{
  return (boost::this_thread::get_id() == application_thread_id_); 
}

} // end namespace Seg3D
