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

#ifdef _MSC_VER
#pragma warning( disable: 4244 4267 )
#endif

#ifdef BUILD_WITH_PYTHON
#include <Python.h>
#include <Core/Python/PythonInterpreter.h>
#include <Application/Socket/ActionSocket.h>

#include "ActionPythonWrapperRegistration.h"
#endif

// STL includes
#include <iostream>
#include <string>
#include <csignal>

// boost includes
#include <boost/preprocessor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/LogStreamer.h>
#include <Core/Utils/LogHistory.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Action/ActionHistory.h>
#include <Core/Log/RolloverLogFile.h>

// Application includes
#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/Tool/ToolFactory.h>

// File that contains a function that registers all the class that need registration,
// such as Actions and Tools
#include "ClassRegistration.h"

// Revision information
#include "Seg3D_RevisionInfo.h"

///////////////////////////////////////////////////////////
// Main Seg3D entry point
///////////////////////////////////////////////////////////

using namespace Seg3D;

bool seg3d_forever = true;

void sighandler(int sig)
{
  CORE_LOG_MESSAGE( std::string("Received signal: ") + Core::ExportToString( sig ) );
  seg3d_forever = false;
}

int main( int argc, char **argv )
{
  // -- Parse the command line parameters --
  Core::Application::Instance()->parse_command_line_parameters( argc, argv );
  
  // -- Check whether the user requested a version / revision number
  if ( Core::Application::Instance()->is_command_line_parameter( "revision") )
  {
    // NOTE: The revision information is gathered by cmake from svn. Hence if the local tree
    // contains modifications this information is not up-to-date
    std::cout << GIT_SEG3D_REVISIONINFO << std::endl;
    return 0;
  }

  if ( Core::Application::Instance()->is_command_line_parameter( "version") )
  {
    // NOTE: This information is gathered by cmake from the main cmake file.
    std::cout << Core::Application::Instance()->GetApplicationName() << " version: " <<  
      Core::Application::Instance()->GetVersion() << std::endl;
    return 0;
  }

  // -- Send message to revolving log file -- 
  // Logs messages in response to Log::Instance()->post_log_signal_
  Core::RolloverLogFile event_log( Core::LogMessageType::ALL_E );

#ifndef NDEBUG
  // -- Stream errors to console window
  new Core::LogStreamer( Core::LogMessageType::ALL_E, &std::cerr );
#endif

  // -- Log application information --
  Core::Application::Instance()->log_start();

  // -- Add plugins into the architecture  
  Core::RegisterClasses();

  // -- Start the application event handler --
  Core::Application::Instance()->start_eventhandler();

  // Initialize the startup tools list
  ToolFactory::Instance()->initialize_states();
  
  // Trigger the application start signal
  Core::Application::Instance()->application_start_signal_();
  
  // -- Warn user about being an alpha/beta version --
  std::string file_to_view = "";
  Core::Application::Instance()->check_command_line_parameter( "file_to_open_on_start", file_to_view );

  if ( sizeof( void * ) == 4 )
  {
    std::string warning = std::string( "<h3>" ) +
      Core::Application::GetApplicationName() + " " + Core::Application::GetVersion() + " 32BIT" 
      "</h3><h6><p align=\"justify\">Please note: " + Core::Application::GetApplicationName()
      + " is meant to run in 64-bit mode. "
      "In 32-bit mode the size of volumes that can be processed are limited, as "
      + Core::Application::GetApplicationName() +
      " may run out of addressable memory. If you have a 64-bit machine,"
      " we would recommend to download the 64-bit version</p></h6>";

      CORE_LOG_WARNING(warning);
  }
#ifdef BUILD_WITH_PYTHON
  size_t name_len = strlen( argv[ 0 ] );
  std::vector< wchar_t > program_name( name_len + 1 );
  mbstowcs( &program_name[ 0 ], argv[ 0 ], name_len + 1 );

  Core::PythonInterpreter::module_list_type python_modules;
  std::string module_name = Core::StringToLower( BOOST_PP_STRINGIZE( APPLICATION_NAME ) );
  python_modules.push_back( Core::PythonInterpreter::module_entry_type( module_name, 
    BOOST_PP_CAT( PyInit_, APPLICATION_NAME ) ) );
  Core::PythonInterpreter::Instance()->initialize( &program_name[ 0 ], python_modules );
  Core::PythonInterpreter::Instance()->run_string( "import " + module_name + "\n" );
  Core::PythonInterpreter::Instance()->run_string( "from " + module_name + " import *\n" );

  // -- Checking for the socket parameter --
  std::string port_number_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "socket", port_number_string ) ||
       Core::Application::Instance()->check_command_line_parameter( "port", port_number_string ))
  {
    int port_number;
    if ( Core::ImportFromString( port_number_string, port_number) )
    {
      // -- Add a socket for receiving actions --
      CORE_LOG_MESSAGE( std::string("Starting a socket on port: ") + Core::ExportToString( port_number ) );
      Seg3D::ActionSocket::Instance()->start( port_number );
      InterfaceManager::Instance()->set_initializing( true );
      InterfaceManager::Instance()->splash_screen_visibility_state_->set( false );
      InterfaceManager::Instance()->set_initializing( false );
    }
  }
#else
  std::string port_number_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "socket", port_number_string ) ||
       Core::Application::Instance()->check_command_line_parameter( "port", port_number_string ))
  {
    CORE_LOG_WARNING("Opening a socket is not supported without Python.");
  }
#endif

  // -- Setup Application Interface Window --
//  std::string file_to_view = "";
//  Core::Application::Instance()->check_command_line_parameter( "file_to_open_on_start", file_to_view );
  

  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  while(seg3d_forever)
  {
    // do nothing, check every second for a termination signal
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  }

  // Trigger the application stop signal
  CORE_LOG_MESSAGE( std::string("sending application stop signal") );
  Core::Application::Instance()->application_stop_signal_();


  // Finish the remainder of the actions that are still on the application thread.
  CORE_LOG_MESSAGE( std::string("finishing application") );
  Core::Application::Instance()->finish();

  // Indicate a successful finish of the program
  CORE_LOG_MESSAGE( std::string("finishing log, then exit") );
  Core::Application::Instance()->log_finish();

  // see if we can just return now that we're not using Qt
  return ( 0 );
  /*#if defined (_WIN32) || defined(__APPLE__)
  return ( 0 );
#else
    // NOTE: On Linux Qt tends to crash in one of its static destructors. Since we do not need these
    // destructors to be executed, we just exit in stead. For Windows we return to WinMain, hence
    // we need to return in that case.
    exit ( 0 );
#endif
  */
}
