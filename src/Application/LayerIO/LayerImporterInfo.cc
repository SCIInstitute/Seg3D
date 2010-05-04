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

// Boost includes
#include <boost/filesystem.hpp>

// Application includes
#include <Application/LayerIO/LayerImporterInfo.h>

namespace Seg3D
{

LayerImporterInfo::LayerImporterInfo( LayerImporterBuilderBaseHandle builder,
  const std::string name, 
  const std::string file_type_string,
  const unsigned int priority ) :
  builder_( builder ),
  name_( name ),
  file_types_( Core::SplitString( file_type_string, ";" ) ),
  any_type_(false),
  priority_( priority )
{
  if (file_types_.size() > 0)
  {
    if (file_types_[0] == "*") any_type_ = true;
  }

  if ( any_type_ ) 
  {
    file_type_string_ = name_ + std::string(" (*)");
  }
  else
  {
    file_type_string_ = name_ + std::string( " (" );
    for ( size_t j = 0; j < file_types_.size(); j++ )
    {
      file_type_string_ += std::string("*")+file_types_[j];
      if ( j != ( file_types_.size() - 1 ) ) file_type_string_ += std::string( " " );
    }
    file_type_string_ += std::string( ")" );
  }
}

LayerImporterInfo::~LayerImporterInfo()
{
}

std::string LayerImporterInfo::name() const 
{ 
  return name_;
}

LayerImporterHandle LayerImporterInfo::build( const std::string& filename ) const
{
  return builder_->build( filename );
}


bool LayerImporterInfo::converts_file_type( const std::string& file_type ) const
{
  if ( any_type_ ) return ( true );
  return ( std::find( file_types_.begin(), file_types_.end(), file_type ) != file_types_.end() );
}

std::string LayerImporterInfo::file_type_string() const
{
  return file_type_string_;
}

unsigned int LayerImporterInfo::priority() const
{
  return priority_;
}

} // end namespace seg3D
