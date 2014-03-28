/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
 University of Utah.
 
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

// File         : the_terminator.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Sep 24 18:05:00 MDT 2006
// Copyright    : (C) 2004-2008 University of Utah
// Description  : a thread terminator convenience class

#ifndef THE_TERMINATOR_HXX_
#define THE_TERMINATOR_HXX_

#if defined(USE_THE_TERMINATORS) || defined(USE_ITK_TERMINATORS)
#define WRAP(x) x
#else
#define WRAP(x)
#endif

#include <Core/Utils/Log.h>

// system includes:
#include <string>
#include <list>

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_thread_storage.hxx>
#include <Core/ITKCommon/the_utils.hxx>


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
    // TODO: this code looks sketchy - is it actually needed?

    // if (termination_requested_ || should_terminate_immediately())
    if (termination_requested_)
    {
      CORE_LOG_MESSAGE("Termination requested in the_terminator_t class");
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
