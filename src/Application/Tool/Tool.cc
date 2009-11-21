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

#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tool/ToolManager.h>

namespace Seg3D {

Tool::Tool(const std::string& toolid) :
  toolid_(toolid)
{
  ToolManager::instance()->add_toolid(toolid);
}

Tool::~Tool()
{
  // This one is only removed when the class is removed
  // As the destruction of the class ensures that no new
  // actions are posted with this Tool as target.
  // Once the tool is unlinked all the actions that are still
  // being posted are removed by the validator. However the name
  // needs to blocked until the tool is really removed to prevent
  // a tool with the same name to be instantiated and actions
  // for multiple tools with the same name being mixed.
  ToolManager::instance()->remove_toolid(toolid_);
}

void
Tool::close_tool()
{
  // Remove all the pointers to the internal state of this tool from the
  // StateManager. This will inhibit any updates from being executed to
  // update the state of this tool.
  std::string tool_stateid = std::string("ToolManager::")+toolid_;
  StateManager::instance()->remove_state(tool_stateid);
}

bool
Tool::add_state(const std::string& key, StateBase* state) const
{
  std::string stateid = std::string("ToolManager::")+toolid_+std::string("::")+key;
  
  // Step (1): Make the state variable aware of its key
  state->set_stateid(stateid);

  // Step (2): Add the state to the StateManager
  return (StateManager::instance()->add_state(stateid,state));  
}

}
