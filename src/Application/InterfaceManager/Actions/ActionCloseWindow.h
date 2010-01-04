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

#ifndef APPLICATION_INTERFACEMANAGER_ACTIONS_ACTIONCLOSETOOL_H
#define APPLICATION_INTERFACEMANAGER_ACTIONS_ACTIONCLOSETOOL_H

#include <Application/Action/Actions.h>

namespace Seg3D {

class ActionCloseWindow : public Action {
  SCI_ACTION_TYPE("CloseWindow","CloseWindow windowid",INTERFACE_E)

// -- Constructor/Destructor --
  public:
    ActionCloseWindow()
    {
      add_argument(windowid_);
    }
    
    virtual ~ActionCloseWindow() 
    {}

    void set(const std::string& windowid)
    {
      windowid_.value() = windowid;
    }

// -- Functions that describe action --
    virtual bool validate(ActionContextHandle& context);
    virtual bool run(ActionContextHandle& context,
                     ActionResultHandle& result);
    
// -- Action parameters --
    ActionParameter<std::string> windowid_;
};

typedef boost::intrusive_ptr<ActionCloseWindow> ActionCloseWindowHandle;

} // end namespace Seg3D

#endif
