// -*- Mode: c++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: t -*-
// NOTE: the first line of this file sets up source code indentation rules
// for Emacs; it is also a hint to anyone modifying this file.

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// File         : the_boost_thread.cxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:33:09 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A thin wrapper for Boost thread class.

// local include:
#include <Core/IRTools_tmp/the_boost_thread.hxx>
#include <Core/IRTools_tmp/the_boost_thread_storage.hxx>
#include <Core/IRTools_tmp/the_boost_mutex.hxx>
#include <Core/IRTools_tmp/the_mutex_interface.hxx>

// system includes:
#include <iostream>

// namespace access:
using std::cout;
using std::cerr;
using std::endl;

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
  assert(!boost_thread_);
  
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
  if (!boost_thread_) return;
  
  if (boost_thread_->get_id() == boost::this_thread::get_id())
  {
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
