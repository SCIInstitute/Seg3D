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

// STL includes
#include <string>

// Boost includes
#include <boost/thread.hpp>

// Application includes 
#include <Application/Filters/LayerFilterNotifier.h> 
 
namespace Seg3D {

class LayerFilterNotifierPrivate
{
public:
  // Name of the filter we are waiting for
  std::string filter_name_;

  // Whether the notifier has been triggered
  bool triggered_;

  // Mutex that protects the condition variable
  boost::mutex notifier_mutex_;

  // Condition variable that is used to keep threads asleep while it is waiting for the
  // change of data status of the layer
  boost::condition_variable notifier_cv_;
};

LayerFilterNotifier::LayerFilterNotifier( const std::string& filter_name ) :
  private_( new LayerFilterNotifierPrivate )
{
  this->private_->triggered_ = false;
  this->private_->filter_name_ = filter_name;
}

LayerFilterNotifier::~LayerFilterNotifier()
{
}

void LayerFilterNotifier::wait()
{
  boost::mutex::scoped_lock lock( this->private_->notifier_mutex_ );
  while ( !this->private_->triggered_ )
  {
    this->private_->notifier_cv_.wait( lock );
  }
}

bool LayerFilterNotifier::timed_wait( double time )
{
  boost::mutex::scoped_lock lock( this->private_->notifier_mutex_ );
  while ( !this->private_->triggered_ )
  {
    if ( !this->private_->notifier_cv_.timed_wait( lock,
      boost::posix_time::millisec( static_cast<long>( time * 1000.0) ) ) ) break;
  }
  
  return this->private_->triggered_;
}

std::string LayerFilterNotifier::get_name() const
{
  return this->private_->filter_name_;
}

void LayerFilterNotifier::trigger()
{
  // If it was already triggered, stop processing this event
  if ( this->private_->triggered_ ) return;

  boost::mutex::scoped_lock lock( this->private_->notifier_mutex_ );
  this->private_->triggered_ = true;
  this->private_->notifier_cv_.notify_all();
}

} // end namespace Seg3D
