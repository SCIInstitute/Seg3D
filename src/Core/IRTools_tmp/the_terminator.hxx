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


// File         : the_terminator.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Sep 24 18:05:00 MDT 2006
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : a thread terminator convenience class

#ifndef THE_TERMINATOR_HXX_
#define THE_TERMINATOR_HXX_

#if defined(USE_THE_TERMINATORS) || defined(USE_ITK_TERMINATORS)
#define WRAP(x) x
#else
#define WRAP(x)
#endif

// system includes:
#include <string>
#include <list>

// TODO: temporary
#include <Core/IRTools_tmp/the_utils.hxx>
#include <Core/IRTools_tmp/the_thread_storage.hxx>


//----------------------------------------------------------------
// the_terminator_t
// 
class the_terminator_t
{
public:
  the_terminator_t(const char * id);
  virtual ~the_terminator_t();
  
  // terminator id accessor:
  inline const std::string & id() const
  { return id_; }
  
  // read-only accessor to the termination request flag:
  inline const bool & termination_requested() const
  { return termination_requested_; }
  
  // this function may be called periodically from any time consuming
  // function -- in case the user has decided to terminate its execution:
  inline void terminate_on_request() const
  {
    // if (termination_requested_ || should_terminate_immediately())
    if (termination_requested_)
    {
      throw_exception();
    }
  }
  
  // this is a no-op for simple terminators, itk terminators will
  // override this to turn on the process AbortGenerateData flag:
  virtual void terminate();
  
  // this function will throw an exception:
  void throw_exception() const;
  
  // make sure there are no terminators left:
  static bool verify_termination();

protected:
  // consult the thread regarding whether termination has been requested
  // for all terminators in the current thread:
  static bool should_terminate_immediately();
  
  // a list of active terminators per current thread:
  static std::list<the_terminator_t *> & terminators();
  
  // add/remove a terminator to/from the list of active terminators
  // in the current thread:
  static void add(the_terminator_t * terminator);
  static void del(the_terminator_t * terminator);
  
  // id of this terminator:
  std::string id_;
  
  // flag indicating that termination was requested explicitly
  // for this terminator:
  bool termination_requested_;
};

//----------------------------------------------------------------
// the_terminators_t
// 
class the_terminators_t
{
public:
  virtual ~the_terminators_t();
  
  virtual void terminate();
  virtual bool verify_termination();
  
  virtual void add(the_terminator_t * terminator);
  virtual void del(the_terminator_t * terminator);
  
  // concurrent access controls:
  virtual void lock() = 0;
  virtual void unlock() = 0;
  
private:
  // the list of terminators:
  std::list<the_terminator_t *> terminators_;
};


#endif // THE_TERMINATOR_HXX_
