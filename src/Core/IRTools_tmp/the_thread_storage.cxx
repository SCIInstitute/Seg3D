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


// File         : the_thread_storage.cxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Jan 9 12:27:00 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A thread storage abstract interface class

// local includes:
#include <Core/IRTools_tmp/the_thread_storage.hxx>
#include <Core/IRTools_tmp/the_terminator.hxx>


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
