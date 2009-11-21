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

#ifndef APPLICATION_ACTION_ACTIONCONTEXT_H
#define APPLICATION_ACTION_ACTIONCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace Seg3D {

class ActionContext;
typedef boost::shared_ptr<ActionContext> ActionContextHandle;

// CLASS ACTIONCONTEXT:
// The action context contains all the information for the action to relay
// information back to the point of origin. As this type of information is not
// cached in provenance or in session files, the ActionContext is a separate
// entity with a pointer to the Action which describes the action that needs
// to be done. Action itself contains the pointers to the functions that need
// to be executed and with which parameters, but not where to report errors,
// the source of the action ect. The latter information is contained in the
// ActionContext. Each source needs to derive its own ActionContext from this
// class and generate the specifics of where information needs to be relayed
// to.

class ActionContext : public boost::noncopyable {

// -- Constructor/destructor --
  public:
    // Wrap a context around an action
    ActionContext();
 
    // Virtual destructor for memory management
    virtual ~ActionContext();

// -- Reporting functions --

    virtual void report_error(const std::string& error);
    virtual void report_warning(const std::string& warning);
    virtual void report_message(const std::string& message);

    virtual void report_progress(double progress);
    virtual void report_done(bool success);

// -- Source information --

    virtual bool from_script() const;
    virtual bool from_interface() const;
    
};

} // end namespace Seg3D

#endif
