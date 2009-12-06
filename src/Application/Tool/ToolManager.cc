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
#include <Application/Tool/ToolFactory.h>

#include <Application/Tool/Actions/ActionOpenTool.h>
#include <Application/Tool/Actions/ActionCloseTool.h>
#include <Application/Tool/Actions/ActionActivateTool.h>

namespace Seg3D {

ToolManager::ToolManager() :
  StateHandler("ToolManager")
{
}

// THREAD-SAFETY:
// The RunActionFromInterface function will migrate the function call to the
// application thread. Hence there is no need to safe guard this function.
// This code can be run directly form the interface thread.

void
ToolManager::dispatch_opentool(const std::string& tool_name) const
{
  // Build new action
  ActionOpenToolHandle action(new ActionOpenTool);
  // Set the action parameters
  action->set(tool_name);
  // Run the action on the application thread
  PostActionFromInterface(action);
}

void
ToolManager::dispatch_closetool(const std::string& toolid) const
{
  // Build new action
  ActionCloseToolHandle action(new ActionCloseTool);
  // Set the action parameters
  action->set(toolid);
  // Run the action on the application thread
  PostActionFromInterface(action);
}

void
ToolManager::dispatch_activatetool(const std::string& toolid) const
{
  // Build new action
  ActionActivateToolHandle action(new ActionActivateTool);
  // Set the action parameters
  action->set(toolid);
  // Run the action on the application thread
  PostActionFromInterface(action);
}


// THREAD-SAFETY:
// Only ActionOpenTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

bool
ToolManager::open_tool(const std::string& toolid ,std::string& new_toolid)
{
  // Lock the list
  boost::unique_lock<boost::recursive_mutex> lock(tool_list_lock_);

  // Check whether an id number was attached
  std::string tool_type = toolid;
  std::string::size_type loc = tool_type.find('_');
  if (loc != std::string::npos) 
  {
    tool_type = tool_type.substr(0,loc);
    // ToolID was already assigned in the action
    // and hence we do not need to generate a new one
    new_toolid = toolid;
  }
  else
  {
    // No final toolID was assigned and a new one has to be made
    new_toolid = create_toolid(tool_type);
  }

  SCI_LOG_DEBUG(std::string("Creating tool: ")+new_toolid+" of type "+tool_type);
    
  // Step (2): Build the tool using the factory. This will generate the default
  // settings.
  ToolHandle tool;
  
  if (!(ToolFactory::Instance()->create_tool(tool_type,new_toolid,tool)))
  {
    SCI_LOG_ERROR(std::string("Could not create tool of type: '")+tool_type+"'");
    return (false);
  }
  
  // Step (3): Add the tool id to the tool and add the tool to the list
  {
    tool_list_[new_toolid] = tool;
  }
    
  SCI_LOG_DEBUG(std::string("Open tool: ")+new_toolid);
  // Step (4): Signal any observers (UIs) that the tool has been opened  
  open_tool_signal_(tool);

  return (true);
}

// THREAD-SAFETY:
// Only ActionCloseTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

void
ToolManager::close_tool(const std::string& toolid)
{
  boost::unique_lock<boost::recursive_mutex> lock(tool_list_lock_);

  // Step (1): Find the tool in the list.
  tool_list_type::iterator it = tool_list_.find(toolid);
  if (it == tool_list_.end()) 
  {
    SCI_LOG_ERROR(std::string("Toolid '"+toolid+"' does not exist"));
    return;
  }
      
   // Step (2): Ensure that the tool is not the active tool
  if (toolid == active_toolid_)
  {
    // Call the tool deactivate function that will unregister the current
    // tool bindings
    (*it).second->deactivate();
    
    // Set no tool as active
    active_toolid_ = "";
  }

 
  // Step (3): Move the tool from the list. The tool handle still persists
  // and will be removed after the signal has been posted.

  tool_list_.erase(it);

  // Step (4): Run the function in the tool that cleans up the parts that
  // need to be cleaned up on the interface thread.
  (*it).second->close();  

  // Step (5): Close any of the registered connections
  (*it).second->close_connections();      

  SCI_LOG_DEBUG(std::string("Close tool: ")+toolid);
  
  // Step (6): Signal that the tool will be closed.   
  close_tool_signal_((*it).second);
}

// THREAD-SAFETY:
// Only ActionActivateTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

void
ToolManager::activate_tool(const std::string& toolid)
{
  // Lock access to tool_list
  boost::unique_lock<boost::recursive_mutex> lock(tool_list_lock_);

  // Check if anything needs to be done
  if (toolid == active_toolid_) return;

  SCI_LOG_DEBUG(std::string("Activate tool: ")+toolid);

  // Step (1): Find the tool in the list.
  tool_list_type::iterator it = tool_list_.find(active_toolid_);

  // Step (2): Deactivate tool if found
  if (it != tool_list_.end())
  {
    (*it).second->deactivate();
  }
  
  // Step (3): Find new active tool
  it = tool_list_.find(toolid);
  
  // Step (4): Activate tool if found
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
  
  // signal for interface
  activate_tool_signal_((*it).second);
}


ToolManager::tool_list_type
ToolManager::tool_list()
{
  boost::unique_lock<boost::recursive_mutex> lock(tool_list_lock_);
  return tool_list_;
}

std::string
ToolManager::active_toolid()
{
  boost::unique_lock<boost::recursive_mutex> lock(tool_list_lock_);
  return active_toolid_;
}

void
ToolManager::lock_tool_list()
{
  tool_list_lock_.lock();
}

void
ToolManager::unlock_tool_list()
{
  tool_list_lock_.unlock();
}

void
ToolManager::add_toolid(const std::string& toolid)
{
  boost::unique_lock<boost::mutex> lock(toolid_list_lock_);
  toolid_list_.insert(toolid);
}        

void
ToolManager::remove_toolid(const std::string& toolid)
{
  boost::unique_lock<boost::mutex> lock(toolid_list_lock_);
  toolid_list_.erase(toolid);
}

bool
ToolManager::is_toolid(const std::string& toolid)
{
  boost::unique_lock<boost::mutex> lock(toolid_list_lock_);
  if(toolid_list_.find(toolid) != toolid_list_.end()) return (true);
  return (false);
}

std::string 
ToolManager::create_toolid(const std::string& tool_type)
{
  boost::unique_lock<boost::mutex> lock(toolid_list_lock_);

  int num = 0;
  std::string toolid;

  do 
  {
    toolid = tool_type+std::string("_")+Utils::to_string(num);
    num++;
  } 
  while(toolid_list_.find(toolid) != toolid_list_.end());
  
  return (toolid);
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<ToolManager> ToolManager::instance_;
  
} // end namespace Seg3D
