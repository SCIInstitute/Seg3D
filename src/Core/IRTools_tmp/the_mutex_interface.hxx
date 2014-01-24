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


// File         : the_mutex_interface.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Feb 18 16:12:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : An abstract mutex class interface.

#ifndef THE_MUTEX_INTERFACE_HXX_
#define THE_MUTEX_INTERFACE_HXX_


//----------------------------------------------------------------
// the_mutex_interface_t
// 
class the_mutex_interface_t
{
protected:
  // the destructor is protected on purpose,
  // see delete_this for details:
  virtual ~the_mutex_interface_t();
  
public:
  // In order to avoid memory management problems with shared libraries,
  // whoever provides this interface instance (via it's creator), has to
  // provide a way to delete the instance as well.  This will avoid
  // issues with multiple-instances of C runtime libraries being
  // used by the app and whatever libraries it links against that
  // either use or provide this interface:
  virtual void delete_this() = 0;
  
  // mutex controls:
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual bool try_lock() = 0;
  
  //----------------------------------------------------------------
  // creator_t
  // 
  typedef the_mutex_interface_t *(*creator_t)();
  
  // specify a thread creation method:
  static void set_creator(creator_t creator);
  
  // create a new instance of a thread:
  static the_mutex_interface_t * create();
  
protected:
  // an abstract mutex creator:
  static creator_t creator_;
};


#endif // THE_MUTEX_INTERFACE_HXX_
