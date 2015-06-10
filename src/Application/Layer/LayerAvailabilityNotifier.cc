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
 
// Application includes 
#include <Application/Layer/Layer.h> 
#include <Application/Layer/LayerAvailabilityNotifier.h> 
#include <Core/Utils/Log.h>

namespace Seg3D {

LayerAvailabilityNotifier::LayerAvailabilityNotifier( LayerHandle layer ) :
  layer_( layer ),
  layer_id_( layer->get_layer_id() ),
  triggered_( false )
{
  // This signal is triggered when the state of the layer is changed
  this->add_connection( layer->data_state_->state_changed_signal_.connect(
    boost::bind( &LayerAvailabilityNotifier::trigger, this ) ) );
    
  // This signal is triggered when the layer is deleted
  this->add_connection( LayerManager::Instance()->layers_deleted_signal_.connect(
    boost::bind( &LayerAvailabilityNotifier::trigger, this ) ) );
}

LayerAvailabilityNotifier::~LayerAvailabilityNotifier()
{
  this->disconnect_all();
}

void LayerAvailabilityNotifier::wait()
{
  CORE_LOG_DEBUG( "Wait on layer " + layer_id_ );
  boost::mutex::scoped_lock lock( this->notifier_mutex_ );
  while ( ! this->triggered_ )
  {
    this->notifier_cv_.wait( lock );
  }
}

bool LayerAvailabilityNotifier::timed_wait( double time )
{
  boost::mutex::scoped_lock lock( this->notifier_mutex_ );
  while ( ! this->triggered_ )
  {
    if ( !( this->notifier_cv_.timed_wait( lock,
      boost::posix_time::millisec( static_cast<long>( time * 1000.0) ) ) ) ) break;
  }
  
  return this->triggered_;
}

std::string LayerAvailabilityNotifier::get_name() const
{
  return this->layer_id_;
}

void LayerAvailabilityNotifier::trigger()
{
  // If it was already triggered, stop processing this event
  if ( this->triggered_ ) return;
  
  LayerHandle layer = this->layer_.lock();

  if ( layer )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    if ( layer->data_state_->get() == Layer::AVAILABLE_C && layer->locked_state_->get() == false )
    {
      // Layer has become available, so trigger waiting thread
      boost::mutex::scoped_lock lock( this->notifier_mutex_ );
      this->triggered_ = true;
      this->notifier_cv_.notify_all();
      CORE_LOG_DEBUG( layer_id_ + " is available.");
    }
    CORE_LOG_DEBUG( layer_id_ + " is not available.");
    // If this one was not triggered, the layer still exists and it is not available
  }
  else
  {
    // Layer does not exist any more, so please trigger the waiting thread
    boost::mutex::scoped_lock lock( this->notifier_mutex_ );
    this->triggered_ = true;
    this->notifier_cv_.notify_all();
  }
}

} // end namespace Seg3D
