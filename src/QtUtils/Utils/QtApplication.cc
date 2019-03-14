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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Utils/QtApplication.h>
#include <QtUtils/Utils/QtEventHandler.h>

// X11 includes
#ifdef X11_THREADSAFE
#include <X11/Xlib.h>
#endif

// Qt includes
#include <QAbstractNativeEventFilter>

namespace QtUtils
{


class OverrideQApplication : public QApplication 
{
public:
  OverrideQApplication( int& argc, char** argv ) : QApplication( argc, argv )
  {
  }
  
  virtual ~OverrideQApplication() 
  {
  }
  
protected:
  bool event( QEvent *event ) 
  {
    if (event->type() == QEvent::FileOpen) 
    {
      std::string filename = ( static_cast< QFileOpenEvent* > ( event )->file() ).toStdString();
      QtApplication::Instance()->osx_file_open_event_signal_( filename );
      return true;
    }
    
    return QApplication::event( event );
  }
};

class QtWin32ApplicationEventFilter : public QAbstractNativeEventFilter
{
public:
  virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE
  {
#ifdef _WIN32

    MSG* msg = reinterpret_cast< MSG* >( message );
    if ( msg->message == WM_ENTERSIZEMOVE )
    {
      QtApplication::Instance()->enter_size_move_signal_();
    }
    else if ( msg->message == WM_EXITSIZEMOVE )
    {
      QtApplication::Instance()->exit_size_move_signal_();
    }
#endif

    return false;
  }
};

class QtApplicationPrivate
{
public:
  // Main QT application class
  QApplication* qt_application_;
  QtWin32ApplicationEventFilter* win32_event_filter_;

  // Class for managing the opengl rendering resources
  QtRenderResourcesContextHandle qt_renderresources_context_;
};


CORE_SINGLETON_IMPLEMENTATION( QtApplication );

QtApplication::QtApplication() :
  private_( new QtApplicationPrivate )
{
  this->private_->qt_application_ = 0;
  this->private_->win32_event_filter_ = 0;
}

bool QtApplication::setup( int& argc, char **argv )
{
  CORE_LOG_DEBUG( "Setup QT Application" );

#ifdef X11_THREADSAFE
  // Make X11 thread safe if the API is available.
  XInitThreads();
#endif

  try
  {
    // Step 1: Main application class
    CORE_LOG_DEBUG( "Creating QApplication" );
	//Check if QApplication exists
	if (!this->private_->qt_application_)
	  this->private_->qt_application_ = new OverrideQApplication(argc, argv);

    this->private_->win32_event_filter_ = new QtWin32ApplicationEventFilter;

    // Set the native event filter for QApplication
    this->private_->qt_application_->installNativeEventFilter( this->private_->win32_event_filter_ );

    // Set the style handler to fusion to get good
    // fundamental behavior, especially for comboboxes
    this->private_->qt_application_->setStyle( "fusion" );

    // Step 2: Create interface class to the main class of the event handler layer
    CORE_LOG_DEBUG( "Creating QtEventHandlerContext" );
    Core::EventHandlerContextHandle qt_eventhandler_context( new QtEventHandlerContext(
        this->private_->qt_application_ ) );

    // Step 3: Insert the event handler into the application layer
    CORE_LOG_DEBUG( "Install the QtEventHandlerContext into the Interface layer" );
    Core::Interface::Instance()->install_eventhandler_context( qt_eventhandler_context );
    Core::Interface::Instance()->start_eventhandler();

    // Step 4: Create opengl render resources
    CORE_LOG_DEBUG( "Creating QtRenderResourcesContext" );
    this->private_->qt_renderresources_context_.reset( new QtRenderResourcesContext );

    CORE_LOG_DEBUG( "Install the QtRenderResources into the Interface layer" );
    // Step 5: Insert the render resources class into the application layer
    Core::RenderResources::Instance()->install_resources_context( 
      this->private_->qt_renderresources_context_ );

  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "QtApplication failed to initialize" );
    return ( false );
  }

  return ( true );
}

void QtApplication::setExternalInstance(QApplication* app)
{
	this->private_->qt_application_ = app;
}

bool QtApplication::exec()
{
  bool success = true;
  try
  {
    CORE_LOG_DEBUG( "Starting main QT event loop" );

    if ( !( this->private_->qt_application_->exec() == 0 ) )
    {
      CORE_LOG_DEBUG( "Qt crashed by dropping out of the event loop" );
      success = false;
    }

    delete this->private_->qt_application_;
    this->private_->qt_application_ = 0;

    CORE_LOG_DEBUG( "Exiting main QT event loop" );
  }
  catch ( Core::Exception& except )
  {
    // Catch any Seg3D generated exceptions and display there message in the log file
    CORE_LOG_ERROR( std::string( "Setup of the interface crashed by throwing an exception: " ) +
      except.message() );
    success = false;
  }
  catch ( std::exception& except )
  {
    // For any other exception
    CORE_LOG_ERROR( std::string( "Setup of the interface crashed by throwing an exception: " ) +
      except.what() );
    success = false;
  }
  catch ( ... )
  {
    // For any other exception
    CORE_LOG_ERROR( std::string( "Setup of the interface crashed by throwing an unknown exception" ) );
    success = false;
  } 

  return ( success );
}

QApplication* QtApplication::qt_application()
{
  return this->private_->qt_application_;
}

QtRenderResourcesContextHandle QtApplication::qt_renderresources_context()
{
  return this->private_->qt_renderresources_context_;
}

} // end namespace Seg3D
