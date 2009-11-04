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

#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>

#include <boost/bind.hpp>

namespace Core {

void
Log::post_error(std::string message, const int line, const char* file)
{
  std::string str = std::string("[")+file+std::string(":")+to_string(line)+
    std::string("] ERROR: ")+message;
  
  boost::unique_lock<boost::mutex> lock(log_mutex_);
  post_log_signal_(ERROR_E, str);
}  

void
Log::post_warning(std::string message, const int line, const char* file)
{
  std::string str = std::string("[")+file+std::string(":")+to_string(line)+
    std::string("] WARNING: ")+message;
  
  boost::unique_lock<boost::mutex> lock(log_mutex_);
  post_log_signal_(WARNING_E, str);
}  

void
Log::post_message(std::string message, const int line, const char* file)
{
  std::string str = std::string("[")+file+std::string(":")+to_string(line)+
    std::string("] MESSAGE: ")+message;
  
  boost::unique_lock<boost::mutex> lock(log_mutex_);
  post_log_signal_(MESSAGE_E, str);
}  

void
Log::post_debug(std::string message, const int line, const char* file)
{
  std::string str = std::string("[")+file+std::string(":")+to_string(line)+
    std::string("] DEBUG: ")+message;
  
  boost::unique_lock<boost::mutex> lock(log_mutex_);
  post_log_signal_(DEBUG_E, str);
}  

Log*
Log::instance()
{
  // if no singleton was allocated, allocate it now
  if (!initialized_)   
  {
    //in case multiple threads try to allocate this one at once.
    {
      boost::unique_lock<boost::mutex> lock(log_mutex_);
      // The first test was not locked and hence not thread safe
      // This one will do a thread-safe allocation of the interface
      // class
      if (log_ == 0) log_ = new Log();
    }
    
    {
      // Enforce memory synchronization so the singleton is initialized
      // before we set initialized to true
      boost::unique_lock<boost::mutex> lock(log_mutex_);
      initialized_ = true;
    }
  }
  return (log_);
}

// Static variables that are located in Application and that need to be
// allocated here
boost::mutex Log::log_mutex_;
bool         Log::initialized_ = false;
Log*         Log::log_ = 0;


class LogStreamerInternal {

  public:
    LogStreamerInternal(unsigned int log_flags, std::ostream* stream);

    void stream_message_slot(unsigned int type, std::string message);
    
  private:
    unsigned int  log_flags_;
    std::ostream* ostream_ptr_;
};


LogStreamerInternal::LogStreamerInternal(unsigned int log_flags, std::ostream* stream) :
  log_flags_(log_flags),
  ostream_ptr_(stream)
{
}

void
LogStreamerInternal::stream_message_slot(unsigned int type, std::string message)
{
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
  Log::instance()->post_log_signal_.connect(
    Log::post_log_signal_type::slot_type(&LogStreamerInternal::stream_message_slot, 
      internal_.get(),_1,_2).track(internal_));
}


} // end namespace
