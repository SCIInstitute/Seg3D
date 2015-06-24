/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ACTION_ACTIONCONTEXTCONTAINER_H
#define CORE_ACTION_ACTIONCONTEXTCONTAINER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Core includes
#include <Core/Action/ActionContext.h>

namespace Core
{


class ActionContextContainer : public ActionContext
{

  // -- Constructor/destructor --
public:
  // Wrap a context around an action
  ActionContextContainer( ActionContextHandle context );

  // Virtual destructor for memory management
  virtual ~ActionContextContainer();

  // -- Reporting functions --
public:
  virtual void report_error( const std::string& error );
  virtual void report_warning( const std::string& warning );
  virtual void report_message( const std::string& message );

  // -- Report back status and results --
public:
  virtual void report_status( ActionStatus status );
  virtual void report_result( const ActionResultHandle& result );
  virtual void report_need_resource( NotifierHandle notifier );

  // -- Utilities
public:
  virtual Core::NotifierHandle get_resource_notifier();
  virtual void reset_context();
  virtual Core::ActionResultHandle get_result();

  // -- Report that action was done --
public:
  virtual void report_done();

  // -- Source/Status information --
public:
  virtual ActionSource source() const;

  // -- Status information --
protected:

  // The last status report from the action engine
  ActionContextHandle context_;
};

} // end namespace Core

#endif
