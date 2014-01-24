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


// File         : the_boost_thread.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:35:09 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A thin wrapper for Boost thread class.

#ifndef THE_BOOST_THREAD_HXX_
#define THE_BOOST_THREAD_HXX_

// Boost includes:
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

// local includes:
#include <Core/IRTools_tmp/the_terminator.hxx>
#include <Core/IRTools_tmp/the_thread_interface.hxx>

// forward declarations:
class the_mutex_interface_t;
class the_thread_storage_t;


//----------------------------------------------------------------
// the_boost_terminators_t
// 
class the_boost_terminators_t : public the_terminators_t
{
public:
  // virtual: concurrent access controls:
  void lock()	{ mutex_.lock(); }
  void unlock()	{ mutex_.unlock(); }
  
private:
  mutable boost::mutex mutex_;
};


//----------------------------------------------------------------
// the_boost_thread_t
// 
// 1. the thread will not take ownership of the transactions.
// 2. the thread will take ownership of the mutex.
// 
class the_boost_thread_t : public the_thread_interface_t
{
private:
  struct callable_t
  {
    callable_t(the_boost_thread_t * thread):
      thread_(thread)
    {}
    
    void operator()()
    {
      thread_->run();
    }
    
  private:
    the_boost_thread_t * thread_;
  };
  
public:
  the_boost_thread_t();
  
  // the destructor is protected on purpose,
  // see delete_this for details:
  virtual ~the_boost_thread_t();
  
  // In order to avoid memory management problems with shared libraries,
  // whoever provides this interface instance (via it's creator), has to
  // provide a way to delete the instance as well.  This will avoid
  // issues with multiple-instances of C runtime libraries being
  // used by the app and whatever libraries it links against that
  // either use or provide this interface:
  virtual void delete_this();
  
  // the creation method:
  static the_thread_interface_t * create()
  { return new the_boost_thread_t(); }
  
  // the thread storage accessor:
  static the_thread_storage_t & thread_storage();
  
  // virtual: start the thread:
  void start();
  
  // virtual:
  void wait();
  
  // virtual: put the thread to sleep:
  void take_a_nap(const unsigned long & microseconds);
  
  // virtual: accessor to the transaction terminators:
  the_terminators_t & terminators();
  
protected:
  // virtual:
  void run();
  
  // the boost thread:
  boost::thread * boost_thread_;
  
  // a list of active terminators for this thread:
  the_boost_terminators_t terminators_;
};


#endif // THE_BOOST_THREAD_HXX_
