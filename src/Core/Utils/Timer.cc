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

#include <boost/thread.hpp>

#include <Core/Utils/Log.h>
#include <Core/Utils/Timer.h>

namespace Core
{

class TimerPrivate
{
public:
  void run();

public:
  Timer* timer_;
  long long interval_;
  bool running_;
  bool single_shot_;
  boost::thread* timer_thread_;
  boost::mutex mutex_;
  boost::condition_variable timer_condition_;
};

void TimerPrivate::run()
{
  {
    boost::mutex::scoped_lock lock( this->mutex_ );
    this->timer_condition_.notify_one();
  }

  CORE_LOG_DEBUG( "Timer started" );

  boost::mutex::scoped_lock lock( this->mutex_ );
  while ( this->running_ )
  {
    this->timer_condition_.timed_wait( lock, boost::posix_time::millisec( this->interval_ ) );
    if ( this->running_ )
    {
      if ( this->single_shot_ )
      {
        this->running_ = false;
      }
      this->timer_->timeout_signal_();
    }
  }

  CORE_LOG_DEBUG( "Timer stopped" );

  this->timer_condition_.notify_all();
}

Timer::Timer( long long interval ) :
  private_( new TimerPrivate )
{
  this->private_->interval_ = interval;
  this->private_->running_ = false;
  this->private_->timer_ = this;
  this->private_->single_shot_ = false;
  this->private_->timer_thread_ = 0;
}

Timer::~Timer()
{
  this->stop();
}

void Timer::start()
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  if ( !this->private_->running_ )
  {
    // Clean up previous timer thread
    if ( this->private_->timer_thread_ != 0 )
    {
      delete this->private_->timer_thread_;
    }
    
    this->private_->running_ = true;
    this->private_->timer_thread_ = new boost::thread( boost::bind( 
      &TimerPrivate::run, this->private_ ) );
    this->private_->timer_condition_.wait( lock );
  }
}

void Timer::set_interval( long long interval )
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  this->private_->interval_ = interval;
}

void Timer::stop()
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  if ( this->private_->running_ )
  {
    this->private_->running_ = false;
    this->private_->timer_condition_.notify_all();
    this->private_->timer_condition_.wait( lock );
  }

  if ( this->private_->timer_thread_ != 0 )
  {
    delete this->private_->timer_thread_;
    this->private_->timer_thread_ = 0;
  }
}

void Timer::set_single_shot( bool single_shot )
{
  boost::mutex::scoped_lock lock( this->private_->mutex_ );
  this->private_->single_shot_ = single_shot;
}

} // end namespace Core
