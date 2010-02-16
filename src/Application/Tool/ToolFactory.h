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

// TOOLBUILDERBASE:
// This class is the base functor for the class that builds the classes in
// the factory

class ToolBuilderBase {
  public:
    // ensure we can delete the builder correctly
    virtual ~ToolBuilderBase() {}
    
    // the functor call to build the object
    virtual ToolHandle build(const std::string& toolid) = 0;
};

// TOOLBUILDER:
// The actual instantiation that builds the tool of type TOOL. This class
// is loaded on top of the base functor and creates the action

template <class TOOL>
class ToolBuilder: public ToolBuilderBase {

  public:
    // ensure we can delete the builder correctly 
    virtual ~ToolBuilder() {}
    
    // The actual builder call
    virtual ToolHandle build(const std::string& toolid) 
    { 
      return ToolHandle(new TOOL(toolid)); 
    }
};


// TOOLINTERFACEBUILDERBASE:
// This class is the base functor for the class that builds the classes in
// the factory
// NOTE: This class does not use a handle to make it easier to incorporate this
// code into Qt at the interface layer.

class ToolInterfaceBuilderBase {
  public:
    // ensure we can delete the builder correctly
    virtual ~ToolInterfaceBuilderBase() {}
    
    // the functor call to build the object
    virtual ToolInterface* build() = 0;
};

// TOOLINTERFACEBUILDER:
// The actual instantiation that builds the tool of type ToolInterface. This 
// class is loaded on top of the base functor and creates the action

template <class TOOLINTERFACE>
class ToolInterfaceBuilder: public ToolInterfaceBuilderBase {

  public:
    // ensure we can delete the builder correctly 
    virtual ~ToolInterfaceBuilder() {}
    
    // The actual builder call
    virtual ToolInterface* build() 
    { 
      return new TOOLINTERFACE; 
    }
};


class ToolInfo {
  public:
    ToolInfo(std::string type,
             ToolBuilderBase* builder, 
             int properties, 
             std::string menu_name,
             std::string shortcut_key) :
      type_(type),
      builder_(builder),
      properties_(properties),
      menu_name_(menu_name),
      shortcut_key_(shortcut_key)
    {}
    
    // Copy constructor and default constructor are needed by the unordered_map
    // class
    ToolInfo(const ToolInfo& toolinfo) :
      type_(toolinfo.type_),
      builder_(toolinfo.builder_),
      properties_(toolinfo.properties_),
      menu_name_(toolinfo.menu_name_),
      shortcut_key_(toolinfo.shortcut_key_)
    {}
    
    ToolInfo() :
      type_(""),
      builder_(0),
      properties_(0),
      menu_name_(""),
      shortcut_key_("")
    {}
    
    std::string       type() const { return type_; }
    ToolBuilderBase*  builder() const { return builder_; }
    int               properties() const { return properties_; }
    std::string       menu_name() const { return menu_name_; }
    std::string       shortcut_key() const { return shortcut_key_; }    
        
  private:
    std::string       type_;
    ToolBuilderBase*  builder_;
    int               properties_;
    std::string       menu_name_;
    std::string       shortcut_key_;
};


// ------------------------------

// TOOLFACTORY:
// The factory object that instantiates the tools

// Forward declaration
class ToolFactory;

// Class definition
class ToolFactory : public boost::noncopyable  {

// -- Constructor --
  private:
    friend class Utils::Singleton<ToolFactory>;
    ToolFactory();
    
  public:
    virtual ~ToolFactory();
  
// -- Tool registration --
  public:
    // REGISTER_TOOL:
    // Register a tool so that it can be automatically build in the tool
    // factory.
  
    template <class TOOL>
    void register_tool()
    {
      // Lock the factory
      lock_type lock( mutex_ );

      // Get the type of the tool
      std::string tool_type = Utils::string_to_lower(TOOL::tool_type());

      // Test is tool was registered before.
      if (tools_.find(tool_type) != tools_.end())
      {
        // Actions that are registered twice, will cause problems
        // Hence the program will throw an exception.
        // As registration is done on startup, this will cause a
        // faulty program to fail always on startup.
        SCI_THROW_LOGICERROR(std::string("Tool '")+tool_type+"' is registered twice");
      }

      // Register the action and set its properties
      tools_[tool_type] = new ToolInfo(TOOL::tool_type(),
                                       new ToolBuilder<TOOL>,
                                       TOOL::tool_properties(),
                                       TOOL::tool_menu_name(),
                                       TOOL::tool_shortcut_key());
      
      SCI_LOG_DEBUG(std::string("Registering tool : ") + tool_type);
    }

