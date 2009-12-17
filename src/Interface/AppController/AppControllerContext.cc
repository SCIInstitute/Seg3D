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

#include <Application/Interface/Interface.h>

#include <Interface/AppController/AppControllerContext.h>

namespace Seg3D {

AppControllerContext::AppControllerContext(AppController* controller) :
  controller_(controller)
{
}

AppControllerContext::~AppControllerContext()
{
}

void
AppControllerContext::report_error(const std::string& error)
{
  AppController::PostMessage(controller_,error);
}

void
AppControllerContext::report_warning(const std::string& warning)
{
  AppController::PostMessage(controller_,warning);
}

void
AppControllerContext::report_message(const std::string& message)
{
  AppController::PostMessage(controller_,message);
}

void
AppControllerContext::report_usage(const std::string& usage)
{
  AppController::PostUsage(controller_,usage);
}

void
AppControllerContext::report_done(bool success)
{
  if (success) AppController::PostMessage(controller_,"");
}

bool
AppControllerContext::from_script() const
{
  return (true);
}

} //end namespace Seg3D
