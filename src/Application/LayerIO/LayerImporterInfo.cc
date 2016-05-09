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
#include <algorithm>

// Boost includes
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

// Application includes
#include <Application/LayerIO/LayerImporterInfo.h>

namespace Seg3D
{

class LayerImporterInfoPrivate
{
  // -- constructor --
public:
  LayerImporterInfoPrivate() :
    type_( LayerImporterType::SINGLE_FILE_E ),
    any_type_( false ),
    priority_( 0 )
  {
  }
  
  // -- information --
public: 
  // Type of this importer
  LayerImporterType type_;

  // Name of the importer
  std::string name_;
  
  // The file types the importer will handle
  std::vector<std::string> file_types_; 
  
  // String with name and file types, to be used as a filter in a file selector
  std::string file_type_string_;
  
  // Any file type, e.g. dicom readers tend to use no extension
  bool any_type_;
  
  // The priority of the importer
  int priority_;  

  // Object that knows how to build the importer
  LayerImporterBuilderBaseHandle builder_;
};


LayerImporterInfo::LayerImporterInfo( LayerImporterBuilderBaseHandle builder, std::string name, 
  std::string file_type_string, int priority, LayerImporterType type ) :
    private_( new LayerImporterInfoPrivate() )
{
  this->private_->builder_ = builder;
  this->private_->name_ = name;
  this->private_->file_types_ = Core::SplitString( file_type_string, ";" );
  this->private_->priority_ = priority;
  this->private_->type_ = type;

  if ( this->private_->file_types_.size() > 0 )
  {
    if ( this->private_->file_types_[0] == "*" ) 
    {
      this->private_->any_type_ = true;
    }
  }

  if ( this->private_->any_type_ ) 
  {
    this->private_->file_type_string_ = this->private_->name_ + std::string( " (* " );
    for ( size_t j = 1; j < this->private_->file_types_.size(); j++ )
    {
      this->private_->file_type_string_ += std::string( "*" ) + 
        this->private_->file_types_[ j ];
      if ( j != ( this->private_->file_types_.size() - 1 ) )
      {
        this->private_->file_type_string_ += std::string( " " );
      }
    }
    this->private_->file_type_string_ += std::string( ")" );
  }
  else
  {
    this->private_->file_type_string_ = this->private_->name_ + std::string( " (" );
    for ( size_t j = 0; j < this->private_->file_types_.size(); j++ )
    {
      this->private_->file_type_string_ += std::string( "*" ) + 
        this->private_->file_types_[ j ];
      if ( j != ( this->private_->file_types_.size() - 1 ) )
      {
        this->private_->file_type_string_ += std::string( " " );
      }
    }
    this->private_->file_type_string_ += std::string( ")" );
  }
}

LayerImporterInfo::~LayerImporterInfo()
{
}

LayerImporterType LayerImporterInfo::get_type() const
{
  return this->private_->type_;
}

std::string LayerImporterInfo::get_name() const 
{ 
  return this->private_->name_;
}

LayerImporterHandle LayerImporterInfo::build() const
{
  return this->private_->builder_->build();
}

bool LayerImporterInfo::converts_file_type( const std::string& file_type, bool strict ) const
{
  if ( this->private_->any_type_ && !strict ) return true;
  std::string lower_file_type = boost::to_lower_copy( file_type );
  return ( std::find( this->private_->file_types_.begin(), this->private_->file_types_.end(), 
    lower_file_type ) != this->private_->file_types_.end() );
}

std::string LayerImporterInfo::get_file_type_string() const
{
  return this->private_->file_type_string_;
}

int LayerImporterInfo::get_priority() const
{
  return this->private_->priority_;
}

} // end namespace seg3D
