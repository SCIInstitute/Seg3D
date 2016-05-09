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

// STL includes
#include <queue>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>


#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

#include <Core/EventHandler/Event.h>
#include <Core/EventHandler/EventHandler.h>
#include <Core/EventHandler/DefaultEventHandlerContext.h>

namespace Core
{

class DefaultEventHandlerContextPrivate
{
public:
  // Queue type that is used to store the events
  typedef std::queue< EventHandle > event_queue_type;

  // Whether the eventhandler started
  bool eventhandler_started_;

  // EventHandler thread id
  boost::thread* eventhandler_thread_;

  // Mutex protecting the event queue
  boost::mutex event_queue_mutex_;

  // Condition variable signaling that a new event was posted
  boost::condition_variable event_queue_new_event_;

  // The event queue
  event_queue_type event_queue_;

  // Indicating that event handling is done
  bool done_;

  // Signal handling to ensure thread is running before returning from
  // start_eventhandler
  boost::mutex thread_mutex_;
  boost::condition_variable thread_condition_variable_; 
  
  // Function for safely starting thread
  void start_thread( EventHandler* eventhandler );  
};

void DefaultEventHandlerContextPrivate::start_thread( EventHandler* eventhandler )
{
  {
    boost::unique_lock< boost::mutex > lock( thread_mutex_ );
    thread_condition_variable_.notify_all();
  }
  eventhandler->run_eventhandler();
}

DefaultEventHandlerContext::DefaultEventHandlerContext() :
  private_( new DefaultEventHandlerContextPrivate )
{
  this->private_->eventhandler_thread_ = 0;
  this->private_->eventhandler_started_ = false;
  this->private_->done_ = false;
}

DefaultEventHandlerContext::~DefaultEventHandlerContext()
{
  delete this->private_->eventhandler_thread_;
}

void DefaultEventHandlerContext::post_event( EventHandle& event )
{
  boost::unique_lock< boost::mutex > lock( this->private_->event_queue_mutex_ );
  this->private_->event_queue_.push( event );
  this->private_->event_queue_new_event_.notify_all();
}

void DefaultEventHandlerContext::post_and_wait_event( EventHandle& event )
{
  // Need a synchronization class to confirm the event is done
  EventSyncHandle sync = EventSyncHandle( new EventSync );
  // Add event to event class so the responder is doing the other part
  // of the synchronization
  event->sync_handle() = sync;

  // Need to lock this lock, so the handshaking does not happen until this
  // thread waits
  boost::unique_lock< boost::mutex > lock( sync->lock_ );

  {
    // Need to lock queue before inserting message
    boost::unique_lock< boost::mutex > lock( this->private_->event_queue_mutex_ );
    // Adding event to queue
    this->private_->event_queue_.push( event );
    this->private_->event_queue_new_event_.notify_all();
  }

  // wait for application to handle the event
  sync->condition_.wait( lock );
}

bool DefaultEventHandlerContext::process_events()
{
  // Only run on the application thread
  if ( boost::this_thread::get_id() != this->private_->eventhandler_thread_->get_id() )
  {
    CORE_THROW_LOGICERROR("process_events was called from a thread that is not processing the events");
  }

  // lock the queue, so it is not changed while we are taking events of the
  // the list.
  boost::unique_lock< boost::mutex > lock( this->private_->event_queue_mutex_ );

  while ( ! ( this->private_->event_queue_.empty() ) )
  {
    // get the next event
    EventHandle event_handle;
    event_handle.swap( this->private_->event_queue_.front() );
    // pop the handled event from the list
    this->private_->event_queue_.pop();

    // unlock so the call back can add another event on the list
    lock.unlock();
    // run the call back
    event_handle->handle_event();
    // lock again so we can change the queue
    lock.lock();
  }

  return ( this->private_->done_ );
}

bool DefaultEventHandlerContext::wait_and_process_events()
{
  // lock the queue, so it is not changed while we are taking events of the
  // the list.
  boost::unique_lock< boost::mutex > lock( this->private_->event_queue_mutex_ );

  // wait for an event to come if the event queue is empty
  if ( this->private_->event_queue_.empty() ) 
  {
    this->private_->event_queue_new_event_.wait( lock );
  }
  
  while ( ! ( this->private_->event_queue_.empty() ) )
  {
    // get the next event
    EventHandle event_handle;
    event_handle.swap( this->private_->event_queue_.front() );
    // pop the handled event from the list
    this->private_->event_queue_.pop();

    // unlock so the call back can add another event on the list
    lock.unlock();

    // run the call back
    event_handle->handle_event();

    // lock again so we can change the queue
    lock.lock();
  }

  return ( this->private_->done_ );
}

bool DefaultEventHandlerContext::is_eventhandler_thread() const
{
  return ( boost::this_thread::get_id() == 
    this->private_->eventhandler_thread_->get_id() );
}

bool DefaultEventHandlerContext::start_eventhandler( EventHandler* eventhandler )
{
  // Generate a new thread that will run the eventhandler
  // It needs a pointer to the run_eventhandler() and will
  // use that as callable

  boost::unique_lock< boost::mutex > lock( this->private_->thread_mutex_ );
  this->private_->eventhandler_thread_ = new
    boost::thread( boost::bind( &DefaultEventHandlerContextPrivate::start_thread,
      this->private_.get(), eventhandler ) );
    
  // wait for thread to be initialized
  this->private_->thread_condition_variable_.wait( lock );
  
  this->private_->eventhandler_started_ = true;
  return ( true );
}

bool DefaultEventHandlerContext::eventhandler_started()
{
  return this->private_->eventhandler_started_;
}

void DefaultEventHandlerContext::terminate_eventhandler()
{
  // Signal that we are done handling events
  {
    // Lock the state of the eventhandler
    boost::unique_lock< boost::mutex > lock( this->private_->event_queue_mutex_ );

    // If it is already done, exit the function as this
    // function has already been executed
    if ( this->private_->done_ == true ) return;

    // Mark the eventhandler as done
    this->private_->done_ = true;

    // Notify the thread waiting for input that it can stop waiting
    this->private_->event_queue_new_event_.notify_one();
  }

  // Join the thread back into the main application thread
  this->private_->eventhandler_thread_->join();
}

} // end namespace Core
