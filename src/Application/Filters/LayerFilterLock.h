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
 
#ifndef APPLICATION_FILTERS_BASEFILTERLOCK_H 
#define APPLICATION_FILTERS_BASEFILTERLOCK_H
 
// Boost includes
#include <boost/smart_ptr.hpp> 
#include <boost/utility.hpp> 
#include <boost/thread/mutex.hpp> 
#include <boost/thread/condition_variable.hpp> 

// Core includes
#include <Core/Utils/Singleton.h>

namespace Seg3D
{

/// This class prevents too many filters running simultaneously by allowing only
/// a certain amount of the filters to run in parallel. If too many threads are started
/// some will have to wait until others are done.

class LayerFilterLockPrivate;
typedef boost::shared_ptr<LayerFilterLockPrivate> LayerFilterLockPrivateHandle;


class LayerFilterLock : public boost::noncopyable
{
  CORE_SINGLETON( LayerFilterLock );

  // -- Constructor/Destructor --
private:
  LayerFilterLock();
  virtual ~LayerFilterLock();
    
  // -- interface --
public:
  /// Lock the resource. The function will continue if enough filter slots are available
  void lock();
  
  /// Unlock the resource.
  void unlock();
  
  // -- internals --
private:
  LayerFilterLockPrivateHandle private_;

};
  
} // end namespace Seg3D

#endif
