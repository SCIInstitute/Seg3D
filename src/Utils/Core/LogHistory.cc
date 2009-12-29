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

#include <Utils/Core/LogHistory.h>

namespace Utils {

LogHistory::LogHistory() :
  log_history_max_size_(0)
{
  // Connect this class to the ActionDispatcher
  log_connection_ = Log::Instance()->post_log_signal.connect(
    boost::bind(&LogHistory::record_log,this,_1,_2));
}

void
LogHistory::set_max_history_size(size_t size)
{
  boost::unique_lock<boost::mutex> lock(log_history_mutex_);
  log_history_max_size_ = size;
  if (log_history_.size() > log_history_max_size_)
  {
    // remove part of the history
    log_history_.erase(log_history_.begin()+size,log_history_.end());
    // signal that buffer has changed
    history_changed_signal(); 
  }
}

size_t
LogHistory::max_history_size()
{
  boost::unique_lock<boost::mutex> lock(log_history_mutex_);
  return (log_history_max_size_ );
}

size_t
LogHistory::history_size()
{
  boost::unique_lock<boost::mutex> lock(log_history_mutex_);
  return (log_history_.size() );
}

LogHistory::log_entry_type
LogHistory::log_entry(size_t index)
{
  boost::unique_lock<boost::mutex> lock(log_history_mutex_);
  if (index < log_history_.size())
  {
    return log_history_[index];
  }
  else
  {
    log_entry_type empty;
    return empty;
  }
}

void 
LogHistory::record_log(int type,std::string message)
{
  boost::unique_lock<boost::mutex> lock(log_history_mutex_);
  log_entry_type entry = std::make_pair(type,message);
  
  log_history_.push_front(entry);
  if (log_history_.size() > log_history_max_size_)
  {
    log_history_.pop_back();
  }
  
  history_changed_signal();
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<LogHistory> LogHistory::instance_;

} // end namespace Utils

