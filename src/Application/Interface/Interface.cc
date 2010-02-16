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

namespace Seg3D {


Interface::Interface() :
  context_(new InterfaceActionContext)
{
}


Interface::~Interface()
{
}


InterfaceActionContextHandle
Interface::interface_action_context()
{
  return context_;
}


void
Interface::PostEvent(boost::function<void ()> function)
{
  Instance()->post_event(function);
}


void
Interface::PostAndWaitEvent(boost::function<void ()> function)
{
  Instance()->post_and_wait_event(function);
}

void 
Interface::PostAction(ActionHandle action)
{
  ActionDispatcher::Instance()->post_action(action,
    ActionContextHandle(Interface::Instance()->interface_action_context()));
}


// Singleton instance
Utils::Singleton<Interface> Interface::instance_;

} // end namespace Seg3D
