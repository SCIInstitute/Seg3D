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

#if defined(_MSC_VER)
#pragma warning(disable : 4267 4244)
#endif

// STL includes
#include <algorithm>
#include <map>

// boost includes
#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>

// Core includes
#include <Core/State/StateIO.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

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
  
  typedef std::set< std::string > tool_menus_type;
  tool_menus_type tool_menus_;

};

CORE_SINGLETON_IMPLEMENTATION( ToolFactory );

ToolFactory::ToolFactory() :
  Core::StateHandler( "toolfactory", false ),
  private_( new ToolFactoryPrivate )
{
}

ToolFactory::~ToolFactory()
{
}

#ifdef BUILD_MANUAL_TOOLS_ONLY
static std::set<std::string> nonManualTools = { "croptool", "resampletool", "thresholdtool", "cannyedgedetectionfilter",
"confidenceconnectedfilter", "connectedcomponentfilter", "curvatureanisotropicdiffusionfilter",
"discretegaussianfilter", "distancefilter", "extractdatalayer", "gradientanisotropicdiffusionfilter",
"gradientmagnitudefilter", "histogramequalizationfilter", "intensitycorrectionfilter",
"maskdatafilter", "medianfilter", "meanfilter", "neighborhoodconnectedfilter",
"otsuthresholdfilter", "thresholdsegmentationlsfilter", "transformtool", "measurementtool",
"pointsetfilter", "speedlinetool", "pointsselecttool", "padtool", "implicitmodeltool", "watershedfilter" };
#endif

void ToolFactory::register_tool(ToolBuilderBase* builder, ToolInfoHandle info,
  std::string tool_name)
{
  tool_name = Core::StringToLower(tool_name);

#ifdef BUILD_MANUAL_TOOLS_ONLY
 //When building a library, we don't want to include the tools in nonManualTools
    if (nonManualTools.find(tool_name) != nonManualTools.end())
    {
      return;
    }
#endif

  lock_type lock(this->get_mutex());

  // Test is tool was registered before.
  if (this->private_->tools_.find(tool_name) != this->private_->tools_.end())
  {
    // Actions that are registered twice, will cause problems
    // Hence the program will throw an exception.
    // As registration is done on startup, this will cause a
    // faulty program to fail always on startup.
    CORE_THROW_LOGICERROR(std::string("Tool '") + tool_name + "' is registered twice");
  }

  // Register the action and set its properties 
  ToolEntry entry;
  entry.builder_ = builder;
  entry.info_ = info;
  this->private_->tools_[tool_name] = entry;

  this->private_->tool_menus_.insert(info->get_menu());

  CORE_LOG_DEBUG(std::string("Registering tool : ") + tool_name);
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


bool ToolFactory::list_tools( ToolInfoList& tool_list, std::string menu )
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
      if ( menu.size() == 0 || ( *it ).second.info_->get_menu() == menu )
      {
        tool_list.push_back( ( *it ).second.info_ );
      }
    }
    ++it;
  }

  if ( tool_list.size() == 0 ) return false;
  std::sort( tool_list.begin(), tool_list.end(), LessToolList );

  return true;
}

bool ToolFactory::list_menus( ToolMenuList& menu_list )
{
  menu_list.clear();
  ToolFactoryPrivate::tool_menus_type::const_iterator it = this->private_->tool_menus_.begin();
  ToolFactoryPrivate::tool_menus_type::const_iterator it_end = this->private_->tool_menus_.end();
  while ( it != it_end )
  {
    menu_list.push_back( *it );
    ++it;
  }
  
  if ( menu_list.size() == 0 ) return false;
  std::sort( menu_list.begin(), menu_list.end(), boost::lambda::_1 > boost::lambda::_2 );
  
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

void ToolFactory::initialize_states()
{
  this->set_initializing( true );

  ToolMenuList tool_menus;
  this->list_menus( tool_menus );
  std::vector< std::string > none_option;
  for ( ToolMenuList::iterator menu_it = tool_menus.begin(); 
    menu_it != tool_menus.end(); ++menu_it )
  {
    const std::string menu_name = *menu_it;
    std::string state_name = boost::regex_replace( menu_name, boost::regex( "[^\\w]" ), "_" );
    state_name = "startup_" + Core::StringToLower( state_name );
    ToolInfoList tools;
    this->list_tools( tools, menu_name );
    Core::OptionLabelPairVector tools_vec;
    for ( size_t i =0; i < tools.size(); ++i )
    {
      tools_vec.push_back( std::make_pair( tools[ i ]->get_name(), tools[ i ]->get_menu_label() ) );
    }
    Core::StateLabeledMultiOptionHandle state;
    this->add_state( state_name, state, none_option, tools_vec );
    this->startup_tools_state_[ menu_name ] = state;
  }
  
  boost::filesystem::path config_path;
  Core::Application::Instance()->get_config_directory( config_path );
  Core::StateIO state_io;
  if ( state_io.import_from_file( config_path / "tools.xml" ) )
  {
    this->load_states( state_io );
  }

  this->set_initializing( false );
}

void ToolFactory::save_settings()
{
  boost::filesystem::path config_path;
  Core::Application::Instance()->get_config_directory( config_path );
  Core::StateIO state_io;
  state_io.initialize();
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    this->save_states( state_io );
  }
  state_io.export_to_file( config_path / "tools.xml" );
}

} // end namespace seg3D
