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

#ifndef CORE_ACTION_ACTIONCACHEDHANDLE_H
#define CORE_ACTION_ACTIONCACHEDHANDLE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL
#include <string>
#include <algorithm>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core
#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Converter/StringConverter.h>

namespace Core
{

// CACHEDPTR CLASSES
// ACTIONCACHEDHANDLE<TYPE>
// A cached handle is something that should not exist in the ation but is there to accelerate
// the performance of the code. For instance when loading a file the UI needs to read the file
// up front before being able to present the user with options. In such a case the importer could
// use the already read data block and significantly speed up the file load, when the action is
// finally issued. However the logic in the action SHOULD NOT depend on these data, as a script
// may not have read the data yet and would require it to be read first. Hence we have cached handle
// objects that contain hints to improve performance. However these hints will be deleted as soon
// as the action finishes.

// ACTIONCACHEDHANDLEBASE:
// Base class needed for uniform delete the auxillary handles in the object

class ActionCachedHandleBase;
typedef boost::shared_ptr< ActionCachedHandleBase > ActionCachedHandleBaseHandle;

class ActionCachedHandleBase
{
  // -- destructor --
public:
  // A destructor is required in this base class.
  virtual ~ActionCachedHandleBase();
  
  // CLEAR_CACHED_HANDLE:
  // Clear the smart pointer to the cached object.
  virtual void clear_cached_handle() = 0;
};

// ACTIONCACHEDHANDLE:
// Parameter for an action.

// Forward declaration:
template< class T > class ActionCachedHandle;

// Class definition:
template< class T >
class ActionCachedHandle : public ActionCachedHandleBase
{
  // -- constructor/destructor --
public:
  ActionCachedHandle()
  {
  }

  virtual ~ActionCachedHandle()
  {
  }

  // -- access to value --
public:
  // HANDLE:
  // General access to the parameter value.
  T& handle()
  {
    return handle_;
  }

  // HANDLE:
  // For run when running with constness.
  const T& handle() const
  {
    return handle_;
  }

  // CLEAR_CACHED_HANDLE:
  // Clear the smart pointer to the cached object.
  virtual void clear_cached_handle()
  {
    handle_.reset();
  }

private:
  // The actual value
  T handle_;
};

} // namespace Core

#endif