  private:
    typedef boost::unordered_map<std::string,ToolInfo*> tool_map_type;
    typedef boost::mutex                                mutex_type;
    typedef boost::unique_lock<mutex_type>              lock_type;
        
    // List with builders that can be called to generate a new object
    tool_map_type    tools_;
    
    // Mutex for protecting registration
    mutex_type       mutex_;
    
// -- ToolInterface registration --
  public:
    // REGISTER_TOOLINTERFACE:
    // Register a tool so that it can be automatically build in the tool
    // factory.
  
    template <class TOOLINTERFACE>
    void register_toolinterface(std::string toolinterface_name)
    {
      toolinterface_name = Utils::string_to_lower(toolinterface_name);
      if (toolinterface_name.substr(toolinterface_name.size()-9) != std::string("interface"))
      {
        SCI_THROW_LOGICERROR(std::string("ToolInterface class name does not end with Interface"));
      }
      
      // Strip out the word interface
      toolinterface_name = toolinterface_name.substr(0,toolinterface_name.size()-9);

      // Lock the factory
      lock_type lock(mutex_);

      // Test is tool was registered before.
      if (toolinterfaces_.find(toolinterface_name) != toolinterfaces_.end())
      {
        // Actions that are registered twice, will cause problems
        // Hence the program will throw an exception.
        // As registration is done on startup, this will cause a
        // faulty program to fail always on startup.
        SCI_THROW_LOGICERROR(std::string("ToolInterface '")+
                                    toolinterface_name+"' is registered twice");
      }

      // Register the action
      toolinterfaces_[toolinterface_name] = new ToolInterfaceBuilder<TOOLINTERFACE>;
      SCI_LOG_DEBUG(std::string("Registering toolinterface : ") + toolinterface_name);
    }

  private:
    typedef boost::unordered_map<std::string,ToolInterfaceBuilderBase*> toolinterface_map_type;
    // List with builders that can be called to generate a new object
    toolinterface_map_type  toolinterfaces_;
    
// -- Instantiate tools and toolinterfaces --
  public:
    
    // CREATE_TOOL:
    // Generate an tool from an iostream object that contains the XML
    // specification of the action.
    bool create_tool(const std::string& tool_type,
                     const std::string& toolid,
                     ToolHandle& tool); // << THREAD-SAFE

    // CREATE_TOOLINTERFACE:
    // Generate an tool from an iostream object that contains the XML
    // specification of the action.
    bool create_toolinterface(const std::string& toolinterface_name,
                              ToolInterface*& toolinterface); // << THREAD-SAFE

// -- List of tools and interfaces --
  public:
    // pairs of tool name and the name the tool should have in the menu
    typedef std::vector<ToolInfo*> tool_list_type;

    // IS_TOOL_TYPE:
    // Check whether a tool with a specified name is available
    bool is_tool_type(const std::string& tool_type); // << THREAD-SAFE

    // LIST_TOOL_TYPES:
    // List the tools of a certain group
    bool list_tool_types(tool_list_type& tool_list, 
                         int tool_property); // << THREAD-SAFE

    // LIST_TOOL_TYPES_WITH_INTERFACE:
    // List the tools of a certain group
    bool list_tool_types_with_interface(tool_list_type& tool_list, 
                                        int tool_property); // << THREAD-SAFE
    
// -- Singleton interface --
  public:
    static ToolFactory* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<ToolFactory> instance_;
};

} // end namespace seg3D

#define SCI_REGISTER_TOOL(name)\
void register_tool_##name()\
{\
  ToolFactory::Instance()->register_tool<name>();\
} 

#define SCI_REGISTER_TOOLINTERFACE(name)\
void register_toolinterface_##name()\
{\
  ToolFactory::Instance()->register_toolinterface<name>(#name);\
} 

#endif
