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
#include <map>

// Application includes
#include <Application/Tool/ToolFactory.h>

namespace Seg3D
{

ToolBuilderBase::~ToolBuilderBase()
{
}

ToolInterfaceBuilderBase::~ToolInterfaceBuilderBase()
{
}

class ToolEntry
{
public:
  // Pointer to the factory builder, that creates the objects
  ToolBuilderBase* builder_;

  // The tool information
  ToolInfoHandle info_;
};

class ToolFactoryPrivate
{
public:
  // List with builders that can be called to generate a new object
  typedef std::map< std::string, ToolEntry > tool_map_type;
  tool_map_type tools_;

  typedef std::map< std::string, ToolInterfaceBuilderBase* > toolinterface_map_type;
  // List with builders that can be called to generate a new object
  toolinterface_map_type toolinterfaces_;

};

CORE_SINGLETON_IMPLEMENTATION( ToolFactory );

ToolFactory::ToolFactory() :
  private_( new ToolFactoryPrivate )
{
}

ToolFactory::~ToolFactory()
{
}

void ToolFactory::register_tool( ToolBuilderBase* builder, ToolInfoHandle info )
{
  lock_type lock( this->get_mutex() );

  // Test is tool was registered before.
  if ( this->private_->tools_.find( info->get_name() ) != this->private_->tools_.end() )
  {
    // Actions that are registered twice, will cause problems
    // Hence the program will throw an exception.
    // As registration is done on startup, this will cause a
    // faulty program to fail always on startup.
    CORE_THROW_LOGICERROR( std::string( "Tool '" ) + info->get_name() + "' is registered twice" );
  }

  // Register the action and set its properties 
  ToolEntry entry;
  entry.builder_ = builder;
  entry.info_ = info;
  this->private_->tools_[ info->get_name() ] = entry;
  CORE_LOG_DEBUG( std::string( "Registering tool : " ) + info->get_name() );
}

void ToolFactory::register_toolinterface( ToolInterfaceBuilderBase* builder,
  std::string toolinterface_name )
{
  toolinterface_name = Core::StringToLower( toolinterface_name );
  
  if ( toolinterface_name.substr( toolinterface_name.size() - 9 ) != std::string( "interface" ) )
  {
    CORE_THROW_LOGICERROR( 
      std::string( "ToolInterface class name does not end with Interface" ) );
  }

  // Strip out the word interface
  toolinterface_name = toolinterface_name.substr( 0, toolinterface_name.size() - 9 );

  // Lock the factory
  lock_type lock( this->get_mutex() );

  // Test is tool was registered before.
  if ( this->private_->toolinterfaces_.find( toolinterface_name ) != 
    this->private_->toolinterfaces_.end() )
  {
    // Actions that are registered twice, will cause problems
    // Hence the program will throw an exception.
    // As registration is done on startup, this will cause a
    // faulty program to fail always on startup.
    CORE_THROW_LOGICERROR( std::string( "ToolInterface '" ) +
      toolinterface_name + "' is registered twice" );
  }

  // Register the action
  this->private_->toolinterfaces_[ toolinterface_name ] = builder;
  
  CORE_LOG_DEBUG( std::string( "Registering toolinterface : " ) + toolinterface_name );
}


bool ToolFactory::is_tool_type( const std::string& tool_name )
{
  lock_type lock( this->get_mutex() );

  if ( this->private_->tools_.find( Core::StringToLower( tool_name ) )
    == this->private_->tools_.end() ) return false;

  return true;
}

bool LessToolList( ToolInfoList::value_type val1, ToolInfoList::value_type val2 )
{
  return ( val1->get_menu_label() < val2->get_menu_label() );
}


bool ToolFactory::list_tools( ToolInfoList& tool_list )
{
  lock_type lock( this->get_mutex() );

  // clear the list
  tool_list.clear();

  ToolFactoryPrivate::tool_map_type::const_iterator it = this->private_->tools_.begin();

  // loop through all the tools
  while ( it != this->private_->tools_.end() )
  {
    if ( this->private_->toolinterfaces_.find( ( *it ).first ) != 
      this->private_->toolinterfaces_.end() )
    {
      tool_list.push_back( ( *it ).second.info_ );
    }
    ++it;
  }

  if ( tool_list.size() == 0 ) return false;
  std::sort( tool_list.begin(), tool_list.end(), LessToolList );

  return true;
}

bool ToolFactory::create_tool( const std::string& tool_type, ToolHandle& tool )
{
  lock_type lock( get_mutex() );

  // Step (1): split the tool type in case it has a number affixed
  std::string tool_name = Core::SplitString( tool_type, "_" )[ 0 ];

  // Step (2): find the tool
  ToolFactoryPrivate::tool_map_type::const_iterator it = 
    this->private_->tools_.find( Core::StringToLower( tool_name ) );

  // Step (3): check its existence
  if ( it == this->private_->tools_.end() ) return false;

  // Step (4): build the tool
  tool = ( *it ).second.builder_->build( tool_type );

  return true;
}

bool ToolFactory::create_toolinterface( const std::string& toolinterface_name,
    ToolInterface*& toolinterface )
{
  lock_type lock( this->get_mutex() );

  // Step (1): find the tool
  ToolFactoryPrivate::toolinterface_map_type::const_iterator it = 
    this->private_->toolinterfaces_.find( toolinterface_name );

  // Step (2): check its existence
  if ( it == this->private_->toolinterfaces_.end() )
  {
    CORE_THROW_LOGICERROR( std::string( "Trying to instantiate tool '" )
      + toolinterface_name + "' that does not exist" );
  }

  // Step (3): build the tool
  toolinterface = (*it).second->build();

  return true;
}

} // end namespace seg3D
