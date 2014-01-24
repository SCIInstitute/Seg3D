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


// File         : the_terminator.cxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Sep 24 18:08:00 MDT 2006
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : a thread terminator convenience class


// local includes:
#include <Core/IRTools_tmp/the_terminator.hxx>
#include <Core/IRTools_tmp/the_exception.hxx>

// system includes:
#include <exception>
#include <sstream>
#include <iostream>
#include <assert.h>

// namespace access:
using std::cerr;
using std::endl;


//----------------------------------------------------------------
// DEBUG_TERMINATORS
// 
// #define DEBUG_TERMINATORS


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
#ifdef DEBUG_TERMINATORS
  cerr << "TERMINATE: " << id_ << ", this: " << this << endl;
#endif // DEBUG_TERMINATORS
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
  os << id_ << " interrupted" << endl;
  
  the_exception_t e(os.str().c_str());
  throw e;
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
  
#ifdef DEBUG_TERMINATORS
  cerr << endl << &terminators_ << ": terminate_all -- begin" << endl;
#endif // DEBUG_TERMINATORS
  
  for (std::list<the_terminator_t *>::iterator i = terminators_.begin();
       i != terminators_.end(); ++i)
  {
    the_terminator_t * t = *i;
    t->terminate();
  }
  
#ifdef DEBUG_TERMINATORS
  cerr << &terminators_ << ": terminate_all -- end" << endl;
#endif // DEBUG_TERMINATORS
}

//----------------------------------------------------------------
// the_terminators_t::verify_termination
// 
bool
the_terminators_t::verify_termination()
{
  the_lock_t<the_terminators_t> lock(*this);
  
#ifdef DEBUG_TERMINATORS
  for (std::list<the_terminator_t *>::iterator iter = terminators_.begin();
       iter != terminators_.end(); ++iter)
  {
    the_terminator_t * t = *iter;
    cerr << "ERROR: remaining terminators: " << t->id() << endl;
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
  
#ifdef DEBUG_TERMINATORS
  cerr << &terminators_ << ": appended " << terminator->id()
       << " terminator, addr " << terminator << endl;
#endif // DEBUG_TERMINATORS
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
    cerr << "ERROR: no such terminator: " << terminator->id() << endl;
    assert(0);
    return;
  }
  
  terminators_.erase(iter);
  
#ifdef DEBUG_TERMINATORS
  cerr << &terminators_ << ": removed " << terminator->id()
       << " terminator, addr " << terminator << endl;
#endif // DEBUG_TERMINATORS
}
