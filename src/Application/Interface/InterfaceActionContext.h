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

#ifndef APPLICATION_INTERFACE_INTERFACEACTIONCONTEXT_H
#define APPLICATION_INTERFACE_INTERFACEACTIONCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/signals2/signal.hpp>

// Core includes
#include <Utils/Core/Log.h>

// Action includes
#include <Application/Action/ActionContext.h>

namespace Seg3D {
 
// CLASS INTERFACEACTIONCONTEXT:

class InterfaceActionContext;
typedef boost::shared_ptr<InterfaceActionContext> InterfaceActionContextHandle;
 
// Class defintion
class InterfaceActionContext : public ActionContext {

// -- Constructor/destructor --
  public:
    // Wrap a context around an action
    InterfaceActionContext();
 
    // Virtual destructor for memory management
    virtual ~InterfaceActionContext();

// -- Reporting functions --
  public:
    virtual void report_error(const std::string& error);
    virtual void report_warning(const std::string& warning);
    virtual void report_message(const std::string& message);
  
// -- Report back status and results --
  public:
    virtual void report_need_resource(ResourceLockHandle& resource);

// -- Report that action was done --
  public:
    virtual void report_done();
    
// -- Source/Status information --
  public: 
    virtual ActionSource source() const { return ACTION_SOURCE_INTERFACE_E; }
  
// -- Signals --  
  public:
    // ACTION_MESSAGE_SIGNAL:
    // This signal is given when a message is posted from the interface action
    // context class
    typedef boost::signals2::signal<void (int, std::string)> action_message_signal_type;
    action_message_signal_type action_message_signal_;
  
    // ACTION_DONE_SIGNAL:
    // This signal is raised when an action returns done
    typedef boost::signals2::signal<void (ActionStatus)> action_done_signal_type;
    action_done_signal_type action_done_signal_;
    
};

} // end namespace Seg3D

#endif
