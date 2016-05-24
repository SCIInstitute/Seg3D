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

// STL includes
#include <stack>

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/State/StateIO.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Application/Application.h>

namespace Core
{

class StateIOPrivate
{
public:
  TiXmlDocument xml_doc_;
  int major_version_;
  int minor_version_;
  int patch_version_;

  mutable TiXmlElement* current_element_;
  mutable std::stack< TiXmlElement* > current_element_stack_;
};

StateIO::StateIO() :
  private_( new StateIOPrivate )
{
  this->private_->current_element_ = 0;
  this->private_->major_version_ = 0;
  this->private_->minor_version_ = 0;
  this->private_->patch_version_ = 0;
}

StateIO::~StateIO()
{
}

bool StateIO::export_to_file( const boost::filesystem::path& path )
{
  return this->private_->xml_doc_.SaveFile( path.string() );
}

bool StateIO::import_from_file( const boost::filesystem::path& path )
{
    std::string error;
    return this->import_from_file( path, error );
}

bool StateIO::import_from_file( const boost::filesystem::path& path,
    std::string& error )
{
    if ( ! boost::filesystem::exists( path ) )
    {
        error = std::string( "File '" ) + path.string() + "' does not exist.";
        return false;
    }

  if ( !this->private_->xml_doc_.LoadFile( path.string(), TIXML_ENCODING_UTF8 ) )
  {
        error = "Could not parse the contents of the file.";
    return false;
  }
  
  this->private_->current_element_ = this->private_->xml_doc_.FirstChildElement();
  if ( this->private_->current_element_ == 0 )
  {
        error = "File does not contain valid XML tags.";
    return false;
  }

  const std::string& name = this->private_->current_element_->ValueStr();
  if ( name == Core::Application::GetApplicationName() )
  { 
    this->private_->current_element_->QueryIntAttribute( "major_version", 
      &this->private_->major_version_ );
    this->private_->current_element_->QueryIntAttribute( "minor_version", 
      &this->private_->minor_version_ );
    this->private_->current_element_->QueryIntAttribute( "patch_version", 
      &this->private_->patch_version_ );
  }
  else
  {
    this->private_->major_version_ = -1;
    this->private_->minor_version_ = -1;
    this->private_->patch_version_ = -1;
  }
  
  return true;
}

void StateIO::initialize()
{
  this->private_->xml_doc_.LinkEndChild( new TiXmlDeclaration( "1.0", "", "" ) );  

  this->private_->current_element_ = new TiXmlElement( Core::Application::GetApplicationName() );
  this->private_->current_element_->SetAttribute( "major_version", 
    Core::Application::GetMajorVersion() );
  this->private_->current_element_->SetAttribute( "minor_version", 
    Core::Application::GetMinorVersion() );
  this->private_->current_element_->SetAttribute( "patch_version", 
    Core::Application::GetPatchVersion() );
  this->private_->xml_doc_.LinkEndChild( this->private_->current_element_ );  
}

const TiXmlElement* StateIO::get_current_element() const
{
  return this->private_->current_element_;
}

TiXmlElement* StateIO::get_current_element()
{
  return this->private_->current_element_;
}

void StateIO::set_current_element( const TiXmlElement* element ) const
{
  this->private_->current_element_ = const_cast< TiXmlElement* >( element );
}

void StateIO::push_current_element() const
{
  this->private_->current_element_stack_.push( this->private_->current_element_ );
}

void StateIO::pop_current_element() const
{
  assert( !this->private_->current_element_stack_.empty() );
  if ( !this->private_->current_element_stack_.empty() )
  {
    this->private_->current_element_ = this->private_->current_element_stack_.top();
    this->private_->current_element_stack_.pop();
  }
}

int StateIO::get_major_version() const
{
  return this->private_->major_version_;
}

int StateIO::get_minor_version() const
{
  return this->private_->minor_version_;
}

int StateIO::get_patch_version() const
{
  return this->private_->patch_version_;
}

} // end namespace Core
