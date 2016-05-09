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
 
#ifndef APPLICATION_LAYER_LAYERABSTRACTFILTER_H 
#define APPLICATION_LAYER_LAYERABSTRACTFILTER_H 
 
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif  
 
// Boost includes
#include <boost/smart_ptr.hpp> 
 
// Core includes
#include <Core/Utils/Runnable.h>

namespace Seg3D
{

/// CLASS BASEFILTER:
/// This class provides the basic underlying framework for running filters in a separate thread
/// from the application thread. It provides a series of functions common to all the filters.

class LayerAbstractFilter;
typedef boost::shared_ptr<LayerAbstractFilter> LayerAbstractFilterHandle;
typedef boost::weak_ptr<LayerAbstractFilter>   LayerAbstractFilterWeakHandle;

class LayerAbstractFilter : public Core::Runnable, 
  public boost::enable_shared_from_this< LayerAbstractFilter >
{

public:
  LayerAbstractFilter();
  virtual ~LayerAbstractFilter();
  
  // -- abort handling -- 
public:
  virtual void abort_and_wait() = 0;

  /// RAISE_ABORT:
  /// Raise the abort flag
  virtual void raise_abort() = 0;

  /// CHECK_ABORT:
  /// Check the abort flag
  virtual bool check_abort() = 0;

};

} // end namespace Seg3D

#endif
