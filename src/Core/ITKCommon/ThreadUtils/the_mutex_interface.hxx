/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

// File         : the_mutex_interface.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Feb 18 16:12:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : An abstract mutex class interface.

#ifndef THE_MUTEX_INTERFACE_HXX_
#define THE_MUTEX_INTERFACE_HXX_


//----------------------------------------------------------------
// the_mutex_interface_t
// 
class the_mutex_interface_t
{
protected:
  // the destructor is protected on purpose,
  // see delete_this for details:
  virtual ~the_mutex_interface_t();
  
public:
  // In order to avoid memory management problems with shared libraries,
  // whoever provides this interface instance (via it's creator), has to
  // provide a way to delete the instance as well.  This will avoid
  // issues with multiple-instances of C runtime libraries being
  // used by the app and whatever libraries it links against that
  // either use or provide this interface:
  virtual void delete_this() = 0;
  
  // mutex controls:
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual bool try_lock() = 0;
  
  //----------------------------------------------------------------
  // creator_t
  // 
  typedef the_mutex_interface_t *(*creator_t)();
  
  // specify a thread creation method:
  static void set_creator(creator_t creator);
  
  // create a new instance of a thread:
  static the_mutex_interface_t * create();
  
protected:
  // an abstract mutex creator:
  static creator_t creator_;
};


#endif // THE_MUTEX_INTERFACE_HXX_
