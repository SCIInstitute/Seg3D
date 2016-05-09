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

// TinyXML includes
#include <tinyxml.h>

// Core includes
#include <Core/Utils/Exception.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/Tool/ToolInfo.h>

namespace Seg3D
{

class ToolInfoPrivate 
{
public:
  ToolInfoPrivate() : version_(-1), is_large_volume_(false) {}
  std::string definition_;

  std::string name_;
  std::string description_;

  std::string menu_;
  std::string menu_label_;
  std::string shortcut_key_;
  std::string hotkeys_and_descriptions_;

  std::string url_;

  int version_;
  bool is_large_volume_;
};


ToolInfo::ToolInfo( const std::string& definition ) :
  private_( new ToolInfoPrivate )
{
  CORE_LOG_DEBUG( std::string( "Registering tool definition = " ) + definition );

  this->private_->definition_ = definition + "\n";

  // Default version number for all tools
  this->private_->version_ = 1;

  // Define a document
  TiXmlDocument doc;
  
  // Parse the xml content into the DOM tree
  if ( ! ( doc.Parse( this->private_->definition_.c_str() ) ) )
  {
    CORE_LOG_DEBUG( "Could not parse the XML definition of a tool" );
    CORE_THROW_INVALIDARGUMENT( "Could not parse the XML definition of a tool" );
  }

  // NOTE: TiXmlHandles are not used for garbage collection
  // They provide a mechanism to avoid entering into a none existing
  // element.
  TiXmlHandle hDoc( &doc );
  
  bool found_tool = false;
  
  for ( TiXmlElement* parameter_element = hDoc.FirstChildElement().Element()
    ; parameter_element; parameter_element = parameter_element->NextSiblingElement() )  
  {
    std::string type( parameter_element->Value() );
    if ( type == "tool" )
    {
      std::string name;
      std::string description;
      
      if ( parameter_element->Attribute( "name" ) )
      {
        name = parameter_element->Attribute( "name" );
      }
      if ( parameter_element->GetText() )
      {
        description = parameter_element->GetText();
      }
      
      if ( name.empty() )
      {
        CORE_LOG_DEBUG( "Tool argument needs to have name" );
        CORE_THROW_INVALIDARGUMENT( "Tool argument needs to have name" );
      }
      
      this->private_->name_ = name;
      this->private_->description_ = description;
      found_tool = true;
    }
    else if ( type == "menu" )
    {
      std::string menu;
      if ( parameter_element->GetText() )
      {
        menu = parameter_element->GetText();
      }
      this->private_->menu_ = menu;
    }
    else if ( type == "menulabel" )
    {
      std::string menulabel;
      if( parameter_element->GetText() )
      {
        menulabel = parameter_element->GetText();
      }         
      this->private_->menu_label_ = menulabel;
    }
    else if ( type == "shortcutkey" )
    {
      std::string shortcut_key;
      if( parameter_element->GetText() )
      {
        shortcut_key = parameter_element->GetText();
      }
      this->private_->shortcut_key_ = shortcut_key;
    }
    else if ( type == "url" )
    {
      std::string url;
      if( parameter_element->GetText() )
      {
        url = parameter_element->GetText();
      }         
      this->private_->url_ = url;
    }
    else if ( type == "version" )
    {
      std::string version;
      if ( parameter_element->GetText() )
      {
        version = parameter_element->GetText();
      }           
      Core::ImportFromString( version, this->private_->version_ );
    }
    else if ( type == "hotkeys" )
    {
      std::string hotkeys;
      if( parameter_element->GetText() )
      {
        hotkeys = parameter_element->GetText();
      }         
      this->private_->hotkeys_and_descriptions_ = hotkeys;
    }
    else if ( type == "largevol" )
    {
      std::string is_largevol;
      if( parameter_element->GetText() )
      {
        is_largevol = parameter_element->GetText();
      }
      Core::ImportFromString( is_largevol, this->private_->is_large_volume_ );
    }
  }

  if ( found_tool == false )
  {
    CORE_THROW_LOGICERROR( "Need a tool tag in the definition of a tool" );
  }
}

std::string ToolInfo::get_definition() const
{
  return this->private_->definition_;
}

std::string ToolInfo::get_name() const
{
  return Core::StringToLower( this->private_->name_ );
}

std::string ToolInfo::get_menu() const
{
  return this->private_->menu_;
}

std::string ToolInfo::get_menu_label() const
{
  return this->private_->menu_label_;
}

std::string ToolInfo::get_description() const
{
  return this->private_->description_;
}

std::string ToolInfo::get_shortcut_key() const
{
  return this->private_->shortcut_key_;
}

std::string ToolInfo::get_hotkeys() const
{
  return this->private_->hotkeys_and_descriptions_;
}

std::string ToolInfo::get_url() const
{
  return this->private_->url_;
}

int ToolInfo::get_version() const
{
  return this->private_->version_;
}

bool ToolInfo::get_is_large_volume() const
{
  return this->private_->is_large_volume_;
}


// Needs to be defined somewhere, so it is unique
ToolInfo::mutex_type ToolInfo::mutex_;

ToolInfo::mutex_type& ToolInfo::GetMutex()
{
  return mutex_;
}



} // end namespace Seg3D
