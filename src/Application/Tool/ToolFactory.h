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

#ifndef APPLICATION_TOOL_TOOLFACTORY_H
#define APPLICATION_TOOL_TOOLFACTORY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <vector>

// Boost includes
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Core/StringUtil.h>
#include <Utils/Singleton/Singleton.h>

// Application includes
#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolInterface.h>

namespace Seg3D {

// ---- Auxilary Classes -----

// TOOLBUILDER:
// This class is the base functor for the class that builds the classes in
// the factory

class ToolBuilderBase {
  public:
    // ensure we can delete the builder correctly
    virtual ~ToolBuilderBase() {}
    // the functor call to build the object
    virtual ToolHandle build(const std::string& toolid) = 0;
};

// TOOLBUILDERT:
// The actual instantiation that builds the tool of type TOOL. This class
// is loaded on top of the base functor and creates the action

template <class TOOL>
class ToolBuilder: public ToolBuilderBase {

  public:
    // ensure we can delete the builder correctly 
    virtual ~ToolBuilder<TOOL>() {}
    // The actual builder call
    virtual ToolHandle build(const std::string& toolid) { return ToolHandle(new TOOL(toolid)); }
};


// TOOLINTERFACEBUILDER:
// This class is the base functor for the class that builds the classes in
// the factory

class ToolInterfaceBuilderBase {
  public:
    // ensure we can delete the builder correctly
    virtual ~ToolInterfaceBuilderBase() {}
    // the functor call to build the object
    virtual ToolInterfaceHandle build() = 0;
};

// TOOLINTERFACEBUILDERT:
// The actual instantiation that builds the tool of type TOOLINTERFACE. This 
// class is loaded on top of the base functor and creates the action

template <class TOOLINTERFACE>
class ToolInterfaceBuilder: public ToolInterfaceBuilderBase {

  public:
    // ensure we can delete the builder correctly 
    virtual ~ToolInterfaceBuilder<TOOLINTERFACE>() {}
    // The actual builder call
    virtual ToolInterfaceHandle build() 
      { return ToolInterfaceHandle(new TOOLINTERFACE); }
};


class ToolInfo {
  public:
    ToolInfo(ToolBuilderBase* builder, int properties, std::string menu_name) :
      builder_(builder),
      properties_(properties),
      menu_name_(menu_name)
    {}
    
    // Copy constructor and default constructor are needed by the unordered_map
    // class
    ToolInfo(const ToolInfo& toolinfo) :
      builder_(toolinfo.builder_),
      properties_(toolinfo.properties_),
      menu_name_(toolinfo.menu_name_)
    {}
    
    ToolInfo() :
      builder_(0),
      properties_(0),
      menu_name_("")
    {}
    
    ToolBuilderBase*  builder_;
    int               properties_;
    std::string       menu_name_;
};


// ------------------------------

// TOOLFACTORY:
// The factory object that instantiates the tools

class ToolFactory : public boost::noncopyable  {

// -- Constructor --

  public:
    ToolFactory();

// -- Tool registration --

  public:
    // REGISTER_TOOL:
    // Register a tool so that it can be automatically build in the tool
    // factory.
  
    template <class TOOL>
    void register_tool(std::string tool_name, 
                       std::string tool_menu_name, 
                       int tool_properties)
    {
      tool_name = Utils::string_to_lower(tool_name);
      // Lock the factory
      boost::unique_lock<boost::mutex> lock(tools_mutex_);

      // Test is tool was registered before.
      if (tools_.find(tool_name) != tools_.end())
      {
        // Actions that are registered twice, will cause problems
        // Hence the program will throw an exception.
        // As registration is done on startup, this will cause a
        // faulty program to fail always on startup.
        SCI_THROW_LOGICERROR(std::string("Tool '")+tool_name+"' is registered twice");
      }

      // Register the action and set its properties
      tools_[tool_name] = ToolInfo(new ToolBuilder<TOOL>,
                                        tool_properties,tool_menu_name);
      
      SCI_LOG_DEBUG(std::string("Registering tool : ") + tool_name);
    }

  private:
    
    // Mutex protecting the singleton interface  
    typedef boost::unordered_map<std::string,ToolInfo>  tool_map_type;
    
    // List with builders that can be called to generate a new object
    tool_map_type    tools_;
    
    // Mutex for protecting registration
    boost::mutex     tools_mutex_;
    
// -- ToolInterface registration --

  public:
    // REGISTER_TOOLINTERFACE:
    // Register a tool so that it can be automatically build in the tool
    // factory.
  
    template <class TOOLINTERFACE>
    void register_toolinterface(std::string toolinterface_name)
    {
      toolinterface_name = Utils::string_to_lower(toolinterface_name);
      // Lock the factory
      boost::unique_lock<boost::mutex> lock(toolinterface_builders_mutex_);

      // Test is tool was registered before.
      if (toolinterface_builders_.find(toolinterface_name) != toolinterface_builders_.end())
      {
        // Actions that are registered twice, will cause problems
        // Hence the program will throw an exception.
        // As registration is done on startup, this will cause a
        // faulty program to fail always on startup.
        SCI_THROW_LOGICERROR(std::string("ToolInterface '")+toolinterface_name+"' is registered twice");
      }

      // Register the action
      toolinterface_builders_[toolinterface_name] = new ToolInterfaceBuilder<TOOLINTERFACE>;
      SCI_LOG_DEBUG(std::string("Registering toolinterface : ") + toolinterface_name);
    }

  private:
    typedef boost::unordered_map<std::string,ToolInterfaceBuilderBase*> toolinterface_map_type;
    // List with builders that can be called to generate a new object
    toolinterface_map_type  toolinterface_builders_;

    // Mutex for protecting registration
    boost::mutex     toolinterface_builders_mutex_;
    
// -- Instantiate tools and toolinterfaces --
  public:
    
    // CREATE_TOOL:
    // Generate an tool from an iostream object that contains the XML
    // specification of the action.
    bool create_tool(const std::string& tool_type,
                     const std::string& toolid,
                     ToolHandle& tool) const;

    // CREATE_TOOLINTERFACE:
    // Generate an tool from an iostream object that contains the XML
    // specification of the action.
    bool create_toolinterface(const std::string& toolinterface_name,
                              ToolInterfaceHandle& toolinterface) const;

// -- List of tools and interfaces --
  public:
    // pairs of tool name and the name the tool should have in the menu
    typedef std::vector<std::pair<std::string,std::string> > tool_list_type;

    // IS_TOOL_TYPE:
    // Check whether a tool with a specified name is available
    bool is_tool_type(const std::string& tool_type) const;

    // LIST_TOOL_TYPES:
    // List the tools of a certain group
    bool list_tool_types(tool_list_type& tool_list, int tool_property) const;
    
// -- Singleton interface --
  public:
    
    static ToolFactory* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<ToolFactory> instance_;
};

} // end namespace seg3D

#define SCI_REGISTER_TOOL(name,menuname,properties)\
void register_tool_##name()\
{\
  ToolFactory::Instance()->register_tool<name>(#name,menuname,properties);\
} 

#define SCI_REGISTER_TOOLINTERFACE(name)\
void register_toolinterface_##name()\
{\
  ToolFactory::Instance()->register_toolinterface<name>(#name);\
} 

#endif
