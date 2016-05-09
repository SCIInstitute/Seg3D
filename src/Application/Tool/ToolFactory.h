/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/Lockable.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Singleton.h>
#include <Core/State/StateLabeledMultiOption.h>

// Application includes
#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolInterface.h>

namespace Seg3D
{

// ---- Auxilary Classes -----

/// TOOLBUILDERBASE:
/// This class is the base functor for the class that builds the classes in
/// the factory

class ToolBuilderBase
{

public:
  // virtual destructor to ensure class can be deleted properly
  virtual ~ToolBuilderBase();

  // the functor call to build the object
  virtual ToolHandle build( const std::string& toolid ) = 0;
};

/// TOOLBUILDER:
/// The actual instantiation that builds the tool of type TOOL. This class
/// is loaded on top of the base functor and creates the action

template <class TOOL>
class ToolBuilder: public ToolBuilderBase
{

public:
  // ensure we can delete the builder correctly
  virtual ~ToolBuilder()
  {
  }

  // The actual builder call
  virtual ToolHandle build( const std::string& toolid )
  {
    return ToolHandle( new TOOL( toolid ) );
  }

};

/// TOOLINTERFACEBUILDERBASE:
/// This class is the base functor for the class that builds the classes in
/// the factory
/// NOTE: This class does not use a handle to make it easier to incorporate this
/// code into Qt at the interface layer.

class ToolInterfaceBuilderBase
{
public:
  // ensure we can delete the builder correctly
  virtual ~ToolInterfaceBuilderBase();

  // the functor call to build the object
  virtual ToolInterface* build() = 0;
};

/// TOOLINTERFACEBUILDER:
/// The actual instantiation that builds the tool of type ToolInterface. This 
/// class is loaded on top of the base functor and creates the action

template <class TOOLINTERFACE>
class ToolInterfaceBuilder: public ToolInterfaceBuilderBase
{

public:
  virtual ~ToolInterfaceBuilder()
  {
  }

  // The actual builder call
  virtual ToolInterface* build()
  {
    return new TOOLINTERFACE;
  }
};

typedef std::vector<ToolInfoHandle> ToolInfoList;
typedef std::vector<std::string> ToolMenuList;

/// ------------------------------

/// TOOLFACTORY:
/// The factory object that instantiates the tools

// Forward declaration
class ToolFactory;
class ToolFactoryPrivate;
typedef boost::shared_ptr<ToolFactoryPrivate> ToolFactoryPrivateHandle;


// Class definition
class ToolFactory : public Core::Lockable, private Core::StateHandler
{
  CORE_SINGLETON( ToolFactory );

// -- Constructor --
private:
  ToolFactory();
  virtual ~ToolFactory();

  // -- Tool registration --
public:
  /// REGISTER_TOOL:
  /// Register a tool so that it can be automatically built in the tool
  /// factory.
  void register_tool( ToolBuilderBase* builder, ToolInfoHandle tool_info,
    std::string tool_name );

  /// REGISTER_TOOLINTERFACE:
  /// Register a tool so that it can be automatically build in the tool
  /// factory.
  void register_toolinterface( ToolInterfaceBuilderBase* builder, 
    std::string toolinterface_name );

  // -- Instantiate tools and toolinterfaces --
public:

  /// CREATE_TOOL:
  /// Generate an tool from a type
  bool create_tool( const std::string& tool_type, ToolHandle& tool );

  /// CREATE_TOOLINTERFACE:
  /// Generate an tool interface from a type
  bool create_toolinterface( const std::string& toolinterface_name,
    ToolInterface*& toolinterface );

  // -- List of tools and interfaces --
public:

  /// IS_TOOL_TYPE:
  /// Check whether a tool with a specified name is available
  bool is_tool_type( const std::string& tool_type );

  /// LIST_TOOL_TYPES:
  /// List the tools
  bool list_tools( ToolInfoList& tool_list, std::string menu = "" );
  
  /// LIST_MENUS:
  /// List the menus
  bool list_menus( ToolMenuList& menu_list );

  // -- State handling --
public:
  /// INITIALIZE_STATES:
  /// Initialize state variables and load settings from file.
  void initialize_states();

  /// SAVE_SETTINGS:
  /// Write the tools settings out to file.
  void save_settings();

  // -- State Variables --
public:
  typedef std::map< std::string, Core::StateLabeledMultiOptionHandle > startup_tools_map_type;
  startup_tools_map_type startup_tools_state_;

  // -- internals of ToolFactory --
private:
  ToolFactoryPrivateHandle private_;

};

} // end namespace seg3D

#define SCI_REGISTER_TOOL(namesp, name)\
namespace Core\
{\
  using namespace namesp;\
  void register_##name()\
  {\
    Seg3D::ToolFactory::Instance()->register_tool( new Seg3D::ToolBuilder<name>, name::GetToolInfo(), #name );\
  } \
}

#define SCI_REGISTER_TOOLINTERFACE(namesp, name)\
namespace Core\
{\
  using namespace namesp;\
  void register_##name()\
  {\
    Seg3D::ToolFactory::Instance()->register_toolinterface( new Seg3D::ToolInterfaceBuilder<name>, #name);\
  }\
}

#endif
