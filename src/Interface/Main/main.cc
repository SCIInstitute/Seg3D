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

// STL includes
#include <iostream>
#include <string>

// Include CMake generated files
#include <Seg3DConfiguration.h>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/LogHistory.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Action/ActionHistory.h>
#include <Core/Action/ActionSocket.h>

// Interface includes
#include <Interface/QtInterface/QtApplication.h>
#include <Interface/AppInterface/AppInterface.h>

// Init Plugins functionality
#include <Init/Init.h>
#include <Init/QtInit.h>

// Boost Includes
#include<boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#ifdef X11_THREADSAFE
#include <X11/Xlib.h>
#endif

//#include <Interface/ControllerInterface/ControllerInterface.h>

///////////////////////////////////////////////////////////
// Main Seg3D entry point
///////////////////////////////////////////////////////////

using namespace Seg3D;

int main( int argc, char **argv )
{

#ifdef X11_THREADSAFE
  // Make X11 thread safe if the API is available.
  XInitThreads();
#endif

  // -- Setup error logging --
  // stream error to the console window
  Core::LogStreamer error_log( Core::LogMessageType::ALL_E, &( std::cerr ) );

  // -- Startup Seg3D --
  SCI_LOG_MESSAGE(std::string("--- Starting Seg3D ")+SEG3D_VERSION+" "+
    SEG3D_BITS+" "+SEG3D_DEBUG_VERSION+" ---");

  // -- Setup action history --
  SCI_LOG_DEBUG("Setup action history");
  Core::ActionHistory::Instance()->set_max_history_size( 300 );

  // -- Parse the command line parameters and put them in a stl::map --
  Core::Application::Instance()->parse_command_line_parameters( argc, argv );

  // -- Checking for the socket parameter --
  std::string port_number_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "socket", port_number_string ) )
  {
    int port_number;
    if ( Core::FromString( port_number_string, port_number) )
    {
      // -- Add a socket for receiving actions --
      SCI_LOG_DEBUG( std::string("Starting a socket on port: ") + 
        Core::ToString( port_number ) );
      Core::ActionSocket::Instance()->start( port_number );
    }
  }
  
  // -- Add plugins into the architecture  
  SCI_LOG_DEBUG("Setup and register all the plugins");
  Seg3D::InitPlugins();
  Seg3D::QtInitResources();

  // -- Setup the QT Interface Layer --
  SCI_LOG_DEBUG("Setup QT Interface");
  if ( !( QtApplication::Instance()->setup( argc, argv ) ) )
  {
    SCI_LOG_ERROR("Could not setup QT Interface");
    return ( -1 );
  }

  // -- Setup Application Interface Window --
  SCI_LOG_DEBUG("Setup Application Interface");

  // The application window needs the qApplication as parent, which is
  // defined in the QtApplication, which integrates the Qt eventloop with
  // the interface eventloop of the Application layer.
  try
  {
    AppInterface* app_interface = new AppInterface;

    // Show the full interface
    app_interface->show();

    // Put the interface on top of all the other windows
    app_interface->raise();

    // -- Run QT event loop --
    SCI_LOG_DEBUG("Start the main QT event loop");

    // Start the event processing loop
    if ( !( QtApplication::Instance()->exec() ) )
    {
      SCI_LOG_ERROR("The interface thread crashed, exiting Seg3D");
      return ( -1 );
    }

  }
  catch ( Core::Exception& except )
  {
    // Catch any Seg3D generated exceptions and display there message in the log file
    SCI_LOG_ERROR(std::string("Setup of the interface crashed by throwing an exception: ") + except.message());
    return ( -1 );
  }
  catch ( std::exception& except )
  {
    // For any other exception
    SCI_LOG_ERROR(std::string("Setup of the interface crashed by throwing an exception: ") + except.what());
    return ( -1 );
  }
  catch ( ... )
  {
    // For any other exception
    SCI_LOG_ERROR(std::string("Setup of the interface crashed by throwing an unknown exception"));
    return ( -1 );
  }

  // Indicate a successful finish of the program
  SCI_LOG_MESSAGE("--- Finished ---");
  return ( 0 );
}

