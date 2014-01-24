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


// File         : the_boost_thread_storage.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sat Oct 25 12:35:47 MDT 2008
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A thin wrapper for Boost thread specific storage.

#ifndef THE_BOOST_THREAD_STORAGE_HXX_
#define THE_BOOST_THREAD_STORAGE_HXX_

// local includes:
#include <Core/IRTools_tmp/the_thread_storage.hxx>

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
