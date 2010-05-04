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

#include <iostream>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( Log );

Log::Log()
{
}

std::string Log::header( const int line, const char* file ) const
{
  boost::filesystem::path filename( file );
//  std::ostringstream oss;
//  oss << boost::this_thread::get_id();

  boost::posix_time::ptime timestamp( boost::posix_time::second_clock::local_time() );

  std::string header_string = std::string( "[" ) + 
    boost::posix_time::to_simple_string( timestamp ) + 
    std::string("|") + filename.filename() +  
    std::string( "|" ) + ToString( line ) + std::string( "]" );
  return header_string;
}

void Log::post_error( std::string message, const int line, const char* file )
{
  std::string str = header( line, file ) + std::string( " ERROR: " ) + message;
  post_log_signal_( LogMessageType::ERROR_E, str );
}

void Log::post_warning( std::string message, const int line, const char* file )
{
  std::string str = header( line, file ) + std::string( " WARNING: " ) + message;
  post_log_signal_( LogMessageType::WARNING_E, str );
}

void Log::post_message( std::string message, const int line, const char* file )
{
  std::string str = header( line, file ) + std::string( " MESSAGE: " ) + message;
  post_log_signal_( LogMessageType::MESSAGE_E, str );
}

void Log::post_debug( std::string message, const int line, const char* file )
{
  std::string str = header( line, file ) + std::string( " DEBUG: " ) + message;
  post_log_signal_( LogMessageType::DEBUG_E, str );
}

class LogStreamerInternal
{

public:
LogStreamerInternal(unsigned int log_flags, std::ostream* stream);

void stream_message(unsigned int type, std::string message);

private:
  unsigned int log_flags_;
  std::ostream* ostream_ptr_;
  boost::mutex stream_mutex_;
};

LogStreamerInternal::LogStreamerInternal(unsigned int log_flags, std::ostream* stream) :
log_flags_(log_flags),
ostream_ptr_(stream)
{
}

void
LogStreamerInternal::stream_message(unsigned int type, std::string message)
{
  boost::unique_lock<boost::mutex> lock(stream_mutex_);
  if (type & log_flags_) (*ostream_ptr_) << message << std::endl;
}

LogStreamer::LogStreamer(unsigned int log_flags, std::ostream* stream)
{
  // Use a shared pointer to register the internals of this class
  internal_ = boost::shared_ptr<LogStreamerInternal>(new LogStreamerInternal(log_flags,stream));

  // If the internals are detroyed, so should the connection:
  // we use the signals2 system to track the shared_ptr and destroy the connection
  // if the object is destroyed. This will ensure that when the slot is called
  // the shared_ptr is locked so that the object is not destroyed while the
  // call back is evaluated.
  Log::Instance()->post_log_signal_.connect(
    Log::post_log_signal_type::slot_type(&LogStreamerInternal::stream_message,
      internal_.get(),_1,_2).track(internal_));

  //   Log::Instance()->post_log_signal.connect(boost::bind(&LogStreamerInternal::stream_message,
  //      internal_.get(),_1,_2));
}

} // end namespace
