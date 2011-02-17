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

CORE_SINGLETON_IMPLEMENTATION( LayerIO );

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
  importer_types.push_back( "All Importers (*)" );

  for ( size_t j = 0; j< this->importer_list_.size(); j++ )
  {
    if ( this->importer_list_[j]->type() == LayerImporterType::SINGLE_FILE_E )
    {
      importer_types.push_back( this->importer_list_[j]->file_type_string() );
    }
  }
  
  return importer_types;
}

LayerIO::importer_types_type LayerIO::get_series_importer_types()
{
  lock_type lock( mutex_ ); 
  std::vector<std::string> series_importer_types;
  series_importer_types.push_back( "All Importers (*)" );

  for ( size_t j = 0; j< this->importer_list_.size(); j++ )
  {
    if ( this->importer_list_[j]->type() == LayerImporterType::FILE_SERIES_E )
    {
      series_importer_types.push_back( this->importer_list_[j]->file_type_string() );
    }
  }

  return series_importer_types;
}

LayerIO::importer_types_type LayerIO::get_exporter_types()
{
  lock_type lock( mutex_ ); 
  std::vector<std::string> exporter_types;
  exporter_types.push_back( "All Importers (*)" );

  for ( size_t j = 0; j< this->exporter_list_.size(); j++ )
  {
    exporter_types.push_back( this->exporter_list_[j]->file_type_string() );
  }

  return exporter_types;
}

bool LayerIO::create_importer( const std::vector<std::string>& filenames, LayerImporterHandle& importer,
    const std::string importername )
{
  // Step (1): clear out old results
  importer.reset();
  
  if ( filenames.size() < 1 ) return false;
  
  if ( !this->create_importer( filenames[ 0 ], importer, importername ) )
  {
    return false;
  }
  
  if ( ! importer->set_file_list( filenames ) )
  {
    importer.reset();
    return false;
  }
  
  return true;
}

bool LayerIO::create_importer( const std::string& filename, 
  LayerImporterHandle& importer,
  const std::string full_importername )
{
  // Step (1): clear out old results
  importer.reset();

  std::string importername = full_importername.substr( 0, full_importername.find( " (" ) );

  // Step (2): determine the file extension  
  boost::filesystem::path full_filename( filename );
  // this includes the dot
  std::string extension = full_filename.extension();
  std::transform( extension.begin(), extension.end(), extension.begin(), tolower );

  // Step (3): determine whether the file exists
  if ( ! ( boost::filesystem::exists( full_filename.parent_path() ) ) ) return false;
  
  // Step (4): Get the right importer
  // Lock the factory
  lock_type lock( mutex_ );
  
  // If no name was given, the most appriopriate is used by
  // searching the list for the importer that deals with this
  // file type and has the highest priority
  if ( importername == "" || importername == "All Importers" )
  {
    int priority = 0;
    
    // Search the list for an approriate importer
    for (size_t j = 0; j < this->importer_list_.size(); j ++ )
    {
      if ( this->importer_list_[j]->converts_file_type( extension, true ) &&  
         this->importer_list_[j]->priority() >= priority )
      {
        LayerImporterHandle new_importer = this->importer_list_[j]->build( filename );
        if ( new_importer->check_header() )
        {
          importer = new_importer;
          priority = this->importer_list_[j]->priority();
        }
      }
    }   

    if ( !importer )
    {
      for (size_t j = 0; j < this->importer_list_.size(); j ++ )
      {
        if ( this->importer_list_[j]->converts_file_type( extension, false ) &&  
           this->importer_list_[j]->priority() >= priority )
        {
          LayerImporterHandle new_importer = this->importer_list_[j]->build( filename );
          if ( new_importer->check_header() )
          {
            importer = new_importer;
            priority = this->importer_list_[j]->priority();
          }
        }
      }   
    }
  }
  // Else find the specific importer, this can be either through the name (using the
  // script) or through the file type string ( from the GUI)  
  else
  {
    for (size_t j = 0; j < this->importer_list_.size(); j ++ )
    {
      if ( ( this->importer_list_[j]->name() == importername ||
        this->importer_list_[j]->file_type_string() == importername ) &&
        this->importer_list_[j]->converts_file_type( extension ) )
      {
        importer = this->importer_list_[j]->build( filename );
        break;
      }
    }
  }

  if ( importer )
  {
    return true;
  }
  else return false;
}

bool LayerIO::create_exporter( LayerExporterHandle& exporter, std::vector< LayerHandle >& layers, 
  const std::string importername /*= ""*/, const std::string extension /*= "" */ )
{
  for (size_t j = 0; j < this->exporter_list_.size(); j ++ )
  {
    if ( ( this->exporter_list_[ j ]->name() == importername ||
      this->exporter_list_[ j ]->file_type_string() == importername ) &&
      this->exporter_list_[ j ]->converts_file_type( extension ) )
    {
      exporter = this->exporter_list_[ j ]->build( layers );
      exporter->set_extension( extension );
      return true;
    }
  }
  return false;
}

} // end namespace seg3D
