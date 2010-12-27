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

#ifdef BUILD_WITH_PYTHON
#include <Python.h>
#include <Application/PythonModule/PythonInterpreter.h>
#endif

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
#include <Core/Log/RolloverLogFile.h>

// QtUtils includes
#include <QtUtils/Utils/QtApplication.h>

// Application includes
#include <Application/Tool/ToolFactory.h>

// Interface includes
#include <Interface/Application/ApplicationInterface.h>

// Resource includes
#include <Resources/QtResources.h>

// File that contains a function that registers all the class that need registration,
// such as Actions and Tools
#include "ClassRegistration.h"

// Revision information
#include "Seg3D_RevisionInfo.h"

///////////////////////////////////////////////////////////
// Main Seg3D entry point
///////////////////////////////////////////////////////////

using namespace Seg3D;

int main( int argc, char **argv )
{
  // -- Parse the command line parameters --
  Core::Application::Instance()->parse_command_line_parameters( argc, argv );
  
  // -- Check whether the user requested a version / revision number
  if ( Core::Application::Instance()->is_command_line_parameter( "revision") )
  {
    // NOTE: The revision information is gathered by cmake from svn. Hence if the local tree
    // contains modifications this information is not up-to-date
    std::cout << SEG3D_REVISIONINFO << std::endl;
    return 0;
  }

  if ( Core::Application::Instance()->is_command_line_parameter( "version") )
  {
    // NOTE: This information is gathered by cmake from the main cmake file.
    std::cout << Core::Application::Instance()->GetApplicationName() << " version: " <<  
      Core::Application::Instance()->GetReleaseName() << " " <<
      Core::Application::Instance()->GetVersion() << std::endl;
    return 0;
  }

  // -- Send message to revolving log file -- 
  Core::RolloverLogFile event_log( Core::LogMessageType::ALL_E );

  // -- Log application information --
  Core::Application::Instance()->log_start();

  // -- Checking for the socket parameter --
  std::string port_number_string;
  if( Core::Application::Instance()->check_command_line_parameter( "socket", port_number_string ) )
  {
    int port_number;
    if ( Core::ImportFromString( port_number_string, port_number) )
    {
      // -- Add a socket for receiving actions --
      CORE_LOG_DEBUG( std::string("Starting a socket on port: ") + Core::ExportToString( port_number ) );
      Core::ActionSocket::Instance()->start( port_number );
    }
  }
    
  // -- Add plugins into the architecture  
  Core::RegisterClasses();

  // -- Start the application event handler --
  Core::Application::Instance()->start_eventhandler();

  // Initialize the startup tools list
  ToolFactory::Instance()->initialize_states();
  
  // -- Setup the QT Interface Layer --
  if ( !( QtUtils::QtApplication::Instance()->setup( argc, argv ) ) ) return ( -1 );

  // -- Warn user about being an alpha version --
  
  std::string file_to_view = "";
  Core::Application::Instance()->check_command_line_parameter( "file_to_open_on_start", file_to_view );

  std::string warning = std::string( "<h3>" ) +
    Core::Application::GetApplicationNameAndVersion() + 
    "</h3><p align=\"left\">NOTE: This version of Seg3D is for Testing and Evaluation"
    " Only!</p>";
  
  QMessageBox::information( 0, 
    QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ), 
    QString::fromStdString( warning )  );

#ifdef BUILD_WITH_PYTHON
  size_t name_len = strlen( argv[ 0 ] );
  std::vector< wchar_t > program_name( name_len + 1 );
  mbstowcs( &program_name[ 0 ], argv[ 0 ], name_len + 1 );
  Seg3D::PythonInterpreter::Instance()->initialize( &program_name[ 0 ] );
  Seg3D::PythonInterpreter::Instance()->start_terminal();
#endif

  // -- Setup Application Interface Window --
//  std::string file_to_view = "";
//  Core::Application::Instance()->check_command_line_parameter( "file_to_open_on_start", file_to_view );
  
  ApplicationInterface* app_interface = new ApplicationInterface( file_to_view );

  // Show the full interface
  app_interface->show();

  // Put the interface on top of all the other windows
  app_interface->raise();

  // The application window needs the qApplication as parent, which is
  // defined in the QtApplication, which integrates the Qt eventloop with
  // the interface eventloop of the Application layer.

  // -- Run QT event loop --
  if ( !( QtUtils::QtApplication::Instance()->exec() ) ) return ( -1 );

  // Finish the remainder of the actions that are still on the application thread.
  Core::Application::Instance()->finish();

  // Indicate a successful finish of the program
  Core::Application::Instance()->log_finish();
  return ( 0 );
}
