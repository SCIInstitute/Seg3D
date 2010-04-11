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

#ifndef APPLICATION_APPLICATION_APPLICATION_H
#define APPLICATION_APPLICATION_APPLICATION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Utils includes
#include <Utils/Core/Log.h>
#include <Utils/Core/Singleton.h>
#include <Utils/EventHandler/EventHandler.h>

//Qt include
#include <QtGui>

namespace Seg3D
{

// CLASS APPLICATION:
// Application is the thread that processes all the actions in the program.

class Application;

class Application : public Utils::EventHandler
{
  CORE_SINGLETON( Application );

  // -- Constructor/Destructor --
private:
  Application();
  virtual ~Application();

  // -- Application wide settings --
public:

  // NUMBER_OF_VIEWERS
  // The number of viewers that is available in the application
  // This is a preset number to simplify the dependency scheme
  size_t number_of_viewers()
  {
    return 6;
  }

  // -- Command line parser --
public:
  // CHECK_COMMAND_LINE_PARAMETERS
  // check to see if a particular parameter has been placed into the map
  // if so it returns the value as a string
  bool check_command_line_parameter( const std::string& key, std::string& value );

  // SET_PARAMETER
  // put parameters from the command line into a map
  void set_command_line_parameter( const std::string& key, const std::string& value );

  // PARSE_COMMAND_LINE_PARAMETERS
  // parse paremeters from the command line
  void parse_command_line_parameters( int argc, char** argv );

private:
  typedef std::map< std::string, std::string > parameters_type;
  parameters_type parameters_;

  // -- Thread safety --
public:
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;

  // GET_MUTEX:
  // Get the mutex that protects this class
  mutex_type& get_mutex() { return mutex_; }

private:
  // Lock for this lock
  mutex_type mutex_;

  // -- Application thread --
public:
  // ISAPPLICATIONTHREAD:
  // Test whether the current thread is the application thread
  static bool IsApplicationThread()
  {
    return ( Instance()->is_eventhandler_thread() );
  }

  // POSTEVENT:
  // Short cut to the event handler
  static void PostEvent( boost::function< void() > function )
  {
    Instance()->post_event( function );
  }

  // POSTANDWAITEVENT:
  // Short cut to the event handler
  static void PostAndWaitEvent( boost::function< void() > function )
  {
    Instance()->post_and_wait_event( function );
  }

};

} // end namespace Seg3D

#endif
