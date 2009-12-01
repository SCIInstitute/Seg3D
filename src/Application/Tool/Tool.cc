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
  StateHandler(std::string("ToolManager::")+toolid),
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
Tool::close()
{

}

void 
Tool::add_connection(boost::signals2::connection connection)
{
  connection_list_.push_back(connection);
}

void
Tool::close_connections()
{  
  // Delete all the connections
  connection_list_type::iterator it = connection_list_.begin();
  connection_list_type::iterator it_end = connection_list_.end();
  while (it !=  it_end)
  {
    (*it).disconnect();
    ++it;
  }
  
  // Clear the list
  connection_list_.clear();
}

void
Tool::activate()
{
  // Defaults to doing nothing
}

void
Tool::deactivate()
{
  // Defaults to doing nothing
}

}
