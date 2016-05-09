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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/LogStreamer.h>

namespace Core
{

class LogStreamerPrivate
{

public:
  LogStreamerPrivate( unsigned int log_flags, std::ostream* stream );

  void stream_message( unsigned int type, std::string message );

private:
  unsigned int log_flags_;
  std::ostream* ostream_ptr_;
  boost::mutex stream_mutex_;
};

LogStreamerPrivate::LogStreamerPrivate( unsigned int log_flags, std::ostream* stream ) :
  log_flags_( log_flags ),
  ostream_ptr_( stream )
{
}

void
LogStreamerPrivate::stream_message( unsigned int type, std::string message )
{
  boost::unique_lock< boost::mutex > lock( this->stream_mutex_ );
  if ( type & this->log_flags_ ) ( *this->ostream_ptr_ ) << message << std::endl;
}

LogStreamer::LogStreamer( unsigned int log_flags, std::ostream* stream )
{
  // Use a shared pointer to register the internals of this class
  this->private_ = boost::shared_ptr< LogStreamerPrivate >( 
    new LogStreamerPrivate( log_flags,stream ) );

  // If the internals are detroyed, so should the connection:
  // we use the signals2 system to track the shared_ptr and destroy the connection
  // if the object is destroyed. This will ensure that when the slot is called
  // the shared_ptr is locked so that the object is not destroyed while the
  // call back is evaluated.
  Log::Instance()->post_log_signal_.connect(
    Log::post_log_signal_type::slot_type( &LogStreamerPrivate::stream_message,
      this->private_.get(), _1, _2 ).track( this->private_ ) );
}

} // end namespace
