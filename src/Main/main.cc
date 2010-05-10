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

// Plugin functionality
#include <Init/Init.h>
#include <Init/QtInit.h>

///////////////////////////////////////////////////////////
// Main Seg3D entry point
///////////////////////////////////////////////////////////

using namespace Seg3D;

int main( int argc, char **argv )
{
  // -- Stream messages to the console window --
  Core::LogStreamer error_log( Core::LogMessageType::ALL_E, &( std::cerr ) );

  // -- Parse the command line parameters --
  Core::Application::Instance()->parse_command_line_parameters( argc, argv );

  // -- Log application information --
  Core::Application::Instance()->log_start();

  // -- Checking for the socket parameter --
  std::string port_number_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "socket", port_number_string ) )
  {
    int port_number;
    if ( Core::FromString( port_number_string, port_number) )
    {
      // -- Add a socket for receiving actions --
      CORE_LOG_DEBUG( std::string("Starting a socket on port: ") + Core::ToString( port_number ) );
      Core::ActionSocket::Instance()->start( port_number );
    }
  }
  
  // -- Add plugins into the architecture  
  Core::InitPlugins();
  Seg3D::QtInitResources();

  // -- Start the application event handler --
  Core::Application::Instance()->start_eventhandler();

  // -- Setup the QT Interface Layer --
  if ( !( QtApplication::Instance()->setup( argc, argv ) ) ) return ( -1 );

  // -- Setup Application Interface Window --
  AppInterface* app_interface = new AppInterface;

  // Show the full interface
  app_interface->show();

  // Put the interface on top of all the other windows
  app_interface->raise();

  // The application window needs the qApplication as parent, which is
  // defined in the QtApplication, which integrates the Qt eventloop with
  // the interface eventloop of the Application layer.

  // -- Run QT event loop --
  if ( !( QtApplication::Instance()->exec() ) ) return ( -1 );

  // Indicate a successful finish of the program
  Core::Application::Instance()->log_finish();
  return ( 0 );
}

