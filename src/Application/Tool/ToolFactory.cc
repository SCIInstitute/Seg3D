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

// STL includes
#include <algorithm>

// Application includes
#include <Application/Tool/ToolFactory.h>

namespace Seg3D {

ToolFactory::ToolFactory()
{
}

bool 
ToolFactory::is_tool_type(const std::string& tool_type)
{
  boost::unique_lock<boost::mutex> lock(tools_mutex_);
  tool_map_type::const_iterator it = tools_.find(Utils::string_to_lower(tool_type));
  if (it == tools_.end()) return (false);
  return (true);
}

bool LessToolList(ToolFactory::tool_list_type::value_type val1,
                  ToolFactory::tool_list_type::value_type val2)
{
  return (val1->menu_name() < val2->menu_name());
}
    
bool 
ToolFactory::list_tool_types(tool_list_type& tool_list, int properties)
{
  boost::unique_lock<boost::mutex> lock(tools_mutex_);

  // clear the list
  tool_list.clear();
  
  tool_map_type::const_iterator it = tools_.begin();

  // loop through all the tools
  while (it != tools_.end())
  {
    if (((*it).second->properties() & properties) == properties)
    {
      tool_list.push_back((*it).second);
    }
    ++it;
  }
  
  if (tool_list.size() == 0) return (false);
  std::sort(tool_list.begin(),tool_list.end(),LessToolList);
  
  return (true);  
}

bool 
ToolFactory::list_tool_types_with_interface(tool_list_type& tool_list, int properties)
{
  boost::unique_lock<boost::mutex> lock(toolinterfaces_mutex_);
  boost::unique_lock<boost::mutex> lock2(tools_mutex_);

  // clear the list
  tool_list.clear();
  
  tool_map_type::const_iterator it = tools_.begin();

  // loop through all the tools
  while (it != tools_.end())
  {
    if (((*it).second->properties() & properties) == properties)
    {
      if (toolinterfaces_.find((*it).first) != toolinterfaces_.end())
      {
        tool_list.push_back((*it).second);
      }
    }
    ++it;
  }
  
  if (tool_list.size() == 0) return (false);
  std::sort(tool_list.begin(),tool_list.end(),LessToolList);
  
  return (true);  
}


bool 
ToolFactory::create_tool(const std::string& tool_type,
                         const std::string& toolid,
                         ToolHandle& tool)
{
  boost::unique_lock<boost::mutex> lock(tools_mutex_);

  // Step (1): find the tool
  tool_map_type::const_iterator it = tools_.find(Utils::string_to_lower(tool_type));

  // Step (2): check its existence
  if (it == tools_.end()) return (false);

  // Step (3): build the tool
  tool = (*it).second->builder()->build(toolid);
    
  return (true);
}


bool 
ToolFactory::create_toolinterface(const std::string& toolinterface_name,
                              ToolInterface*& toolinterface)
{
  boost::unique_lock<boost::mutex> lock(toolinterfaces_mutex_);

  // Step (1): find the tool
  toolinterface_map_type::const_iterator it = toolinterfaces_.find(toolinterface_name);

  // Step (2): check its existence
  if (it == toolinterfaces_.end())
  {
    SCI_THROW_LOGICERROR(std::string("Trying to instantiate tool '")
                              +toolinterface_name +"' that does not exist");
  }

  // Step (3): build the tool
  toolinterface = (*it).second->build();
  
  return (true);
}

// Singleton interface needs to live somewhere
Utils::Singleton<ToolFactory> ToolFactory::instance_;

} // end namespace seg3D
