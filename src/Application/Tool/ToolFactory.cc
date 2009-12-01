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

#include <Application/Tool/ToolFactory.h>

namespace Seg3D {

ToolFactory::ToolFactory()
{
}

bool 
ToolFactory::is_tool_type(const std::string& tool_type) const
{
  tool_map_type::const_iterator it = tool_builders_.find(tool_type);
  if (it == tool_builders_.end()) return (false);
  return (true);
}
    
bool 
ToolFactory::list_tool_types(tool_list_type& tool_list, int tool_group) const
{
  // clear the list
  tool_list.clear();
  group_map_type::const_iterator it = tool_group_.begin();

  // loop through all the tools
  while (it != tool_group_.end())
  {
    if ((*it).second == tool_group) tool_list.push_back((*it).first);
    ++it;
  }
  
  if (tool_list.size() == 0) return (false);
  return (true);  
}

bool 
ToolFactory::create_tool(const std::string& tool_type,
                         const std::string& toolid,
                         ToolHandle& tool) const
{
  // Step (1): find the tool
  tool_map_type::const_iterator it = tool_builders_.find(tool_type);

  // Step (2): check its existence
  if (it == tool_builders_.end())
  {
    SCI_THROW_LOGICERROR(std::string("Trying to instantiate tool '")
                                    +tool_type +"'that does not exist");
  }

  // Step (3): build the tool
  tool = (*it).second->build(toolid);
  
  // Step (4): insert the type_name and the properties into the tool
  tool->set_tool_type(tool_type);
  group_map_type::const_iterator group_it = tool_group_.find(tool_type);
  tool->set_tool_group((*group_it).second);
  
  return (true);
}


bool 
ToolFactory::create_toolinterface(const std::string& toolinterface_name,
                              ToolInterfaceHandle& toolinterface) const
{
  // Step (1): find the tool
  toolinterface_map_type::const_iterator it = toolinterface_builders_.find(toolinterface_name);

  // Step (2): check its existence
  if (it == toolinterface_builders_.end())
  {
    SCI_THROW_LOGICERROR(std::string("Trying to instantiate tool '")
                                    +toolinterface_name +"'that does not exist");
  }

  // Step (3): build the tool
  toolinterface = (*it).second->build();
  
  return (true);
}

// Singleton interface needs to live somewhere
Utils::Singleton<ToolFactory> ToolFactory::instance_;

} // end namespace seg3D
