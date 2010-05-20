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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>

// QtInterface includes
#include <QtInterface/Utils/QtApplication.h>
#include <QtInterface/Utils/QtEventHandler.h>

// X11 includes
#ifdef X11_THREADSAFE
#include <X11/Xlib.h>
#endif

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( QtApplication );

QtApplication::QtApplication() :
  qt_application_( 0 )
{
}

bool QtApplication::setup( int argc, char **argv )
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
    qt_application_ = new QApplication( argc, argv );

    // Step 2: Create interface class to the main class of the event handler layer
    CORE_LOG_DEBUG( "Creating QtEventHandlerContext" );
    Core::EventHandlerContextHandle qt_eventhandler_context( new QtEventHandlerContext(
        qt_application_ ) );

    // Step 3: Insert the event handler into the application layer
    CORE_LOG_DEBUG( "Install the QtEventHandlerContext into the Interface layer" );
    Core::Interface::Instance()->install_eventhandler_context( qt_eventhandler_context );
    Core::Interface::Instance()->start_eventhandler();

    // Step 4: Create opengl render resources
    CORE_LOG_DEBUG( "Creating QtRenderResourcesContext" );
    qt_renderresources_context_ = QtRenderResourcesContextHandle( new QtRenderResourcesContext );

    CORE_LOG_DEBUG( "Install the QtRenderResources into the Interface layer" );
    // Step 5: Insert the render resources class into the application layer
    Core::RenderResources::Instance()->install_resources_context( qt_renderresources_context_ );

  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "QtApplication failed to initialize" );
    return ( false );
  }

  return ( true );
}

bool QtApplication::exec()
{
  bool success = true;
  try
  {
    CORE_LOG_DEBUG( "Starting main QT event loop" );

    if ( !( qt_application_->exec() == 0 ) )
    {
      CORE_LOG_DEBUG( "Qt crashed by dropping out of the event loop" );
      success = false;
    }

    delete qt_application_;
    qt_application_ = 0;

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
  return qt_application_;
}

QtRenderResourcesContextHandle QtApplication::qt_renderresources_context()
{
  return qt_renderresources_context_;
}

} // end namespace Seg3D
