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

#ifndef CORE_UTILS_LOGHISTORY_H
#define CORE_UTILS_LOGHISTORY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <deque>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

// Core includes
#include <Core/Utils/Singleton.h>

namespace Core
{

// CLASS LOGHISTORY:
/// This class records the last entries of the log and keeps a record of the
/// last entries.

// Forward declaration
class LogHistory;

// Class definition
class LogHistory : public Singleton< LogHistory >
{

public:
  typedef std::pair< int, std::string > log_entry_type;
  typedef std::deque< log_entry_type > log_history_type;

  // -- constructor / destructor --
private:
  friend class Singleton< LogHistory > ;
  LogHistory();

  // -- History recording --

public:
  // SET_MAX_HISTORY_SIZE:
  /// Set the size of the action history buffer
  void set_max_history_size( size_t size );

  // MAX_HISTORY_SIZE:
  /// Retrieve the size of this buffer
  size_t max_history_size();

  // HISTORY_SIZE:
  size_t history_size();

  // LOG_ENTRY
  /// Get the nth log entry in the buffer
  log_entry_type log_entry( size_t index );

private:
  boost::mutex log_history_mutex_;
  log_history_type log_history_;
  size_t log_history_max_size_;

  boost::signals2::connection log_connection_;

  void record_log( int type, std::string message );

  // -- History changed signal --
public:
  typedef boost::signals2::signal< void() > history_changed_signal_type;

  // HISTORY_CHANGED_SIGNAL:
  /// Signal indicating that the history changed
  history_changed_signal_type history_changed_signal_;

};

} // end namespace Core

#endif
