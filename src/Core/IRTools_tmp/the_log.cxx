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


// File         : the_log.cxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Mar 23 11:04:53 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A text log object -- behaves almost like a std::ostream.

// local includes:
#include <Core/IRTools_tmp/the_log.hxx>


//----------------------------------------------------------------
// the_log_t::the_log_t
// 
the_log_t::the_log_t():
  mutex_(NULL)
{
  mutex_ = the_mutex_interface_t::create();
}

//----------------------------------------------------------------'
// the_log_t::~the_log_t
// 
the_log_t::~the_log_t()
{
  mutex_->delete_this();
}

//----------------------------------------------------------------
// the_log_t::log_no_lock
// 
void
the_log_t::log_no_lock(std::ostream & (*f)(std::ostream &))
{
  f(line_);
}

//----------------------------------------------------------------
// the_log_t::operator
// 
the_log_t &
the_log_t::operator << (std::ostream & (*f)(std::ostream &))
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  log_no_lock(f);
  return *this;
}

//----------------------------------------------------------------
// the_log_t::precision
// 
std::streamsize
the_log_t::precision()
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  std::streamsize p = line_.precision();
  return p;
}

//----------------------------------------------------------------
// the_log_t::precision
// 
std::streamsize
the_log_t::precision(std::streamsize n)
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  std::streamsize p = line_.precision(n);
  return p;
}

//----------------------------------------------------------------
// the_log_t::flags
// 
std::ios::fmtflags
the_log_t::flags() const
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  std::ios::fmtflags f = line_.flags();
  return f;
}

//----------------------------------------------------------------
// the_log_t::flags
// 
std::ios::fmtflags
the_log_t::flags(std::ios::fmtflags fmt)
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  std::ios::fmtflags f = line_.flags(fmt);
  return f;
}

//----------------------------------------------------------------
// the_log_t::setf
// 
void
the_log_t::setf(std::ios::fmtflags fmt)
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  line_.setf(fmt);
}

//----------------------------------------------------------------
// the_log_t::setf
// 
void
the_log_t::setf(std::ios::fmtflags fmt, std::ios::fmtflags msk)
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  line_.setf(fmt, msk);
}

//----------------------------------------------------------------
// the_log_t::unsetf
// 
void
the_log_t::unsetf(std::ios::fmtflags fmt)
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  line_.unsetf(fmt);
}

//----------------------------------------------------------------
// the_log_t::copyfmt
// 
void
the_log_t::copyfmt(std::ostream & ostm)
{
  the_lock_t<the_mutex_interface_t> lock(mutex_);
  line_.copyfmt(ostm);
}


//----------------------------------------------------------------
// null_log
// 
the_null_log_t *
null_log()
{
  static the_null_log_t * log = NULL;
  if (log == NULL)
  {
    log = new the_null_log_t;
  }
  
  return log;
}


//----------------------------------------------------------------
// cerr_log
// 
the_stream_log_t *
cerr_log()
{
  static the_stream_log_t * log = NULL;
  if (log == NULL)
  {
    log = new the_stream_log_t(std::cerr);
  }
  
  return log;
}


//----------------------------------------------------------------
// cout_log
// 
the_stream_log_t *
cout_log()
{
  static the_stream_log_t * log = NULL;
  if (log == NULL)
  {
    log = new the_stream_log_t(std::cout);
  }
  
  return log;
}
