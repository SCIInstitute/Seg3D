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

#include <Application/ToolManager/ToolManager.h>
#include <Application/ToolManager/Actions/ActionCloseTool.h>

namespace Seg3D {

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
SCI_REGISTER_ACTION(CloseTool);


bool
ActionCloseTool::validate( ActionContextHandle& context )
{
  if (!(StateEngine::Instance()->is_stateid(toolid_.value())))
  {
    context->report_error(std::string("ToolID '")+toolid_.value()+"' is invalid");
    return false;
  }
  
  return true; // validated
}


bool 
ActionCloseTool::run( ActionContextHandle& context, ActionResultHandle& result )
{
  ToolManager::Instance()->close_tool(toolid_.value());
  return true; // success
}


ActionHandle
ActionCloseTool::Create( const std::string& toolid )
{
  // Create new action
  ActionCloseTool* action = new ActionCloseTool;

  // Set action parameters
  action->toolid_.value() = toolid;

  // Post the new action
  return ActionHandle(action);
}


void
ActionCloseTool::Dispatch( const std::string& toolid )
{
  Interface::PostAction(Create(toolid));
}

} // end namespace Seg3D
