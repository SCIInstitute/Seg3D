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


#include <Application/ActionManager/ActionManagerHandler.h>


namespace Seg3D {

ActionManagerHandler::ActionManagerHandler()
{
}

void
ActionManagerHandler::handle_action_slot(ActionHandle action)
{
  SCI_LOG_DEBUG("Dispatching Action");

  if (action->test_property(Action::UNDOABLE_E))
  {
    if (action->test_property(Action::UPDATEABLE_E))
    {
      // get previous action and merge action 
    }
    else
    {
      action->build_undo();
    
      action->do_action();
    }
  }
  else
  {
    action->do_action();
  }
}

void
ActionManagerHandler::undo_action()
{
}

void
ActionManagerHandler::redo_action()
{
}

} // end namespace Seg3D
