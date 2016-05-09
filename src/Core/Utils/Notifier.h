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

#ifndef CORE_UTILS_NOTIFIER_H
#define CORE_UTILS_NOTIFIER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

namespace Core
{

class Notifier;
typedef boost::shared_ptr<Notifier> NotifierHandle;

class Notifier : public boost::noncopyable
{
  // -- constructor / destructor --
public:
  Notifier();
  virtual ~Notifier();

public: 
  // WAIT:
  /// Wait for the event to be triggered. If the event was already triggered this function
  /// returns immediately.
  virtual void wait() = 0;
  
  // WAIT:
  /// Wait for the event to be triggered. If the event was already triggered this function
  /// returns immediately with true. After the timeout the function returns. If a timeout
  /// was triggered it returns false.
  virtual bool timed_wait( double timeout ) = 0;

  // GET_NAME:
  /// The name of the resource we are waiting for
  virtual std::string get_name() const = 0;
};

} // end namespace Core

#endif
