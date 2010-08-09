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

#ifndef CORE_APPLICATION_APPLICATION_H
#define CORE_APPLICATION_APPLICATION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <map>

// Boost includes
#include <boost/filesystem/path.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>
#include <Core/EventHandler/EventHandler.h>

namespace Core
{

// CLASS APPLICATION:
// Application is the thread that processes all the actions in the program.

class Application;

class Application : public EventHandler, public Lockable
{
  CORE_SINGLETON( Application );

  // -- Constructor/Destructor --
private:
  Application();
  virtual ~Application();

  // -- Command line parser --
public:
  // IS_COMMAND_LINE_PARAMETERS:
  bool is_command_line_parameter( const std::string& key );

  // CHECK_COMMAND_LINE_PARAMETERS:
  // check to see if a particular parameter has been placed into the map
  // if so it returns the value as a string
  bool check_command_line_parameter( const std::string& key, std::string& value );

  // SET_PARAMETER:
  // put parameters from the command line into a map
  void set_command_line_parameter( const std::string& key, const std::string& value );

  // PARSE_COMMAND_LINE_PARAMETERS:
  // parse paremeters from the command line
  void parse_command_line_parameters( int argc, char** argv );

private:
  typedef std::map< std::string, std::string > parameters_type;
  parameters_type parameters_;

  // -- Log information on the current executable --
public:
  // LOG_START:
  // Log information about the system to the log file
  void log_start();

  // LOG_FINISH:
  // Log an end tag mentioning that the program has finished
  void log_finish();

  // -- Directory information --
public:
  // GET_USER_DIRECTORY:
  // Get the user directory on the current system 
  bool get_user_directory( boost::filesystem::path& user_dir, bool config_path = false );

  // GET_CONFIG_DIRECTORY:
  // Get the configuration directory on the current system  
  bool get_config_directory( boost::filesystem::path& config_dir );

  // GET_USER_DESKTOP_DIRECTORY:
  // get the path of the users desktop directory
  bool get_user_desktop_directory( boost::filesystem::path& user_desktop_dir );

  // GET_USER_NAME:
  // Get the current username
  bool get_user_name( std::string& user_name );

  // -- Application thread --
public:
  // ISAPPLICATIONTHREAD:
  // Test whether the current thread is the application thread
  static bool IsApplicationThread();

  // POSTEVENT:
  // Short cut to the event handler
  static void PostEvent( boost::function< void() > function );

  // POSTANDWAITEVENT:
  // Short cut to the event handler
  static void PostAndWaitEvent( boost::function< void() > function );

  // -- Program information --
public: 
  
  // GETVERSION:
  // Get the application version
  static std::string GetVersion();
  
  // GETMAJORVERSION:
  // Major release version
  static int GetMajorVersion();

  // GETMINORVERSION:
  // Minor release version
  static int GetMinorVersion();

  // GETPATCHVERSION:
  // Patch version
  static int GetPatchVersion();
  
  // IS64BIT:
  // Is the executable a 64bit version
  static bool Is64Bit();
  
  // IS32BIT:
  // Is the executable a 64bit version
  static bool Is32Bit();
  
  // GETAPPLICATIONNAME:
  // Get the name of the application
  static std::string GetApplicationName();
};

#define ASSERT_ON_APPLICATION_THREAD()\
  assert( Core::Application::IsApplicationThread() )

} // end namespace Core

#define ASSERT_IS_APPLICATION_THREAD() assert( Core::Application::IsApplicationThread() )

#endif
