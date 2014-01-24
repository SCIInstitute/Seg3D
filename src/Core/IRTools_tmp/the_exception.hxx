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


// File         : the_exception.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Sep 24 18:06:00 MDT 2006
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : an exception convenience class

#ifndef THE_EXCEPTION_HXX_
#define THE_EXCEPTION_HXX_

// system includes:
#include <exception>
#include <string>
#include <sstream>


//----------------------------------------------------------------
// the_exception_t
// 
class the_exception_t : public std::exception
{
public:
  the_exception_t(const char * description = NULL,
		  const char * file = NULL,
		  const unsigned int & line = 0)
  {
    std::ostringstream os;
    
    if (file != NULL)
    {
      os << file << ':' << line << " -- ";
    }
    
    if (description != NULL)
    {
      os << description;
    }
    
    what_ = os.str();
  }
  
  virtual ~the_exception_t() throw ()
  {}
  
  // virtual:
  const char * what() const throw()
  { return what_.c_str(); }
  
  // data:
  std::string what_;
};


#endif // THE_EXCEPTION_HXX_
