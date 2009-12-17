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

#ifndef APPLICTION_ACTION_ACTIONS_ACTIONREDO_H
#define APPLICTION_ACTION_ACTIONS_ACTIONREDO_H

#include <Application/Action/Actions.h>

namespace Seg3D {

class ActionRedo : public Action {
    SCI_ACTION_TYPE("Redo","Redo",APPLICATION_E);

// -- Constructor/Destructor --
  public:
    ActionRedo() 
    {
    }
    
    virtual ~ActionRedo() 
    {}  

// -- Functions that describe action --
    virtual bool validate(ActionHandle& self,
                          ActionContextHandle& context);
    virtual bool run(ActionHandle& self,
                     ActionContextHandle& context);

};

typedef boost::shared_ptr<ActionRedo> ActionRedoHandle;

} // end namespace Seg3D

#endif
