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

#include <iostream>

// TinyXML includes
#include <tinyxml.h>

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
  ActionInfoPrivate();
  // Definition of the action in XML
  std::string definition_;

  // The type of the action, i.e. the name with which it should be called
  std::string type_;

  // Description of the action, i.e. what does it do
  std::string description_;

  // Properties of the action
  std::vector<std::string> properties_;
  
  // Vector of keys that can be added
  std::vector<std::string> key_;    
  // Vector of default values for each key
  std::vector<std::string> key_default_value_;
  // Vector of descriptions for each key
  std::vector<std::string> key_description_;
  // Vector of properties per argument
  std::vector<std::vector<std::string> > key_properties_;
      
  // Description of how to use the action
  std::string usage_;
  
  // Test whether the action described was valid or not
  // NOTE: If not valid the program will not register the action
  bool valid_;
  
  // Default properties
  bool is_undoable_;
  bool changes_project_data_;
};

ActionInfoPrivate::ActionInfoPrivate() :
  valid_( false ),
  is_undoable_( false ),
  changes_project_data_( false )
{
}


ActionInfo::ActionInfo( const std::string& definition ) :
  private_( new ActionInfoPrivate )
{
  CORE_LOG_MESSAGE( std::string( "Registering action: " ) + definition );

  // NOTE: We need to add an end line, otherwise tinyXML does not accept the xml string
  this->private_->definition_ = definition + "\n";

  // Define a document
  TiXmlDocument doc;
  
  // Parse the xml content into the DOM tree
  if ( ! ( doc.Parse( this->private_->definition_.c_str() ) ) )
  {
    CORE_LOG_ERROR( "Action Registration: Could not parse the XML definition of an action" );
    CORE_LOG_ERROR( "Action Registration: Skipping registration of this action" );
    return;
  }

  // NOTE: TiXmlHandles are not used for garbage collection
  // They provide a mechanism to avoid entering into a none existing
  // element.
  TiXmlHandle hDoc( &doc );
  
  bool found_action = false;
  
  for ( TiXmlElement* parameter_element = hDoc.FirstChildElement().Element()
    ; parameter_element; parameter_element = parameter_element->NextSiblingElement() )  
  {
    // NOTE: Cannot pass the pointer directly into the string, as the string can be empty.
    // In that case a zero pointer is returned and STL does not handle that case for us
    std::string type;
    if ( parameter_element->Value() )
    {
      type = parameter_element->Value();
    }
    
    //if ( type == "argument" )
    //{
    //  std::string name;
    //  if ( parameter_element->Attribute( "name" ) ) 
    //  {
    //    name = parameter_element->Attribute( "name" );
    //  }
    //  std::string description;
    //  if ( parameter_element->GetText() )
    //  {
    //    description = parameter_element->GetText();
    //  }
    //  
    //  if ( name.empty() )
    //  {
    //    CORE_LOG_ERROR( "Action Registration: Action argument needs to have name." );
    //    CORE_LOG_ERROR( "Action Registration: Skipping registration of this action" );
    //    return;
    //  }
    //  
    //  this->private_->argument_.push_back( name );
    //  this->private_->argument_description_.push_back( description );
    //  std::vector< std::string > empty_properties;
    //  this->private_->argument_properties_.push_back( empty_properties );
    //}
    //else if ( type == "action" )
    if ( type == "action" )
    {
      std::string name;
      if ( parameter_element->Attribute( "name" ) )
      {
        name = parameter_element->Attribute( "name" );
      }
      std::string description;
      if ( parameter_element->GetText() )
      {
        description = parameter_element->GetText();
      }
      
      if ( name.empty() )
      {
        CORE_LOG_ERROR( "Action Registration: Action needs to have name." );
        CORE_LOG_ERROR( "Action Registration: Skipping registration of this action" );
        return;
      }
      
      this->private_->type_ = name;
      this->private_->description_ = description;
      
      // XML has the passed the minimum definition requirement
      found_action = true;
    }
    else if ( type == "key" )
    {
      std::string name;
      if ( parameter_element->Attribute( "name" ) )
      {
        name = parameter_element->Attribute( "name" );
      }
      std::string default_value;
      if ( parameter_element->Attribute( "default" ) )
      {
        default_value = parameter_element->Attribute( "default" );
      }
      std::string description;
      if ( parameter_element->GetText() )
      {
        description = parameter_element->GetText();
      }
      
      if ( name.empty() )
      {       
        CORE_LOG_ERROR( "Action Registration: Action key/value pair needs to have name." );
        CORE_LOG_ERROR( "Action Registration: Skipping registration of this action" );
        return;
      }
      
      this->private_->key_.push_back( name );
      this->private_->key_default_value_.push_back( default_value );
      this->private_->key_description_.push_back( description );    
      std::vector< std::string > empty_properties;
      this->private_->key_properties_.push_back( empty_properties );
    }
    else if ( type == "property" )
    {
      std::string property;
      if ( parameter_element->GetText() )
      {
        property = parameter_element->GetText();
      } 

      std::string name;
      if ( parameter_element->Attribute( "name" ) )
      {
        name = parameter_element->Attribute( "name" );
        for ( size_t k = 0; k < this->private_->key_.size(); k++ )
        {
          if ( this->private_->key_[ k ] == name )
          {
            this->private_->key_properties_[ k ].push_back( property );
          }
        }
      }
      else
      {   
        this->private_->properties_.push_back( property );
      }
    }
  }

  if ( found_action == false )
  {
    CORE_LOG_ERROR( "Need an action tag in the definition of an action" );
    CORE_LOG_ERROR( "Action Registration: Skipping registration of this action" );
    return;
  }
  
  std::string usage = this->private_->type_;
  
  for ( size_t j = 0; j < this->private_->key_.size(); j++ )
  {
    if ( this->private_->key_default_value_[ j ].empty() )
    {
      usage += ( " " + this->private_->key_[ j ] + "=<" + this->private_->key_[ j ] + ">" );
    }
    else
    {
      usage += std::string( " [" ) + this->private_->key_[ j ] + "=" +
        this->private_->key_default_value_[ j ] + "]";
    }
  }

  this->private_->usage_ = usage;
  
  // We parsed everything, so action info is now valid
  this->private_->valid_ = true;
  
  if ( std::find( this->private_->properties_.begin(), this->private_->properties_.end(), 
    "changes_project_data" ) !=  this->private_->properties_.end() )
  {
    this->private_->changes_project_data_ = true;
  }

  if ( std::find( this->private_->properties_.begin(), this->private_->properties_.end(), 
    "is_undoable" ) !=  this->private_->properties_.end() )
  {
    this->private_->is_undoable_ = true;
  }
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

std::vector<std::string> ActionInfo::get_properties() const
{
  return this->private_->properties_;
}


std::string ActionInfo::get_usage() const
{ 
  return this->private_->usage_;
}
  
size_t ActionInfo::get_num_key_value_pairs() const
{
  return this->private_->key_.size();
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

std::vector<std::string> ActionInfo::get_key_properties( size_t index ) const
{
  if ( index >= this->private_->key_properties_.size() )
  {
    return std::vector<std::string>();
  } 
  return this->private_->key_properties_[ index ];
}
  
bool ActionInfo::is_valid() const
{
  return this->private_->valid_;
}

bool ActionInfo::changes_project_data() const
{
  return this->private_->changes_project_data_;
}

bool ActionInfo::is_undoable() const
{
  return this->private_->is_undoable_;
}

// Define a mutex that protects all of the ActionInfo classes
// Needs to be defined somewhere, so it is unique
ActionInfo::mutex_type ActionInfo::mutex_;

ActionInfo::mutex_type& ActionInfo::GetMutex()
{
  return mutex_;
}

} // end namespace Core
