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


// File         : the_boost_thread.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:35:09 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thin wrapper for Boost thread class.

#ifndef THE_BOOST_THREAD_HXX_
#define THE_BOOST_THREAD_HXX_

// Boost includes:
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_terminator.hxx>
#include <Core/ITKCommon/ThreadUtils/the_thread_interface.hxx>

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
