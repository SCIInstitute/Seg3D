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


// File         : the_mutex_interface.cxx
// Author       : Pavel A. Koshevoy
// Created      : Wed Feb 21 08:22:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : An abstract mutex class interface.

// local includes:
#include <Core/IRTools_tmp/the_mutex_interface.hxx>

// system includes:
#include <stddef.h>


//----------------------------------------------------------------
// the_mutex_interface_t::creator_
// 
the_mutex_interface_t::creator_t
the_mutex_interface_t::creator_ = NULL;

//----------------------------------------------------------------
// the_mutex_interface_t::~the_mutex_interface_t
// 
the_mutex_interface_t::~the_mutex_interface_t()
{}

//----------------------------------------------------------------
// the_mutex_interface_t::set_creator
// 
void
the_mutex_interface_t::set_creator(the_mutex_interface_t::creator_t creator)
{
  creator_ = creator;
}

//----------------------------------------------------------------
// the_mutex_interface_t::create
// 
the_mutex_interface_t *
the_mutex_interface_t::create()
{
  if (creator_ == NULL) return NULL;
  return creator_();
}
