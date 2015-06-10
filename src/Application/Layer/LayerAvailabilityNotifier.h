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
 
#ifndef APPLICATION_LAYER_LAYERAVAILABILITYNOTIFIER_H
#define APPLICATION_LAYER_LAYERAVAILABILITYNOTIFIER_H

// Boost includes
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

// STL includes
#include <string>

// Core includes
#include <Core/Utils/Notifier.h>
#include <Core/Utils/ConnectionHandler.h>

// Application includes
#include <Application/Layer/LayerManager.h>

namespace Seg3D {

class LayerAvailabilityNotifier : public Core::Notifier, Core::ConnectionHandler
{
  // -- constuctor/ destructor -- 
public:
  LayerAvailabilityNotifier( LayerHandle layer );
  virtual ~LayerAvailabilityNotifier();

  // -- overloaded functions --
public:
  /// WAIT:
  /// Wait for the event to be triggered. If the event was already triggered this function
  /// returns immediately.
  virtual void wait();
  
  /// WAIT:
  /// Wait for the event to be triggered. If the event was already triggered this function
  /// returns immediately with true. After the timeout the function returns. If a timeout
  /// was triggered it returns false.
  virtual bool timed_wait( double timeout );

  /// GET_NAME:
  /// The name of the resource we are waiting for
  virtual std::string get_name() const;

private:
  /// TRIGGER:
  /// this function is called when a change in the LayerManager occurs or a change in the
  /// layer state. At that point in time we need to recheck whether
  void trigger();

  /// Weak handle to the layer that we are monitoring
  LayerWeakHandle layer_;
  
  /// Name of the layer we are waiting for
  std::string layer_id_;
  
  /// Whether the notifier has been triggered
  bool triggered_;
  
  /// Mutex that protects the condition variable
  boost::mutex notifier_mutex_;
  
  /// Condition variable that is used to keep threads asleep while it is waiting for the
  /// change of data status of the layer
  boost::condition_variable notifier_cv_;
};

} // end namespace Seg3D

#endif
