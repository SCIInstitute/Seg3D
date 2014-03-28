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

// File         : the_thread_storage.cxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Jan 9 12:27:00 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thread storage abstract interface class

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_thread_storage.hxx>
#include <Core/ITKCommon/ThreadUtils/the_terminator.hxx>


//----------------------------------------------------------------
// the_dummy_terminators_t
// 
class the_dummy_terminators_t : public the_terminators_t
{
public:
  // virtual:
  void lock() {}
  void unlock() {}
};

//----------------------------------------------------------------
// the_dummy_thread_storage_t
// 
class the_dummy_thread_storage_t : public the_thread_storage_t
{
public:
  // virtual:
  bool is_ready() const
  { return true; }
  
  bool thread_stopped() const
  { return false; }
  
  the_terminators_t & terminators()
  { return terminators_; }
  
  unsigned int thread_id() const
  { return ~0u; }
  
private:
  the_dummy_terminators_t terminators_;
};

//----------------------------------------------------------------
// the_dummy_thread_storage
// 
static the_thread_storage_t &
the_dummy_thread_storage()
{
  static the_dummy_thread_storage_t thread_storage;
  return thread_storage;
}

//----------------------------------------------------------------
// thread_storage_provider_
// 
static the_thread_storage_provider_t
thread_storage_provider_ = the_dummy_thread_storage;

//----------------------------------------------------------------
// set_the_thread_storage_provider
// 
the_thread_storage_provider_t
set_the_thread_storage_provider(the_thread_storage_provider_t p)
{
  the_thread_storage_provider_t old = thread_storage_provider_;
  thread_storage_provider_ = p;
  return old;
}

//----------------------------------------------------------------
// the_thread_storage
// 
the_thread_storage_t &
the_thread_storage()
{
  return thread_storage_provider_();
}
