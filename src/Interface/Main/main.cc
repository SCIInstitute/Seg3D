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

// Include CMake generated files
#include <Seg3DConfiguration.h>

// Core includes
#include <Utils/Core/Log.h>
#include <Utils/Core/LogHistory.h>
#include <Utils/Core/Environment.h>

// Application includes
#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>
#include <Application/Action/ActionHistory.h>
#include <Application/Action/ActionSocket.h>

// Interface includes
#include <Interface/QtInterface/QtApplication.h>
#include <Interface/AppInterface/AppInterface.h>

// Init Plugins functionality
#include <Init/Init.h>
#include <Init/QtInit.h>

//#include <Interface/ControllerInterface/ControllerInterface.h>

///////////////////////////////////////////////////////////
// Main Seg3D entry point
///////////////////////////////////////////////////////////

using namespace Seg3D;

int main(int argc, char **argv)
{
  

  
  // -- Setup error logging --
  // stream error to the console window
  Utils::LogStreamer error_log(Utils::Log::ALL_E,&(std::cerr));

  // stream errors to a history log that maintains the last 1000 entries
  Utils::LogHistory::Instance()->set_max_history_size(1000);

  // -- Startup Seg3D --
  SCI_LOG_MESSAGE(std::string("--- Starting Seg3D ")+SEG3D_VERSION+" "+
                  SEG3D_BITS+" "+SEG3D_DEBUG_VERSION+" ---");

  // -- Setup action history --
  SCI_LOG_DEBUG("Setup action history");
  Seg3D::ActionHistory::Instance()->set_max_history_size(300);
  
  
  ///////////  TEST CODE FOR COMMAND LINE PARAMETERS ////////  
  int socketNumber = 3000;
  
//  if(std::string(argv[1]) == "-s")
//  {
//  }
  ///////////  END TEST CODE FOR COMMAND LINE PARAMETERS ////

  // -- Add a socket for receiving actions --
  //ActionSocket::Instance()->start(3000);
  SCI_LOG_DEBUG("Starting the socket");
  ActionSocket::Instance()->start(socketNumber);
  
  // -- Add plugins into the architecture  
  SCI_LOG_DEBUG("Setup and register all the plugins");
  Seg3D::InitPlugins();
  Seg3D::QtInitResources();
  
  // -- Setup the QT Interface Layer --
  SCI_LOG_DEBUG("Setup QT Interface");
  if (!(QtApplication::Instance()->setup(argc,argv))) 
  {
    SCI_LOG_ERROR("Could not setup QT Interface");  
    return (-1);
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
    if (!(QtApplication::Instance()->exec()))
    {
      SCI_LOG_ERROR("The interface thread crashed, exiting Seg3D");  
      return (-1);
    }    
    
  }
  catch(Utils::Exception& except)
  {  
    // Catch any Seg3D generated exceptions and display there message in the log file
    SCI_LOG_ERROR(std::string("Setup of the interface crashed by throwing an exception: ") + except.message());
    return(-1);
  }
  catch(std::exception& except)
  {
    // For any other exception
    SCI_LOG_ERROR(std::string("Setup of the interface crashed by throwing an exception: ") + except.what());
    return(-1);
  }
  catch(...)
  {
    // For any other exception
    SCI_LOG_ERROR(std::string("Setup of the interface crashed by throwing an unknown exception"));
    return(-1);
  }
  
  // Indicate a successful finish of the program
  SCI_LOG_MESSAGE("--- Finished ---");
  return (0);
}

// Function for parsing the command line parameters -- TEST ONLY
int
parse_command_line_parameters( int argc, char **argv)
{
  
  return 0;
}
