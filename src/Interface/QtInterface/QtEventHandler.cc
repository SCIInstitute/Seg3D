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

// Qt includes
#include <QMessageBox>

// Utils includes
#include <Utils/Core/Exception.h>
#include <Utils/Core/Log.h>

// Interface includes
#include <Interface/QtInterface/QtEventHandler.h>

namespace Seg3D
{

// CLASS QtUserEvent:

QtUserEvent::QtUserEvent( Utils::EventHandle& event_handle ) :
  QEvent( QEvent::User ), event_handle_( event_handle )
{
}

QtUserEvent::~QtUserEvent()
{
}

// CLASS QtEventFilter:

bool QtEventFilter::eventFilter( QObject* obj, QEvent* event )
{
  if ( event->type() == QEvent::User )
  {
    // Cast to the right type
    QtUserEvent *user_event = dynamic_cast< QtUserEvent * > ( event );

    // Run the code that was send to us
    try
    {
      user_event->event_handle_->handle_event();
    }
    catch ( Utils::Exception& except )
    {
      // Catch any Seg3D generated exceptions and display there message in the log file
      std::string error_message = 
        std::string( "Interface event loop crashed by throwing an exception: " ) + 
        except.message();
        
      SCI_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ), 
        QString::fromStdString( error_message) );
      QCoreApplication::exit( -1 );
      return ( true );
    }
    catch ( std::exception& except )
    {
      // For any other exception
      std::string error_message = 
        std::string( "Interface event loop crashed by throwing an exception: " ) + 
        except.what();
        
      SCI_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ),  
        QString::fromStdString( error_message) );
      QCoreApplication::exit( -1 );
      return ( true );
    }
    catch ( ... )
    {
      // For any other exception
      std::string error_message =  
        std::string( "Interface event loop crashed by throwing an unknown exception" );

      SCI_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ),  
        QString::fromStdString( error_message) );
      QCoreApplication::exit( -1 );
      return ( true );
    }

    return ( true );
  }
  else
  {
    return ( QObject::eventFilter( obj, event ) );
  }
}

// CLASS QtEventHandlerObject

QtEventHandlerObject::QtEventHandlerObject( QApplication* parent, 
  QtEventHandlerContext* context ) :
  QObject( parent ),
  parent_( parent ),
  event_handler_context_( context ),
  event_filter_enabled_( false )
{
  this->connect( parent, SIGNAL( aboutToQuit() ), SLOT( cleanup() ) );
  this->timer_id_ = this->startTimer( 50 );
  this->parent_->installEventFilter( this );
}

bool QtEventHandlerObject::eventFilter( QObject* watched, QEvent* event )
{
  if ( event->type() == QEvent::User )
  {
    if ( event_filter_enabled_ )
    {
      this->event_handler_context_->process_events();
    }
    return true;
  }
  
  return QObject::eventFilter( watched, event );
}

void QtEventHandlerObject::timerEvent( QTimerEvent* event )
{
  this->event_handler_context_->process_events();
  this->event_filter_enabled_ = true;
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
  qapplication_( qapplication ), 
  interface_thread_id_( boost::this_thread::get_id() )
{
}

QtEventHandlerContext::~QtEventHandlerContext()
{
}

void QtEventHandlerContext::post_event( Utils::EventHandle& event )
{
  {
  }

  // Generate a new event message for QT
  // This is an ordinary pointer as QT will
  // take ownership of it and delete it.

  //QtUserEvent* qevent = new QtUserEvent( event );

  // Insert call into the QT event loop
  if ( !QCoreApplication::closingDown() )
  {
    {
      lock_type lock( this->mutex_ );
      this->events_.push( event );
    }
    QCoreApplication::postEvent( qapplication_, new QEvent( QEvent::User ) );
  }
}

void QtEventHandlerContext::post_and_wait_event( Utils::EventHandle& event )
{
  // Need a synchronization class to confirm the event is done
  Utils::EventSyncHandle sync = Utils::EventSyncHandle( new Utils::EventSync );

  // Add event to event class so the responder is doing the other part
  // of the synchronization
  event->sync_handle() = sync;

  // Generate a new event message for QT:
  // This is an ordinary pointer as QT will take ownership of it and delete it.
  QtUserEvent* qevent = new QtUserEvent( event );

  // Handshaking for communication with QT
  boost::unique_lock< boost::mutex > lock( sync->lock_ );

  // Insert call into the QT event loop
  QApplication::postEvent( qapplication_, qevent );

  // Wait for QT to handle the event
  sync->condition_.wait( lock );
}

bool QtEventHandlerContext::process_events()
{
  if ( !this->is_eventhandler_thread() )
  {
    SCI_THROW_LOGICERROR( "Cannot process events from outside the Qt thread" );
  }


  lock_type lock( this->mutex_ );

  while ( !this->events_.empty() )
  {
    try
    {
      Utils::EventHandle event = this->events_.front();
      this->events_.pop();
      // NOTE: It's important to unlock before handling the event, 
      // otherwise deadlock might happen.
      lock.unlock();
      event->handle_event();
      lock.lock();
    }
    catch ( Utils::Exception& except )
    {
      // Catch any Seg3D generated exceptions and display there message in the log file
      std::string error_message = 
        std::string( "Interface event loop crashed by throwing an exception: " ) + 
        except.message();

      SCI_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ), 
        QString::fromStdString( error_message) );
      QCoreApplication::exit( -1 );
    }
    catch ( std::exception& except )
    {
      // For any other exception
      std::string error_message = 
        std::string( "Interface event loop crashed by throwing an exception: " ) + 
        except.what();

      SCI_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ),  
        QString::fromStdString( error_message) );
      QCoreApplication::exit( -1 );
    }
    catch ( ... )
    {
      // For any other exception
      std::string error_message =  
        std::string( "Interface event loop crashed by throwing an unknown exception" );

      SCI_LOG_ERROR( error_message );
      QMessageBox::critical( 0, QString( "Fatal Error" ),  
        QString::fromStdString( error_message) );
      QCoreApplication::exit( -1 );
    }
  }

  return false;
}

bool QtEventHandlerContext::wait_and_process_events()
{
  SCI_THROW_LOGICERROR("Cannot wait on the Qt thread");
}

bool QtEventHandlerContext::start_eventhandler( Utils::EventHandler* eventhandler )
{
  // Allow Qt to intercept the actions in its main event loop
  QtEventHandlerObject* event_handler_obj = 
    new QtEventHandlerObject( this->qapplication_, this );

  return ( true );
}

void QtEventHandlerContext::terminate_eventhandler()
{
  SCI_THROW_LOGICERROR("Cannot terminate the Qt thread");
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
}

} // end namespace Seg3D
