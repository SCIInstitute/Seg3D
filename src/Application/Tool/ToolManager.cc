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

#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>

#include <Application/Tool/ToolManager.h>

#include <Application/Tool/Actions/ActionOpenTool.h>
#include <Application/Tool/Actions/ActionCloseTool.h>

namespace Seg3D {

ToolManager::ToolManager()
{
}

ToolManager::~ToolManager()
{
}

// THREAD-SAFETY:
// The RunActionFromInterface function will migrate the function call to the
// application thread. Hence there is no need to safe guard this action.
// This code can be run directly form the interface thread.

void
ToolManager::dispatch_opentool(const std::string& tool_name) const
{
  // Build new action
  ActionOpenToolHandle action(new ActionOpenTool);
  // Set the action parameters
  action->set(tool_name,"");
  // Run the action on the application thread
  RunActionFromInterface(action);
}

void
ToolManager::dispatch_closetool(const std::string& toolid) const
{
  // Build new action
  ActionCloseToolHandle action(new ActionCloseTool);
  // Set the action parameters
  action->set(toolid);
  // Run the action on the application thread
  RunActionFromInterface(action);
}


// THREAD-SAFETY:
// Only ActionOpenTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

void
ToolManager::open_tool(const std::string& toolname, const std::string& toolid)
{
  // Step (1): If no tool id was supplied, create a new unique one
  if (toolid == "") toolid = create_new_tool_id(toolname);
  
  // Step (2): Build the tool using the factory. This will generate the default
  // settings.
  ToolHandle tool;
  if (!(ToolFactory::instance()->create_tool(toolname,tool)))
  {
    SCI_LOG_ERROR(std::string("Could not create tool: '")+toolname+"'");
    return;
  }
  
  // Step (3): Add the tool id to the tool and add the tool to the list
  tool->set_toolid(toolid);
  tool_list_[toolid] = tool;
    
  // Step (4): Signal any observers (UIs) that the tool has been opened  
  open_tool_signal_(tool);
}

void
ToolManager::close_tool(const std::string& toolid)
{
  ToolHandle tool;
 
  // Step (1): Find the tool in the list
  tool_list_type::iterator it = tool_list_.find(toolid);
  if (it == tool_list_.end()) 
  {
    SCI_LOG_ERROR(std::string("Toolid '"+toolid+"' does not exist"));
    return;
  }
    
  // Step (2): Reset the toolid so it is marked as a tool that is no longer
  // available.
   
  tool->set_toolid("");  
    
  // Step (3): Signal that the tool will be closed.   
  close_tool_signal_(tool);
  
  // Step (4): Remove the tool from the current list
  tool_list_.remove(toolid);
}

} // end namespace Seg3D
