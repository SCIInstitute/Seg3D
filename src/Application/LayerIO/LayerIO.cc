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
  // Step (1): clear out old results
  importer.reset();

  // Step (2): determine the file extension  
  boost::filesystem::path full_filename( filename );
  // this includes the dot
  std::string extension = full_filename.extension();

  // Step (3): determine whether the file exists
  if ( ! ( boost::filesystem::exists( full_filename ) ) ) return false;
  
  // Step (4): Get the right importer
  // Lock the factory
  lock_type lock( mutex_ );
  
  // If no name was given, the most appriopriate is used by
  // searching the list for the importer that deals with this
  // file type and has the highest priority
  if ( importername == "" || importername == "All Importers (*)" )
  {
    unsigned int priority = 0;
    
    // Search the list for an approriate importer
    for (size_t j = 0; j < importer_list_.size(); j ++ )
    {
      if ( importer_list_[j]->converts_file_type( extension ) &&  
         importer_list_[j]->priority() >= priority )
      {
        LayerImporterHandle new_importer = importer_list_[j]->build( filename );
        if ( new_importer->check_header() )
        {
          importer = new_importer;
          priority = importer_list_[j]->priority();
        }
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
        importer = importer_list_[j]->build( filename );
        break;
      }
    }
  }

  if ( importer ) return true;
  else return false;
}

} // end namespace seg3D
