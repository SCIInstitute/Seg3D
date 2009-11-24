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
ToolFactory::list_tool_types(tool_properties_list_type& tool_types_list) const
{
  // clear the list
  tool_types_list.clear();
  properties_map_type::const_iterator it = tool_properties_.begin();

  // loop through all the tools
  while (it != tool_properties_.end())
  {
    tool_properties_pair_type properties_pair = 
      make_pair((*it).first,(*it).second);
    ++it;
    tool_types_list.push_back(properties_pair);
  }
  if (tool_types_list.size() == 0) return (false);
  return (true);
}
    
    
bool 
ToolFactory::list_tool_types_with_interface(tool_properties_list_type& tool_types_list) const
{
  // clear the list
  tool_types_list.clear();
  properties_map_type::const_iterator it = tool_properties_.begin();

  // loop through all the tools
  while (it != tool_properties_.end())
  {
    tool_properties_pair_type properties_pair = 
      make_pair((*it).first,(*it).second);
    ++it;
    // check whether the interface exists
    if (toolinterface_builders_.find((*it).first) == toolinterface_builders_.end())
    {
      tool_types_list.push_back(properties_pair);
    }
  }
  if (tool_types_list.size() == 0) return (false);
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
  properties_map_type::const_iterator prop_it = tool_properties_.find(tool_type);
  tool->set_properties((*prop_it).second);
  
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
