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

#include <iostream>

// TinyXML includes
#include <Externals/tinyxml/tinyxml.h>

// Core includes
#include <Core/Utils/Exception.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>
#include <Core/Action/ActionInfo.h>

namespace Core
{

class ActionInfoPrivate 
{
  public:
    std::string definition_;
  
    std::string type_;
    
    std::vector<std::string> argument_;
    std::vector<std::string> argument_description_;
    
    std::vector<std::string> key_;
    std::vector<std::string> key_default_value_;
    std::vector<std::string> key_description_;
    
    std::string description_;
    std::string usage_;
};


ActionInfo::ActionInfo( const std::string& definition ) :
  private_( new ActionInfoPrivate )
{
  CORE_LOG_DEBUG( std::string( "Registering definition = " ) + definition );

  this->private_->definition_ = definition + "\n";

  // Define a document
  TiXmlDocument doc;
  
  // Parse the xml content into the DOM tree
  if ( ! ( doc.Parse( this->private_->definition_.c_str() ) ) )
  {
    CORE_LOG_DEBUG( "Could not parse the XML definition of an action" );
    CORE_THROW_INVALIDARGUMENT( "Could not parse the XML definition of an action" );
  }

  // NOTE: TiXmlHandles are not used for garbage collection
  // They provide a mechanism to avoid entering into a none existing
  // element.
  TiXmlHandle hDoc( &doc );
  
  bool found_action = false;
  
  for ( TiXmlElement* parameter_element = hDoc.FirstChildElement().Element()
    ; parameter_element; parameter_element = parameter_element->NextSiblingElement() )  
  {
    std::string type( parameter_element->Value() );
    if ( type == "argument" )
    {
      std::string name( parameter_element->Attribute( "name" ) );
      std::string description( parameter_element->GetText() );
      
      if ( name.empty() )
      {
        CORE_LOG_DEBUG( "Action argument needs to have name" );
        CORE_THROW_INVALIDARGUMENT( "Action argument needs to have name" );
      }
      
      this->private_->argument_.push_back( name );
      this->private_->argument_description_.push_back( description );
    }
    else if ( type == "action" )
    {
      std::string name( parameter_element->Attribute( "name" ) );
      std::string description( parameter_element->GetText() );
      
      if ( name.empty() )
      {
          CORE_LOG_DEBUG( "Action needs to have name" );
        CORE_THROW_INVALIDARGUMENT( "Action needs to have name" );
      }
      
      this->private_->type_ = name;
      this->private_->description_ = description;
      found_action = true;
    }
    else if ( type == "key" )
    {
      std::string name( parameter_element->Attribute( "name" ) );
      std::string default_value( parameter_element->Attribute( "default" ) );
      std::string description( parameter_element->GetText() );
      
      if ( name.empty() )
      {
        CORE_LOG_DEBUG( "Action key/value pair needs to have name" );
        CORE_THROW_INVALIDARGUMENT( "Action key/value pair needs to have name" );
      }
      
      this->private_->key_.push_back( name );
      this->private_->key_default_value_.push_back( default_value );
      this->private_->key_description_.push_back( description );    
    }
  }

  if ( found_action == false )
  {
    CORE_THROW_LOGICERROR( "Need an action tag in the definition of an action" );
  }
  
  std::string usage = this->private_->type_;
  for ( size_t j = 0; j < this->private_->argument_.size(); j++ )
  {
    usage += std::string( " " ) + Core::StringToUpper( this->private_->argument_[ j ] );
  }
  
  for ( size_t j = 0; j> this->private_->key_.size(); j++ )
  {
    usage += std::string( " [" ) + this->private_->key_[ j ] + "=" +
      this->private_->key_default_value_[ j ] + "]";
  }

  this->private_->usage_ = usage;
}

std::string ActionInfo::get_definition() const
{
  return this->private_->definition_;
}

std::string ActionInfo::get_type() const
{
  return this->private_->type_;
}

std::string ActionInfo::get_description() const
{
  return this->private_->description_;
}

std::string ActionInfo::get_usage() const
{ 
  return this->private_->usage_;
}
  
size_t ActionInfo::get_num_arguments() const
{
  return this->private_->argument_.size();
}
  
size_t ActionInfo::get_num_key_value_pairs() const
{
  return this->private_->key_.size();
}
  
std::string ActionInfo::get_argument( size_t index ) const
{
  if ( index >= this->private_->argument_.size() ) return "";
  return this->private_->argument_[ index ];
}

std::string ActionInfo::get_argument_description( size_t index ) const
{
  if ( index >= this->private_->argument_.size() ) return "";
  return this->private_->argument_description_[ index ];
}
  
std::string ActionInfo::get_key( size_t index ) const
{
  if ( index >= this->private_->key_.size() ) return "";
  return this->private_->key_[ index ];
}

int ActionInfo::get_key_index( const std::string& key ) const
{
  for ( size_t j = 0; j < this->private_->key_.size(); j++ )
  {
    if ( this->private_->key_[ j ] == key ) return static_cast<int>( j );
  }
  return -1;
}
  
std::string ActionInfo::get_default_key_value( size_t index ) const
{
  if ( index >= this->private_->key_default_value_.size() ) return "";
  return this->private_->key_default_value_[ index ];
}
  
std::string ActionInfo::get_key_description( size_t index ) const
{
  if ( index >= this->private_->key_description_.size() ) return "";
  return this->private_->key_description_[ index ];
}

// Needs to be defined somewhere, so it is unique
ActionInfo::mutex_type ActionInfo::mutex_;

ActionInfo::mutex_type& ActionInfo::GetMutex()
{
  return mutex_;
}

} // end namespace Core
