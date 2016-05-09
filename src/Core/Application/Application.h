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

#ifndef CORE_APPLICATION_APPLICATION_H
#define CORE_APPLICATION_APPLICATION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Boost includes
#include <boost/filesystem/path.hpp>
#include <boost/signals2.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>
#include <Core/EventHandler/EventHandler.h>

namespace Core
{

// CLASS APPLICATION:
/// Application is the thread that processes all the actions in the program.

class Application;
class ApplicationPrivate;
typedef boost::shared_ptr<ApplicationPrivate> ApplicationPrivateHandle;


class Application : public EventHandler, public RecursiveLockable
{
  CORE_SINGLETON( Application );

  // -- Constructor/Destructor --
private:
  Application();
  virtual ~Application();

  // -- finish application --
public:
  // FINISH:
  /// Execute the remainder of the actions and terminate the application thread
  /// NOTE: This function should be called by main at the end of the program to ensure
  /// that actions like saving the last session are properly executed.
  void finish();

  // RESET:
  /// Reset the application. It triggers the reset_signal_.
  /// NOTE: This function should only be called in the application thread.
  void reset();



  // -- Command line parser --
public:
  // IS_COMMAND_LINE_PARAMETERS:
  bool is_command_line_parameter( const std::string& key );

  // CHECK_COMMAND_LINE_PARAMETERS:
  /// check to see if a particular parameter has been placed into the map
  /// if so it returns the value as a string
  bool check_command_line_parameter( const std::string& key, std::string& value );

    // GET_ARGUMANT:
    /// Get the argument (required parameter)
    std::string get_argument(int idx);

  // SET_PARAMETER:
  /// put parameters from the command line into a map
  void set_command_line_parameter( const std::string& key, const std::string& value );

  // PARSE_COMMAND_LINE_PARAMETERS:
  /// parse parameters from the command line
  void parse_command_line_parameters( int argc, char** argv, int num_arguments = 0 );

private:
  typedef std::map< std::string, std::string > parameters_type;
  parameters_type parameters_;
    
    typedef std::vector<std::string> arguments_type;
    arguments_type arguments_;

  // -- Log information on the current executable --
public:
  // LOG_START:
  /// Log information about the system to the log file
  void log_start();

  // LOG_FINISH:
  // Log an end tag mentioning that the program has finished successfully
  void log_finish();

  // -- Directory information --
public:
  // GET_USER_DIRECTORY:
  /// Get the user directory on the current system  
  bool get_user_directory( boost::filesystem::path& user_dir, bool config_path = false );

  // GET_CONFIG_DIRECTORY:
  /// Get the configuration directory on the current system 
  bool get_config_directory( boost::filesystem::path& config_dir );

  // GET_USER_DESKTOP_DIRECTORY:
  /// get the path of the users desktop directory
  bool get_user_desktop_directory( boost::filesystem::path& user_desktop_dir );

  // GET_USER_NAME:
  /// Get the current username
  bool get_user_name( std::string& user_name );

  // GET_APPLICACTION_FILEPATH:
  /// The directory from which the application was launched
  bool get_application_filepath( boost::filesystem::path& app_filepath );
  
  // GET_APPLICACTION_FILENAME:
  /// The directory from which the application was launched plus the name of the executable
  bool get_application_filename( boost::filesystem::path& app_filename );

  // -- Memory information --
public:
  // GET_TOTAL_VIRTUAL_MEMORY:
  /// Get the total amount of virtual memory available
  long long get_total_virtual_memory();

  // GET_TOTAL_PHYSICAL_MEMORY:
  /// Get the total amount of physical memory available
  long long get_total_physical_memory();
  
  // GET_TOTAL_ADDRESSABLE_MEMORY:
  /// Get the amount of addressable memory available
  long long get_total_addressable_memory();

  // GET_TOTAL_ADDRESSABLE_PHYSICAL_MEMORY:
  /// Get the amount of addressable memory available inside RAM
  long long get_total_addressable_physical_memory();
  
  // GET_MY_VIRTUAL_MEMORY_USED:
  /// Get the amount of virtual memory used by current process
  long long get_my_virtual_memory_used();
  
  // GET_MY_PHYSICAL_MEMORY_USED:
  /// Get the amount of physical memory used by current process
  long long get_my_physical_memory_used();
  
  // -- Process information --
public:
  // GET_PROCESS_ID:
  /// Get the process id for the current process
  int get_process_id();

  // -- OSX information --
public:
  bool is_osx_10_5_or_less();

  // -- Signals --
public:
  // RESET_SIGNAL
  /// This signal is triggered by calling the reset function.
  /// WARNING: Do NOT trigger this signal directly. Call the reset function instead.
  boost::signals2::signal< void () > reset_signal_;

  // APPLICATION_START_SIGNAL
  /// This signal is triggered at the start of the application, before the splash
  /// screen is shown. This signal can be used to initialize code from plugins
  boost::signals2::signal< void () > application_start_signal_;

  // APPLICATION_STOP_SIGNAL
  /// This signal is triggered at the end of the program
  boost::signals2::signal< void () > application_stop_signal_;

  // -- internals --
private:
  // internals of this class
  ApplicationPrivateHandle private_;

  // -- Application thread --
public:
  // ISAPPLICATIONTHREAD:
  /// Test whether the current thread is the application thread
  static bool IsApplicationThread();

  // POSTEVENT:
  /// Short cut to the event handler
  static void PostEvent( boost::function< void() > function );

  // POSTANDWAITEVENT:
  /// Short cut to the event handler
  static void PostAndWaitEvent( boost::function< void() > function );

  // GETMUTEX:
  /// Get the mutex of the application.
  static mutex_type& GetMutex();

  // RESET:
  /// Reset the application.
  static void Reset();

  // -- Program information --
public: 
  
  // GETVERSION:
  /// Get the application version
  static std::string GetVersion();
  
  // GETMAJORVERSION:
  /// Major release version
  static int GetMajorVersion();

  // GETMINORVERSION:
  /// Minor release version
  static int GetMinorVersion();

  // GETPATCHVERSION:
  /// Patch version
  static int GetPatchVersion();
  
  // IS64BIT:
  /// Is the executable a 64bit version
  static bool Is64Bit();
  
  // IS32BIT:
  /// Is the executable a 64bit version
  static bool Is32Bit();
  
  // GETAPPLICATIONNAME:
  /// Get the name of the application
  static std::string GetApplicationName();
  
  // GETAPPLICATIONNAMEANDVERSION
  /// Get the name of the application and its version
  static std::string GetApplicationNameAndVersion();

  // GETABOUT
  /// Get the information that should be shown in the about screen
  static std::string GetAbout();
  
  // GETUTILNAME:
  /// Get the name of the utility
  static std::string GetUtilName();

  // SETUTILNAME:
  /// Set the name of the utility
  static void SetUtilName(const std::string& name);

};

} // end namespace Core

#define ASSERT_IS_APPLICATION_THREAD() assert( Core::Application::IsApplicationThread() )

#endif
