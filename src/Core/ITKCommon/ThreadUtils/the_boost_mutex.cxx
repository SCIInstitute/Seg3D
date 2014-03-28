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

// File         : the_boost_mutex.cxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:33:23 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thin wrapper Boost mutex class.

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>

// system includes:
#include <iostream>

// namespace access:
using std::cerr;
using std::endl;

//----------------------------------------------------------------
// define
// 
// #define DEBUG_MUTEX


//----------------------------------------------------------------
// the_boost_mutex_t::the_boost_mutex_t
// 
the_boost_mutex_t::the_boost_mutex_t():
  the_mutex_interface_t()
{}

//----------------------------------------------------------------
// the_boost_mutex_t::~the_boost_mutex_t
// 
the_boost_mutex_t::~the_boost_mutex_t()
{}

//----------------------------------------------------------------
// the_boost_mutex_t::delete_this
// 
void
the_boost_mutex_t::delete_this()
{
  delete this;
}

//----------------------------------------------------------------
// the_boost_mutex_t::create
// 
the_mutex_interface_t *
the_boost_mutex_t::create()
{
  return new the_boost_mutex_t();
}

//----------------------------------------------------------------
// the_boost_mutex_t::lock
// 
void
the_boost_mutex_t::lock()
{
#ifdef DEBUG_MUTEX
  cerr << this << "\tlock" << endl;
#endif
  
  mutex_.lock();
}

//----------------------------------------------------------------
// the_boost_mutex_t::unlock
// 
void
the_boost_mutex_t::unlock()
{
#ifdef DEBUG_MUTEX
  cerr << this << "\tunlock" << endl;
#endif
  
  mutex_.unlock();
}

//----------------------------------------------------------------
// the_boost_mutex_t::try_lock
// 
bool
the_boost_mutex_t::try_lock()
{
#ifdef DEBUG_MUTEX
  cerr << this << "\ttry_lock" << endl;
#endif
  
  return mutex_.try_lock();
}
