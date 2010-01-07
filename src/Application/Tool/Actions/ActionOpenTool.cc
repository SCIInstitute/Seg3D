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

#include <Application/Tool/ToolManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tool/Actions/ActionOpenTool.h>

namespace Seg3D {

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
SCI_REGISTER_ACTION(OpenTool);

// VALIDATE:
// As the action could be user input, we need to validate whether the action
// is valid and can be executed.

bool
ActionOpenTool::validate(ActionContextHandle& context)
{
  // Check whether an id number was attached
  std::string tool_type = toolid_.value();
  std::string::size_type loc = tool_type.find('_');
  if (loc != std::string::npos) tool_type = tool_type.substr(0,loc);

  // Check whether the tool has a valid type
  if (!(ToolFactory::Instance()->is_tool_type(tool_type)))
  {
    context->report_error(std::string("No tool available of type '")+tool_type+"'");
    return (false);
  }

  // Check whether name does not exist, if it exists we have to report an
  // error.
  
  if (loc != std::string::npos)
  {
    if (!(ToolManager::Instance()->is_toolid(toolid_.value())))
    {
      context->report_error(std::string("ToolID '")+toolid_.value()+"' is already in use");
      return (false);
    }
  }
  
  return (true); // validated
}

// RUN:
// The code that runs the actual action

bool 
ActionOpenTool::run(ActionContextHandle& context, ActionResultHandle& result)
{
  std::string active_tool = ToolManager::Instance()->active_toolid();

  // Open and Activate the tool
  std::string new_tool_id;
  ToolManager::Instance()->open_tool(toolid_.value(),new_tool_id);  
  ToolManager::Instance()->activate_tool(new_tool_id);

  result = ActionResultHandle(new ActionResult(new_tool_id));
  return (true); // success
}

// DISPATCH:
// Dispatch this action with given parameters (from interface)

void
ActionOpenTool::Dispatch(const std::string& toolid)
{
  // Create new action
  ActionOpenTool* action = new ActionOpenTool;

  // Set action parameters
  action->toolid_.value() = toolid;

  // Post the new action
  PostActionFromInterface(ActionHandle(action));
}

} // end namespace Seg3D
