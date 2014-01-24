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


// File         : the_log.hxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Mar 23 10:34:12 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A text log object -- behaves almost like a std::ostream.

#ifndef THE_LOG_HXX_
#define THE_LOG_HXX_

// system includes:
#include <iosfwd>
#include <iostream>
#include <iomanip>
#include <sstream>

// TODO: temporary
#include <Core/IRTools_tmp/the_utils.hxx>
#include <Core/IRTools_tmp/the_mutex_interface.hxx>


//----------------------------------------------------------------
// the_log_t
// 
class the_log_t
{
protected:
  void log_no_lock(std::ostream & (*f)(std::ostream &));
  
public:
  the_log_t();
  virtual ~the_log_t();
  
  virtual the_log_t &
  operator << (std::ostream & (*f)(std::ostream &));
  
  template <typename data_t>
  the_log_t &
  operator << (const data_t & data)
  {
    the_lock_t<the_mutex_interface_t> lock(mutex_);
    line_ << data;
    return *this;
  }
  
  std::streamsize precision();
  std::streamsize precision(std::streamsize n);
  
  std::ios::fmtflags flags() const;
  std::ios::fmtflags flags(std::ios::fmtflags fmt);
  
  void setf(std::ios::fmtflags fmt);
  void setf(std::ios::fmtflags fmt, std::ios::fmtflags msk);
  void unsetf(std::ios::fmtflags fmt);
  
  void copyfmt(std::ostream & ostm);
  
  std::ostringstream line_;
  mutable the_mutex_interface_t * mutex_;
};


//----------------------------------------------------------------
// the_null_log_t
//
class the_null_log_t : public the_log_t
{
public:
  // virtual:
  the_log_t & operator << (std::ostream & (*)(std::ostream &))
  { return *this; }
  
  template <typename data_t>
  the_log_t & operator << (const data_t &)
  { return *this; }
};

//----------------------------------------------------------------
// the_stream_log_t
//
class the_stream_log_t : public the_log_t
{
public:
  the_stream_log_t(std::ostream & ostm):
    ostm_(ostm)
  {}
  
  // virtual:
  the_log_t & operator << (std::ostream & (*f)(std::ostream &))
  {
    the_lock_t<the_mutex_interface_t> lock(the_log_t::mutex_);
    the_log_t::log_no_lock(f);
    ostm_ << the_log_t::line_.str();
    the_log_t::line_.str("");
    return *this;
  }
  
  template <typename data_t>
  the_log_t & operator << (const data_t & data)
  { return the_log_t::operator << (data); }
  
  std::ostream & ostm_;
};


//----------------------------------------------------------------
// the_text_log_t
// 
class the_text_log_t : public the_log_t
{
public:
  // virtual:
  the_log_t & operator << (std::ostream & (*f)(std::ostream &))
  {
    the_lock_t<the_mutex_interface_t> lock(the_log_t::mutex_);
    the_log_t::log_no_lock(f);
    text_ += the_log_t::line_.str();
    the_log_t::line_.str("");
    return *this;
  }
  
  template <typename data_t>
  the_log_t & operator << (const data_t & data)
  { return the_log_t::operator << (data); }
  
  inline std::string text()
  { return text_; }
  
  std::string text_;
};

//----------------------------------------------------------------
// null_log
// 
extern the_null_log_t * null_log();

//----------------------------------------------------------------
// cerr_log
// 
extern the_stream_log_t * cerr_log();

//----------------------------------------------------------------
// cout_log
// 
extern the_stream_log_t * cout_log();


#endif // THE_LOG_HXX_
