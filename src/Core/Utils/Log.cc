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

#include <iostream>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
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
    std::string( "|" ) + filename.filename().string() +  
    std::string( "|" ) + ExportToString( line ) + std::string( "]" );
  return header_string;
}

void Log::post_critical_error( std::string message, const int line, const char* file )
{
  std::string str = this->header( line, file ) + std::string( " ERROR: " ) + message;
  post_log_signal_( LogMessageType::ERROR_E, str );
  post_status_signal_( LogMessageType::ERROR_E, message );
  post_critical_signal_( LogMessageType::CRITICAL_ERROR_E, message );
}

void Log::post_error( std::string message, const int line, const char* file )
{
  std::string str = this->header( line, file ) + std::string( " ERROR: " ) + message;
  post_log_signal_( LogMessageType::ERROR_E, str );
  post_status_signal_( LogMessageType::ERROR_E, message );
}

void Log::post_warning( std::string message, const int line, const char* file )
{
  std::string str = this->header( line, file ) + std::string( " WARNING: " ) + message;
  post_log_signal_( LogMessageType::WARNING_E, str );
  this->post_status_signal_( LogMessageType::WARNING_E, message );
}

void Log::post_message( std::string message, const int line, const char* file )
{
  std::string str = this->header( line, file ) + std::string( " MESSAGE: " ) + message;
  post_log_signal_( LogMessageType::MESSAGE_E, str );
}

void Log::post_success( std::string message, const int line, const char* file )
{
  std::string str = this->header( line, file ) + std::string( " SUCCESS: " ) + message;
  post_log_signal_( LogMessageType::SUCCESS_E, str );
  post_status_signal_( LogMessageType::SUCCESS_E, message );
}

void Log::post_debug( std::string message, const int line, const char* file )
{
  std::string str = this->header( line, file ) + std::string( " DEBUG: " ) + message;
  post_log_signal_( LogMessageType::DEBUG_E, str );
}

} // end namespace
