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

#ifndef CORE_UTILS_LOG_H
#define CORE_UTILS_LOG_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <Core/Utils/EnumClass.h>
#include <Core/Utils/Singleton.h>

// STL includes
#include <string>

// Boost includes
#include <boost/smart_ptr.hpp>
#include <boost/signals2.hpp>

namespace Core
{

// CLASS LOG:
/// This class records a log entry and pushes it onto a signal so more than
/// one recorder can record the log entries.

// Forward declaration
class Log;

// Please update LogTests if enums change
CORE_ENUM_CLASS
(
  LogMessageType,
  ERROR_E = 0x01, 
  WARNING_E = 0x02, 
  MESSAGE_E = 0x04, 
  DEBUG_E = 0x08, 
  SUCCESS_E = 0x10,
  CRITICAL_ERROR_E = 0x20,
  NODEBUG_E = ERROR_E | WARNING_E | MESSAGE_E | SUCCESS_E | CRITICAL_ERROR_E,
  ALL_E = ERROR_E | WARNING_E | MESSAGE_E | DEBUG_E | SUCCESS_E | CRITICAL_ERROR_E,
  STATUS_BAR_E = ERROR_E | WARNING_E | SUCCESS_E | CRITICAL_ERROR_E
 )

// Class definition
class Log : public boost::noncopyable
{
  CORE_SINGLETON( Log );
  // -- constructor / destructor --
private:
  Log();

  // -- functions for logging --
public:

  // POST_CRITICAL_ERROR:
  /// Post an error onto the log signal !!THIS WILL CAUSE AN ERROR DIALOG TO DISPLAY FOR THE USER!!
  void post_critical_error( std::string message, const int line, const char* file );

  // POST_ERROR:
  /// Post an error onto the log signal
  void post_error( std::string message, const int line, const char* file );

  // POST_WARNING:
  /// Post a warning onto the log signal
  void post_warning( std::string message, const int line, const char* file );

  // POST_MESSAGE:
  /// Post a message onto the log signal
  void post_message( std::string message, const int line, const char* file );

  // POST_SUCCESS:
  /// Post a message onto the log signal
  void post_success( std::string message, const int line, const char* file );

  // POST_DEBUG:
  /// Post debug information onto the log signal
  void post_debug( std::string message, const int line, const char* file );

private:
  // HEADER:
  /// Generate a uniform header for the message that is posted
  std::string header( const int line, const char* file ) const;

  // -- signal where to receive the logging information from --
public:
  typedef boost::signals2::signal< void( unsigned int, std::string ) > post_log_signal_type;

  // POST_LOG_SIGNAL
  /// Signal indicating that a message needs to be written to the log file
  post_log_signal_type post_log_signal_;

  // POST_STATUS_SIGNAL
  /// Signal indicating that a message needs to be written to the status bar
  post_log_signal_type post_status_signal_;
  
  // POST_CRITICAL_SIGNAL
  /// Signal indicating that a message needs to be written to the status bar
  post_log_signal_type post_critical_signal_;

};

// MACROS FOR AUTOMATICALLY INCLUDING LINE NUMBER AND FILE IN THE
// LOG FILE 

#define CORE_LOG_CRITICAL_ERROR(message)\
  Core::Log::Instance()->post_critical_error(message,__LINE__,__FILE__)

#define CORE_LOG_ERROR(message)\
Core::Log::Instance()->post_error(message,__LINE__,__FILE__)

#define CORE_PRINT_AND_LOG_ERROR(message)\
std::cerr << "ERROR: " << message << std::endl; \
Core::Log::Instance()->post_error(message,__LINE__,__FILE__)


#define CORE_LOG_WARNING(message)\
Core::Log::Instance()->post_warning(message,__LINE__,__FILE__)

#define CORE_LOG_MESSAGE(message)\
Core::Log::Instance()->post_message(message,__LINE__,__FILE__)

#define CORE_LOG_SUCCESS(message)\
  Core::Log::Instance()->post_success(message,__LINE__,__FILE__)

//TODO: remove after upgrade to VS2017 
#ifndef NDEBUG
#ifdef BUILD_STANDALONE_LIBRARY
#error "Only build standalone library in Release mode."
#endif
#endif

#ifdef NDEBUG
#define CORE_LOG_DEBUG(message)
#else
#define CORE_LOG_DEBUG(message)\
Core::Log::Instance()->post_debug(message,__LINE__,__FILE__)
#endif

} // end namespace Core

#endif
