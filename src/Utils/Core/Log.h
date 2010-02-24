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

#ifndef UTILS_CORE_LOG_H
#define UTILS_CORE_LOG_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <Utils/Singleton/Singleton.h>

// STL includes
#include <string>

// Boost includes
#include <boost/smart_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

namespace Utils {

// CLASS LOG:
// This class records a log entry and pushes it onto a signal so more than
// one recorder can record the log entries.


// Forward declaration
class Log;

// Class definition
class Log : public boost::noncopyable {

// -- constructor / destructor --
  private:
    friend class Singleton<Log>;
    Log();

// -- Message types supported --
  public:
    enum { ERROR_E = 0x01,
           WARNING_E = 0x02,
           MESSAGE_E = 0x04,
           DEBUG_E = 0x08,
           ALL_E = ERROR_E|WARNING_E|MESSAGE_E|DEBUG_E };

// -- functions for logging --  
  public:

    // POST_ERROR:
    // Post an error onto the log signal
    
    void post_error(std::string message, const int line, const char* file);

    // POST_WARNING:
    // Post a warning onto the log signal
    
    void post_warning(std::string message, const int line, const char* file);

    // POST_MESSAGE:
    // Post a message onto the log signal

    void post_message(std::string message, const int line, const char* file);

    // POST_DEBUG:
    // Post debug information onto the log signal

    void post_debug(std::string message, const int line, const char* file);

  private:
    // HEADER:
    // Generate a uniform header for the message that is posted
    std::string header(const int line, const char* file) const;

// -- signal where to receive the logging information from --
  public:
    typedef boost::signals2::signal<void (unsigned int, std::string)> post_log_signal_type;
  
    // POST_LOG_SIGNAL
    // Signal indicating that the history changed

    post_log_signal_type post_log_signal_;

// -- Singleton interface --
  public:
    
    static Log* Instance() { return instance_.instance(); }
    static Singleton<Log> instance_;
    
};

// MACROS FOR AUTOMATICALLY INCLUDING LINE NUMBER AND FILE IN THE
// LOG FILE 

#define SCI_LOG_ERROR(message)\
Utils::Log::Instance()->post_error(message,__LINE__,__FILE__)

#define SCI_LOG_WARNING(message)\
Utils::Log::Instance()->post_warning(message,__LINE__,__FILE__)

#define SCI_LOG_MESSAGE(message)\
Utils::Log::Instance()->post_message(message,__LINE__,__FILE__)

#ifdef NDEBUG
#define SCI_LOG_DEBUG(message)
#else
#define SCI_LOG_DEBUG(message)\
Utils::Log::Instance()->post_debug(message,__LINE__,__FILE__)
#endif

// CLASS LOGSTREAMER:
// Class for streaming log messages to standard output channels

// Internals are separated from the interface
class LogStreamerInternal;

class LogStreamer {
  
  public:
    LogStreamer(unsigned int log_flags, std::ostream* stream);

  private:
    boost::shared_ptr<LogStreamerInternal> internal_;

};

} // end namespace Utils

#endif
