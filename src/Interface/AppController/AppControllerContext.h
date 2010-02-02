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

#ifndef INTERFACE_APPCONTROLLER_APPCONTROLLERCONTEXT_H
#define INTERFACE_APPCONTROLLER_APPCONTROLLERCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <QPointer>

// Include all action related classes
#include <Application/Action/Actions.h>

// Include interface code
#include <Interface/AppController/AppController.h>

namespace Seg3D {

class AppControllerContext;
typedef boost::shared_ptr<AppControllerContext> AppControllerContextHandle;

class AppControllerContext : public ActionContext {

  public:
    AppControllerContext(AppController* controller);
    virtual ~AppControllerContext();

    virtual void report_error(const std::string& error);
    virtual void report_warning(const std::string& warning);
    virtual void report_message(const std::string& message);
    virtual void report_usage(const std::string& usage);

    virtual void report_done(bool success);
    
    // Run from the command line as if it is a script
    virtual bool from_script() const;

  private:
    // To which controller does the action information need to be relayed
    AppController::qpointer_type controller_;
};

} //end namespace Seg3D

#endif
