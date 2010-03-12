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
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{

LayerImporterInfo::LayerImporterInfo( LayerImporterBuilderBaseHandle builder,
  const std::string name, 
  const std::string file_type_string,
  const unsigned int priority ) :
  builder_( builder ),
  name_( name ),
  file_types_( Utils::split_string( file_type_string, ";" ) ),
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


LayerIO::LayerIO()
{
}

LayerIO::~LayerIO()
{
}

LayerIO::importer_types_type LayerIO::get_importer_types()
{
  lock_type lock( mutex_ ); 
  std::vector<std::string> importer_types;
  for ( size_t j = 0; j<importer_list_.size(); j++ )
  {
    importer_types.push_back( importer_list_[j]->file_type_string() );
  }
  return importer_types;
}

bool LayerIO::create_importer( const std::string& filename, 
  LayerImporterHandle& importer,
  const std::string importername )
{
  // Step (1): determine the file extension
   
  boost::filesystem::path full_filename( filename );
  // this includes the dot
  std::string extension = full_filename.extension();

  // Step (2): determine whether the file exists
  if ( ! ( boost::filesystem::exists( full_filename ) ) ) return false;
  
  // Step (3): Get the right importer
  // Lock the factory
  lock_type lock( mutex_ );

  LayerImporterInfoHandle importer_info;
  
  // If no name was given, the most appriopriate is used by
  // searching the list for the importer that deals with this
  // file type and has the highest priority
  if ( importername == "" )
  {
    unsigned int priority = 0;
    
    // Search the list for an approriate importer
    for (size_t j = 0; j < importer_list_.size(); j ++ )
    {
      if ( importer_list_[j]->converts_file_type( extension ) &&  
         importer_list_[j]->priority() >= priority )
      {
        importer_info = importer_list_[j];
      }
    }
  }
  // Else find the specific importer, this can be either through the name (using the
  // script) or through the file type string ( from the GUI)  
  else
  {
    for (size_t j = 0; j < importer_list_.size(); j ++ )
    {
      if ( ( importer_list_[j]->name() == importername ||
        importer_list_[j]->file_type_string() == importername ) &&
        importer_list_[j]->converts_file_type( extension ) )
      {
        importer_info = importer_list_[j];
      }
    }
  }
  
  if ( importer_info )
  {
    importer = importer_info->build( filename );
    return true;
  }
  else
  {
    importer.reset();
    return false;
  }
}

} // end namespace seg3D
