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

// File         : the_boost_thread.cxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:33:09 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thin wrapper for Boost thread class.

// local include:
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread_storage.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_mutex_interface.hxx>

// system includes:
#include <iostream>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

//----------------------------------------------------------------
// DEBUG_THREAD
// 
// #define DEBUG_THREAD


//----------------------------------------------------------------
// THREAD_STORAGE
// 
static the_boost_thread_storage_t THREAD_STORAGE;

//----------------------------------------------------------------
// the_boost_thread_t::the_boost_thread_t
// 
the_boost_thread_t::the_boost_thread_t():
  the_thread_interface_t(the_boost_mutex_t::create()),
  boost_thread_(NULL)
{
  if (THREAD_STORAGE.get() == NULL)
  {
    THREAD_STORAGE.reset(new the_thread_observer_t(*this));
  }
}

//----------------------------------------------------------------
// the_boost_thread_t::~the_boost_thread_t
// 
the_boost_thread_t::~the_boost_thread_t()
{
  if (boost_thread_)
  {
    wait();
  }
}

//----------------------------------------------------------------
// the_boost_thread_t::delete_this
// 
void
the_boost_thread_t::delete_this()
{
  delete this;
}

//----------------------------------------------------------------
// ImageProcessingThread::thread_storage
// 
the_thread_storage_t &
the_boost_thread_t::thread_storage()
{
  return THREAD_STORAGE;
}

//----------------------------------------------------------------
// the_boost_thread_t::start
// 
void
the_boost_thread_t::start()
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
#ifdef DEBUG_THREAD
  cerr << "start of thread " << this << " requested" << endl;
#endif
  
  if (boost_thread_)
  {
    if (!stopped_)
    {
      // already running:
#ifdef DEBUG_THREAD
      cerr << "thread " << this << " is already running" << endl;
#endif
      return;
    }
    else
    {
      // wait for the shutdown to succeed, then restart the thread:
#ifdef DEBUG_THREAD
      cerr << "waiting for thread " << this << " to shut down" << endl;
#endif
      wait();
    }
  }
  
#ifdef DEBUG_THREAD
  cerr << "starting thread " << this << endl;
#endif
  
  // we shouldn't have a Boost thread at this stage:
//  assert(!boost_thread_);
  if (boost_thread_ != 0)
  {
    CORE_THROW_EXCEPTION("boost thread should not be instantiated yet");
  }
  
  // clear the termination flag:
  stopped_ = false;
  boost_thread_ = new boost::thread(callable_t(this));
}

//----------------------------------------------------------------
// the_boost_thread_t::wait
// 
void
the_boost_thread_t::wait()
{
  if (! boost_thread_) return;
  
  if (boost_thread_->get_id() == boost::this_thread::get_id())
  {
//    CORE_THROW_EXCEPTION("invalid thread ID");
    // TODO: no-op might be OK here
    assert(false);
    return;
  }
  
  boost_thread_->join();
  delete boost_thread_;
  boost_thread_ = NULL;
}

//----------------------------------------------------------------
// the_boost_thread_t::take_a_nap
// 
void
the_boost_thread_t::take_a_nap(const unsigned long & microseconds)
{
  boost::xtime xt;
  xt.sec = microseconds / 1000000;
  xt.nsec = microseconds % 1000000;
  boost::thread::sleep(xt);
}

//----------------------------------------------------------------
// the_boost_thread_t::terminators
// 
the_terminators_t &
the_boost_thread_t::terminators()
{
  return terminators_;
}

//----------------------------------------------------------------
// the_boost_thread_t::run
// 
void
the_boost_thread_t::run()
{
  // setup the thread storage:
  {
    the_lock_t<the_mutex_interface_t> locker(mutex_);
    THREAD_STORAGE.reset(new the_thread_observer_t(*this));
  }
  
  // process the transactions:
  work();
  
  // clean up the thread storage:
  THREAD_STORAGE.reset(NULL);
}
