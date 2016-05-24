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

// Qt includes
#include <QMessageBox>

// Core includes
#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

// Interface includes
#include <QtUtils/Utils/QtEventHandler.h>

namespace QtUtils
{

// CLASS QtEventHandlerObject

QtEventHandlerObject::QtEventHandlerObject( QApplication* parent, 
  QtEventHandlerContext* context ) :
  QObject( parent ),
  parent_( parent ),
  event_handler_context_( context )
{
  this->connect( parent, SIGNAL( aboutToQuit() ), SLOT( cleanup() ) );
  this->timer_id_ = this->startTimer( 100 );
  this->parent_->installEventFilter( this );
}

bool QtEventHandlerObject::eventFilter( QObject* watched, QEvent* event )
{
  if ( event->type() == QEvent::User )
  {
    this->event_handler_context_->process_events();
    return true;
  }
  
  return QObject::eventFilter( watched, event );
}

void QtEventHandlerObject::timerEvent( QTimerEvent* event )
{
  this->event_handler_context_->process_events();
}

void QtEventHandlerObject::cleanup()
{
  this->parent_->removeEventFilter( this );
  this->killTimer( this->timer_id_ );
  this->event_handler_context_->empty_event_queue();
}

// CLASS QtEventHandlerContext:
// This class needs to be constructed from the main Qt thread

QtEventHandlerContext::QtEventHandlerContext( QApplication* qapplication ) :
  EventHandlerContext(),
  eventhandler_started_( false ),
  eventhandler_stopped_( false ),
  qapplication_( qapplication ), 
  interface_thread_id_( boost::this_thread::get_id() )
{
}

QtEventHandlerContext::~QtEventHandlerContext()
{
}

void QtEventHandlerContext::post_event( Core::EventHandle& event )
{
  if ( !QCoreApplication::closingDown() )
  {
    {
      lock_type lock( this->mutex_ );
      if ( this->eventhandler_stopped_ ) return;
      this->events_.push( event );
    }
    // Generate an empty Qt user event and insert it to the Qt main event loop.
    // The QtEventHandlerObject event filter will filter this event out and trigger
    // process_events.
    // This is an ordinary pointer as QT will take ownership of it and delete it.
    QCoreApplication::postEvent( qapplication_, new QEvent( QEvent::User ) );
  }
}

void QtEventHandlerContext::post_and_wait_event( Core::EventHandle& event )
{
  // Need a synchronization class to confirm the event is done
  Core::EventSyncHandle sync = Core::EventSyncHandle( new Core::EventSync );

  // Add event to event class so the responder is doing the other part
  // of the synchronization
  event->sync_handle() = sync;

  // Handshaking for communication with QT
  boost::unique_lock< boost::mutex > lock( sync->lock_ );

  // Add the event to the queue
  {
    lock_type lock( this->mutex_ );
    if ( this->eventhandler_stopped_ ) return;
    this->events_.push( event );
  }

  // Post a user event to the QT event loop
  QCoreApplication::postEvent( qapplication_, new QEvent( QEvent::User ) );

  // Wait for QT to handle the event
  sync->condition_.wait( lock );
}

bool QtEventHandlerContext::process_events()
{
  if ( !this->is_eventhandler_thread() )
  {
    CORE_THROW_LOGICERROR( "Cannot process events from outside the Qt thread" );
  }

  lock_type lock( this->mutex_ );

  while ( !this->events_.empty() )
  {
    try
    {
      Core::EventHandle event = this->events_.front();
      this->events_.pop();
      // NOTE: It's important to unlock before handling the event, 
      // otherwise deadlock might happen.
      lock.unlock();
      event->handle_event();
      lock.lock();
    }
    catch ( Core::Exception& except )
    {
      // Catch any Seg3D generated exceptions and display there message in the log file
      std::string error_message = 
        std::string( "Interface event loop crashed by throwing an exception: " ) + 
        except.message();

      CORE_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ), 
        QString::fromStdString( "Fatal Error:\n\n" + error_message ) );
      QCoreApplication::exit( -1 );
    }
    catch ( std::exception& except )
    {
      // For any other exception
      std::string error_message = 
        std::string( "Interface event loop crashed by throwing an exception: " ) + 
        except.what();

      CORE_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ), 
        QString::fromStdString( "Fatal Error:\n\n" + error_message ) );
      QCoreApplication::exit( -1 );
    }
    catch ( ... )
    {
      // For any other exception
      std::string error_message =  
        std::string( "Interface event loop crashed by throwing an unknown exception" );

      CORE_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ),
        QString::fromStdString( "Fatal Error:\n\n" + error_message ) );
      QCoreApplication::exit( -1 );
    }
  }

  return false;
}

bool QtEventHandlerContext::wait_and_process_events()
{
  CORE_THROW_LOGICERROR("Cannot wait on the Qt thread");
}

bool QtEventHandlerContext::start_eventhandler( Core::EventHandler* eventhandler )
{
  // Allow Qt to intercept the actions in its main event loop 
  new QtEventHandlerObject( this->qapplication_, this );

  eventhandler_started_ = true;
  return ( true );
}

bool QtEventHandlerContext::eventhandler_started()
{
  return  eventhandler_started_;
}


void QtEventHandlerContext::terminate_eventhandler()
{
  CORE_THROW_LOGICERROR("Cannot terminate the Qt thread");
}

bool QtEventHandlerContext::is_eventhandler_thread() const
{
  return ( boost::this_thread::get_id() == interface_thread_id_ );
}

void QtEventHandlerContext::empty_event_queue()
{
  lock_type lock( this->mutex_ );
  while ( !this->events_.empty() )
  {
    this->events_.pop();
  }
  
  this->eventhandler_stopped_ = true;
}

} // end namespace QtUtils
