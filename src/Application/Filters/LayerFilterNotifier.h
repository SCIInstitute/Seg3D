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
 
#ifndef APPLICATION_FILTERS_LAYEFILTERNOTIFIER_H
#define APPLICATION_FILTERS_LAYEFILTERNOTIFIER_H

// Boost includes
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/Utils/Notifier.h>

namespace Seg3D {

class LayerFilterNotifier;
typedef boost::shared_ptr< LayerFilterNotifier > LayerFilterNotifierHandle;

class LayerFilterNotifierPrivate;
typedef boost::shared_ptr< LayerFilterNotifierPrivate > LayerFilterNotifierPrivateHandle;

class LayerFilterNotifier : public Core::Notifier
{
  // -- constructor/ destructor -- 
public:
  LayerFilterNotifier( const std::string& filter_name );
  virtual ~LayerFilterNotifier();

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
  friend class LayerFilterPrivate;
  /// TRIGGER:
  /// Called by LayerFilter when it has finished processing.
  void trigger();

private:
  LayerFilterNotifierPrivateHandle private_;
};

} // end namespace Seg3D

#endif
