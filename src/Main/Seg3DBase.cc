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
#pragma warning( disable: 4244 )
#endif


#ifdef BUILD_WITH_PYTHON
#include <Python.h>
#include <Core/Python/PythonInterpreter.h>
#include <Core/Python/PythonCLI.h>
#include <Application/Socket/ActionSocket.h>

#include "ActionPythonWrapperRegistration.h"
#endif

// STL includes
#include <iostream>
#include <string>

// boost includes
#include <boost/preprocessor.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/LogStreamer.h>
#include <Core/Utils/LogHistory.h>
#include <Core/Interface/Interface.h>
#include <Core/Action/ActionHistory.h>
#include <Core/Log/RolloverLogFile.h>

// Application includes
#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/Tool/ToolFactory.h>

// Resource includes
#include <Resources/QtResources.h>

// File that contains a function that registers all the class that need registration,
// such as Actions and Tools
#include "ClassRegistration.h"

// Revision information
#include "Seg3D_RevisionInfo.h"

#include "Seg3DBase.h"

#if defined (_WIN32)
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <fstream>
// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;
void RedirectIOToConsole()
{
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);
}
#endif

namespace Seg3D
{

Seg3DBase::Seg3DBase() :
  revision(Core::Application::Instance()->is_command_line_parameter( "revision")),
  version(Core::Application::Instance()->is_command_line_parameter( "version")),
  help(Core::Application::Instance()->is_command_line_parameter( "help")),
  display_splash_screen(!Core::Application::Instance()->is_command_line_parameter( "nosplash")),
  start_sockets(false)
{
  boost::filesystem::path path;
  Core::Application::Instance()->get_application_filename(path);
  this->program_name = path.string();
  Core::Application::Instance()->check_command_line_parameter( "file_to_open_on_start", this->file_to_view );
  Core::Application::Instance()->check_command_line_parameter( "python", this->python_script );

  std::string port_number_string = "";
  if (!Core::Application::Instance()->check_command_line_parameter( "socket", port_number_string ))
    Core::Application::Instance()->check_command_line_parameter( "port", port_number_string );
  if (port_number_string != "")
  {
    start_sockets = Core::ImportFromString( port_number_string, this->port_number);
  }
}

Seg3DBase::~Seg3DBase()
{}

bool Seg3DBase::information_only()
{
  bool information_param_requested = false;
  // -- Parse the command line parameters --

  // -- Check whether the user requested a version / revision number
  if ( Core::Application::Instance()->is_command_line_parameter( "revision") )
  {
    // NOTE: The revision information is gathered by cmake from git. Hence if the local tree
    // contains modifications this information is not up-to-date
    std::cout << GIT_SEG3D_REVISIONINFO << std::endl;
    information_param_requested = true;
  }

  if ( Core::Application::Instance()->is_command_line_parameter( "version") )
  {
    // NOTE: This information is gathered by cmake from the main cmake file.
    std::cout << Core::Application::Instance()->GetApplicationName() << " version: " <<
      Core::Application::Instance()->GetVersion() << std::endl;
    information_param_requested = true;
  }

  if ( Core::Application::Instance()->is_command_line_parameter( "help") )
  {
    this->print_usage();
    information_param_requested = true;
  }

  return information_param_requested;
}

bool Seg3DBase::initialize()
{
  // -- Send message to revolving log file --
  // Logs messages in response to Log::Instance()->post_log_signal_
  Core::RolloverLogFile event_log( Core::LogMessageType::ALL_E );

#ifndef NDEBUG
#if defined (_WIN32)
  RedirectIOToConsole();
#endif
  // -- Stream errors to console window
  new Core::LogStreamer( Core::LogMessageType::ALL_E, &std::cerr );
#endif

  Core::Application::Instance()->log_start();
  Core::RegisterClasses();
  Core::Application::Instance()->start_eventhandler();
  ToolFactory::Instance()->initialize_states();
  Core::Application::Instance()->application_start_signal_();

  this->check_32_bit();
  this->initialize_sockets();
  this->initialize_python();

  return true;
}

void Seg3DBase::close()
{
  // Trigger the application stop signal
  CORE_LOG_MESSAGE( std::string("sending application stop signal") );
  Core::Application::Instance()->application_stop_signal_();


  // Finish the remainder of the actions that are still on the application thread.
  CORE_LOG_MESSAGE( std::string("finishing application") );
  Core::Application::Instance()->finish();

  // Indicate a successful finish of the program
  CORE_LOG_MESSAGE( std::string("finishing log, then exit") );
  Core::Application::Instance()->log_finish();
}

void Seg3DBase::print_usage()
{
  std::cout << "USAGE: " << Core::Application::GetApplicationName() << std::endl;
  std::cout << "Optional parameters:" << std::endl;
  std::cout << "  --revision              - Get Git revision information." << std::endl;
  std::cout << "  --version               - Version number." << std::endl;
  std::cout << "  --socket=SCALAR         - Open a socket on the given port number." << std::endl;
  std::cout << "  --python=FILE           - Run the python script in the given file." << std::endl;
  std::cout << "  --nosplash              - Run without opening the splash screen." << std::endl;
  std::cout << "  --headless              - Run without opening the GUI." << std::endl;
  std::cout << "  --logging=FILE          - Log to the specified file." << std::endl;
  std::cout << "  --help                  - Print this usage message." << std::endl << std::endl;
}

void Seg3DBase::check_32_bit()
{
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

    this->warning(warning);
  }
}

void Seg3DBase::initialize_python()
{
#ifdef BUILD_WITH_PYTHON
  std::wstring program_name(this->program_name.begin(), this->program_name.end());

  Core::PythonInterpreter::module_list_type python_modules;
  std::string module_name = Core::StringToLower( BOOST_PP_STRINGIZE( APPLICATION_NAME ) );
  python_modules.push_back( Core::PythonInterpreter::module_entry_type( module_name,
    BOOST_PP_CAT( PyInit_, APPLICATION_NAME ) ) );
  Core::PythonInterpreter::Instance()->initialize( program_name.c_str(), python_modules );
  Core::PythonInterpreter::Instance()->run_string( "import " + module_name + "\n" );
  Core::PythonInterpreter::Instance()->run_string( "from " + module_name + " import *\n" );

  if ( this->python_script != "" )
  {
    Core::PythonCLI::Instance()->execute_file(this->python_script);
    this->display_splash_screen = false;
  }
#endif
}

void Seg3DBase::initialize_sockets()
{
  if (!this->start_sockets)
    return;
#ifdef BUILD_WITH_PYTHON
  // -- Add a socket for receiving actions --
    CORE_LOG_MESSAGE( std::string("Starting a socket on port: ") + Core::ExportToString( this->port_number ) );
    Seg3D::ActionSocket::Instance()->start( this->port_number );
    InterfaceManager::Instance()->set_initializing( true );
    InterfaceManager::Instance()->splash_screen_visibility_state_->set( false );
    InterfaceManager::Instance()->set_initializing( false );
#else
  std::string warning("<h6><p align=\"justify\">Opening a socket is not supported without Python.</p></h6>");
  this->warning(warning);
#endif
}

} //namespace Seg3D
