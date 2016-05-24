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
 

// Application includes
#include <Application/Filters/LayerFilterLock.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( LayerFilterLock );

class LayerFilterLockPrivate
{
public:
  int max_filter_count_;
  int current_filter_count_;

  boost::mutex mutex_;
  boost::condition_variable condition_variable_;
};

LayerFilterLock::LayerFilterLock() :
  private_( new LayerFilterLockPrivate )
{
  this->private_->max_filter_count_ = 4;
  this->private_->current_filter_count_ = 0;
}

LayerFilterLock::~LayerFilterLock()
{
}

void LayerFilterLock::lock()
{
  boost::unique_lock<boost::mutex> lock( this->private_->mutex_ );
  while ( this->private_->current_filter_count_ >=  this->private_->max_filter_count_ )
  {
    this->private_->condition_variable_.wait( lock );
  }
  
  this->private_->current_filter_count_++;
}

void LayerFilterLock::unlock()
{
  boost::unique_lock<boost::mutex> lock( this->private_->mutex_ );
  this->private_->current_filter_count_--;
  this->private_->condition_variable_.notify_all();
}

} // end namespace Core
