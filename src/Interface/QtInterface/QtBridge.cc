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

#include <Interface/QtInterface/QtBridge.h>
#include <Interface/QtInterface/QtBridgeInternal.h>

namespace Seg3D {

// -- Checkbox connector --

void QtCheckBoxSignal(QPointer<QCheckBox> qobject_ptr, bool state)
{
  if (!(Interface::Instance()->IsInterfaceThread()))
  {
    PostInterface(boost::bind(&QtCheckBoxSignal,qobject_ptr,state));
    return;
  }

  if (qobject_ptr.data()) qobject_ptr->setChecked(state); 
}

bool
QtBridge::connect(QCheckBox* qcheckbox, 
                     StateValue<bool>::Handle& state_handle)
{
  // Connect the dispatch into the StateVariable (with auxillary object)
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtCheckBoxSlot(qcheckbox,state_handle);
    
  // Connect the state signal back into the Qt Variable  
  state_handle->connect(boost::bind(&QtCheckBoxSignal,qcheckbox,_1));
  
  return (true);
}
  
bool
QtBridge::connect(QToolButton* qtoolbutton, 
                  boost::function<void ()> function)
{
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtToolButtonSlot(qtoolbutton,function);
  
  return (true);
}

// -- Tool menu connector --

bool
QtBridge::connect(QAction* qaction, 
                     boost::function<void ()> function)
{
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionSlot(qaction,function);

  return (true);
}


} // end namespace Seg3D
