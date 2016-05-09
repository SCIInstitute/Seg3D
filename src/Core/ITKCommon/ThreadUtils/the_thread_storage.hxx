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

// File         : the_thread_storage.hxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Jan 2 09:30:00 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thread storage abstract interface class.

#ifndef THE_THREAD_STORAGE_HXX_
#define THE_THREAD_STORAGE_HXX_

// forward declarations:
class the_thread_interface_t;
class the_terminators_t;

//----------------------------------------------------------------
// the_thread_observer_t
// 
class the_thread_observer_t
{
public:
  the_thread_observer_t(the_thread_interface_t & thread):
    thread_(thread)
  {}
  
  the_thread_interface_t & thread_;
};

//----------------------------------------------------------------
// the_thread_storage_t
// 
class the_thread_storage_t
{
public:
  virtual ~the_thread_storage_t() {}
  
  // check whether the thread storage has been initialized:
  virtual bool is_ready() const = 0;
  
  // check whether the thread has been stopped:
  virtual bool thread_stopped() const = 0;
  
  // terminator access:
  virtual the_terminators_t & terminators() = 0;
  
  // thread id:
  virtual unsigned int thread_id() const = 0;
};

//----------------------------------------------------------------
// the_thread_storage_provider_t
// 
typedef the_thread_storage_t&(*the_thread_storage_provider_t)();

//----------------------------------------------------------------
// set_the_thread_storage_provider
// 
// Set the new thread storage provider, return the old provider.
// 
extern the_thread_storage_provider_t
set_the_thread_storage_provider(the_thread_storage_provider_t p);

//----------------------------------------------------------------
// the_thread_storage
// 
// Thread storage accessors.
// 
extern the_thread_storage_t & the_thread_storage();


#endif // THE_THREAD_STORAGE_HXX_
