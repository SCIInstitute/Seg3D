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

// File         : the_boost_thread_storage.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:35:47 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thin wrapper for Boost thread specific storage.

#ifndef THE_BOOST_THREAD_STORAGE_HXX_
#define THE_BOOST_THREAD_STORAGE_HXX_

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_thread_storage.hxx>

// Boost includes:
#include <boost/thread/tss.hpp>


//----------------------------------------------------------------
// the_boost_thread_storage_t
//
class the_boost_thread_storage_t :
  public boost::thread_specific_ptr<the_thread_observer_t>,
  public the_thread_storage_t
{
public:
  // virtual: check whether the thread storage has been initialized:
  bool is_ready() const
  {
    return (boost::thread_specific_ptr<the_thread_observer_t>::get() != NULL);
  }
  
  // virtual: check whether the thread has been stopped:
  bool thread_stopped() const
  {
    return
      boost::thread_specific_ptr<the_thread_observer_t>::get()->
      thread_.stopped();
  }
  
  // virtual: terminator access:
  the_terminators_t & terminators()
  {
    return
      boost::thread_specific_ptr<the_thread_observer_t>::get()->
      thread_.terminators();
  }
  
  // virtual:
  unsigned int thread_id() const
  {
    return
      boost::thread_specific_ptr<the_thread_observer_t>::get()->
      thread_.id();
  }
};


#endif // THE_BOOST_THREAD_STORAGE_HXX_
