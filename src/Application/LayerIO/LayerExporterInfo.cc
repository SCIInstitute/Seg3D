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
#include <boost/filesystem.hpp>

// Application includes
#include <Application/LayerIO/LayerExporterInfo.h>

namespace Seg3D
{

LayerExporterInfo::LayerExporterInfo( LayerExporterBuilderBaseHandle builder,
  const std::string name, 
  const std::string file_type_string ) :
  builder_( builder ),
  name_( name ),
  file_types_( Core::SplitString( file_type_string, ";" ) ),
  any_type_(false)
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

LayerExporterInfo::~LayerExporterInfo()
{
}

std::string LayerExporterInfo::get_name() const 
{ 
  return name_;
}

LayerExporterHandle LayerExporterInfo::build( std::vector< LayerHandle >& layers ) const
{
  return builder_->build( layers );
}


bool LayerExporterInfo::converts_file_type( const std::string& file_type ) const
{
  if ( any_type_ ) return ( true );
  return ( std::find( file_types_.begin(), file_types_.end(), file_type ) != file_types_.end() );
}

std::string LayerExporterInfo::get_file_type_string() const
{
  return file_type_string_;
}

} // end namespace seg3D
