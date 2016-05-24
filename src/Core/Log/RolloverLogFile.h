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

#ifndef CORE_LOG_ROLLOVERLOGFILE_H
#define CORE_LOG_ROLLOVERLOGFILE_H

// Boost includes
#include <boost/shared_ptr.hpp>

namespace Core
{
// Internals are separated from the interface
class RolloverLogFilePrivate;
typedef boost::shared_ptr< RolloverLogFilePrivate > RolloverLogFilePrivateHandle;

// CLASS ROLLOVERLOGFILE:
/// Class for writing log messages to a log file that gets rolled over based on file size, number of 
/// files, and age. Creates log file in config directory.  Log file name includes application name, 
/// version, and timestamp.  Logs messages in response to Log::Instance()->post_log_signal_.
class RolloverLogFile 
{
public:
  /// Create log file, hook up to post_log_signal
  RolloverLogFile( unsigned int log_flags );

  // SET_MAX_FILES:
  /// Set the maximum number of rollover log files
  void set_max_files( int num_files );
  
  // SET_MAX_LINES:
  /// Set the maximum number of lines per event log file
  void set_max_lines( int num_lines );

  // SET_MAX_AGE:
  /// Set the maximum age of a log file.  Log files older than this will be deleted. 
  void set_max_age( int num_days );

private:
  RolloverLogFilePrivateHandle private_;
};

} // end namespace Core

#endif
