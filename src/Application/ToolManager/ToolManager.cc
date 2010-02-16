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

#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>

#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/ToolManager/Actions/ActionCloseTool.h>
#include <Application/ToolManager/Actions/ActionActivateTool.h>

namespace Seg3D {

ToolManager::ToolManager() :
  StateHandler("ToolManager")
{
}

ToolManager::~ToolManager()
{
  disconnect_all();
}

// THREAD-SAFETY:
// Only ActionOpenTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

bool
ToolManager::open_tool(const std::string& toolid ,std::string& new_toolid)
{
  // Step (1): Make the function thread safe
  lock_type lock(tool_list_mutex_);

  // Step (2): Add an entry in the debug log
  SCI_LOG_DEBUG(std::string("Open tool: ")+toolid);

  // STEP (3): Create a new toolid and extract the tool type from the string
  if (!( StateEngine::Instance()->create_stateid(toolid,new_toolid)))
  {
    SCI_LOG_ERROR(std::string("Could not create tool  '")+toolid+"'");
    return (false);  
  }

  std::string::size_type loc = new_toolid.find('_');  
  std::string tool_type = new_toolid.substr(0,loc);

  // Step (4): Build the tool using the factory. This will generate the default
  // settings.
  ToolHandle tool;
  
  if (!(ToolFactory::Instance()->create_tool(tool_type,new_toolid,tool)))
  {
    SCI_LOG_ERROR(std::string("Could not create tool of type: '")+tool_type+"'");
    return (false);
  }
  
  // Step (5): Add the tool id to the tool and add the tool to the list
  {
    tool_list_[new_toolid] = tool;
  }
    
  SCI_LOG_DEBUG(std::string("Open tool: ")+new_toolid);
  
  // Step (6): Signal any observers (UIs) that the tool has been opened  
  open_tool_signal_(tool);

  // All done
  return true;
}

// THREAD-SAFETY:
// Only ActionCloseTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

void
ToolManager::close_tool(const std::string& toolid)
{
  // Step (1): Make the function thread safe
  lock_type lock(tool_list_mutex_);

  // Step (2): Add an entry in the debug log
  SCI_LOG_DEBUG(std::string("Close tool: ")+toolid);

  // Step (3): Find the tool in the list.
  tool_list_type::iterator it = tool_list_.find(toolid);
  if (it == tool_list_.end()) 
  {
    SCI_LOG_ERROR(std::string("Toolid '"+toolid+"' does not exist"));
    return;
  }

  // Step (4): Get the tool from the iterator 
  ToolHandle tool = (*it).second;
  if (toolid == active_toolid_)
  {
    // Call the tool deactivate function that will unregister the current
    // tool bindings
    tool->deactivate();
    
    // Set no tool as active
    active_toolid_ = "";
  }

  // Step (5): Move the tool from the list. The tool handle still persists
  // and will be removed after the signal has been posted.
  tool_list_.erase(it);

  // Step (6): Run the function in the tool that cleans up the parts that
  // need to be cleaned up on the interface thread.
  tool->close();  

  // Step (7): Signal that the tool will be closed.   
  close_tool_signal_(tool);
}

// THREAD-SAFETY:
// Only ActionActivateTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

void
ToolManager::activate_tool(const std::string& toolid)
{
  // Step (1): Make the function thread safe
  lock_type lock(tool_list_mutex_);

  // Step (2): Add an entry in the debug log
  SCI_LOG_DEBUG(std::string("Activate tool: ")+toolid);

  // Step (3): Check if anything needs to be done
  if (toolid == active_toolid_) return;

  // Step (4): Deactivate tool if found
  tool_list_type::iterator it = tool_list_.find(active_toolid_);
  if (it != tool_list_.end())
  {
    (*it).second->deactivate();
  }
  
  // Step (5): Find new active tool and activate it
  it = tool_list_.find(toolid);
  if (it != tool_list_.end())
  {
    (*it).second->activate();
    active_toolid_ = toolid;
  }
  else
  {
    active_toolid_ = "";
    return;
  }
  
  // Step (4): signal for interface
  activate_tool_signal_((*it).second);
}


ToolManager::tool_list_type
ToolManager::tool_list()
{
  lock_type lock(tool_list_mutex_);
  return tool_list_;
}


std::string
ToolManager::active_toolid()
{
  lock_type lock(tool_list_mutex_);
  return active_toolid_;
}


ToolManager::mutex_type&
ToolManager::get_mutex()
{
  return tool_list_mutex_;
}


// Singleton interface needs to be defined somewhere
Utils::Singleton<ToolManager> ToolManager::instance_;
  
} // end namespace Seg3D
