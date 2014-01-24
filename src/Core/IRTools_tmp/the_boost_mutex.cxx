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


// File         : the_boost_mutex.cxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:33:23 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A thin wrapper Boost mutex class.

// local includes:
#include <Core/IRTools_tmp/the_boost_mutex.hxx>

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
