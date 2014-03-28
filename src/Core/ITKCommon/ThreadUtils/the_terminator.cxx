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


// File         : the_terminator.cxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Sep 24 18:08:00 MDT 2006
// Copyright    : (C) 2004-2008 University of Utah
// Description  : a thread terminator convenience class


// local includes:
#include <Core/ITKCommon/ThreadUtils/the_terminator.hxx>

#include <Core/Utils/Exception.h>

// system includes:
#include <exception>
#include <sstream>
#include <iostream>
#include <cassert>


//----------------------------------------------------------------
// the_terminator_t::the_terminator_t
// 
the_terminator_t::the_terminator_t(const char * id):
  id_(id),
  termination_requested_(should_terminate_immediately())
{
  terminate_on_request();
  add(this);
}

//----------------------------------------------------------------
// the_terminator_t::
// 
the_terminator_t::~the_terminator_t()
{
  del(this);
}

//----------------------------------------------------------------
// the_terminator_t::terminate
// 
void
the_terminator_t::terminate()
{
#ifndef NDEBUG
  std::ostringstream os;
  os << "TERMINATE: " << id_ << ", this: " << this;
  CORE_LOG_DEBUG(os.str());
#endif
  termination_requested_ = true;
}

//----------------------------------------------------------------
// the_terminator_t::throw_exception
// 
void
the_terminator_t::throw_exception() const
{
  // abort, termination requested:
  std::ostringstream os;
  os << id_ << " interrupted" << std::endl;
  
  CORE_THROW_EXCEPTION(os.str());
  
//  the_exception_t e(os.str().c_str());
//  throw e;
}

//----------------------------------------------------------------
// the_terminator_t::verify_termination
// 
bool
the_terminator_t::verify_termination()
{
  return the_thread_storage().terminators().verify_termination();
}

//----------------------------------------------------------------
// the_terminator_t::should_terminate_immediately
// 
bool
the_terminator_t::should_terminate_immediately()
{
  return the_thread_storage().thread_stopped();
}

//----------------------------------------------------------------
// the_terminator_t::add
// 
void
the_terminator_t::add(the_terminator_t * terminator)
{
  the_thread_storage().terminators().add(terminator);
}

//----------------------------------------------------------------
// the_terminator_t::del
// 
void
the_terminator_t::del(the_terminator_t * terminator)
{
  the_thread_storage().terminators().del(terminator);
}


//----------------------------------------------------------------
// the_terminators_t::~the_terminators_t
// 
the_terminators_t::~the_terminators_t()
{
  for (std::list<the_terminator_t *>::iterator i = terminators_.begin();
       i != terminators_.end(); ++i)
  {
    the_terminator_t * t = *i;
    delete t;
  }
    
  terminators_.clear();
}

//----------------------------------------------------------------
// the_terminators_t::terminate
// 
void
the_terminators_t::terminate()
{
  the_lock_t<the_terminators_t> lock(*this);
  
#ifndef NDEBUG
  std::ostringstream os;
  os << std::endl << &terminators_ << ": terminate_all -- begin";
  CORE_LOG_DEBUG(os.str());
#endif
  
  for (std::list<the_terminator_t *>::iterator i = terminators_.begin();
       i != terminators_.end(); ++i)
  {
    the_terminator_t * t = *i;
    t->terminate();
  }
  
#ifndef NDEBUG
  std::ostringstream os;
  os << &terminators_ << ": terminate_all -- end";
  CORE_LOG_DEBUG(os.str());
#endif
}

//----------------------------------------------------------------
// the_terminators_t::verify_termination
// 
bool
the_terminators_t::verify_termination()
{
  the_lock_t<the_terminators_t> lock(*this);
  
#ifndef NDEBUG
  for (std::list<the_terminator_t *>::iterator iter = terminators_.begin();
       iter != terminators_.end(); ++iter)
  {
    the_terminator_t * t = *iter;
    std::ostringstream os;
    os << "ERROR: remaining terminators: " << t->id();
    CORE_LOG_DEBUG(os.str());
  }
#endif
  
  return terminators_.empty();
}

//----------------------------------------------------------------
// the_terminators_t::add
// 
void
the_terminators_t::add(the_terminator_t * terminator)
{
  the_lock_t<the_terminators_t> lock(*this);
  terminators_.push_front(terminator);
  
#ifndef NDEBUG
  std::ostringstream os;
  os << &terminators_ << ": appended " << terminator->id()
     << " terminator, addr " << terminator;
  CORE_LOG_DEBUG(os.str());
#endif
}

//----------------------------------------------------------------
// the_terminators_t::del
// 
void
the_terminators_t::del(the_terminator_t * terminator)
{
  the_lock_t<the_terminators_t> lock(*this);
  
  std::list<the_terminator_t *>::iterator iter =
    std::find(terminators_.begin(), terminators_.end(), terminator);
  
  if (iter == terminators_.end())
  {
    std::cerr << "ERROR: no such terminator: " << terminator->id() << std::endl;
    // TODO: no-op might be OK here
    assert(false);
    return;
  }
  
  terminators_.erase(iter);
  
#ifndef NDEBUG
  std::ostringstream os;
  os << &terminators_ << ": removed " << terminator->id()
     << " terminator, addr " << terminator;
  CORE_LOG_DEBUG(os.str());
#endif
}
