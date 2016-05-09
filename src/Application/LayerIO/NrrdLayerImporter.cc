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
#include <fstream>

// Boost includes
#include <boost/filesystem.hpp>

// Application includes
#include <Application/LayerIO/NrrdLayerImporter.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

SEG3D_REGISTER_IMPORTER( Seg3D, NrrdLayerImporter );

namespace Seg3D
{

// Private implementation
class NrrdLayerImporterPrivate
{
public:
  Core::NrrdDataHandle nrrd_data_;
};


NrrdLayerImporter::NrrdLayerImporter() :
  private_( new NrrdLayerImporterPrivate )
{
}

NrrdLayerImporter::~NrrdLayerImporter()
{
}

bool NrrdLayerImporter::get_file_info( LayerImporterFileInfoHandle& info )
{
  if ( !this->private_->nrrd_data_ )
  {
    // NOTE: We load the full data set, as Teem does not support reading headers only.
    // Hence we need to read the full file
    std::string error;
    if ( ! ( Core::NrrdData::LoadNrrd( get_filename() , this->private_->nrrd_data_, error ) ) )
    {
      this->set_error( error );
      return false;
    } 
  }
  
  info = LayerImporterFileInfoHandle( new LayerImporterFileInfo );
  info->set_data_type( this->private_->nrrd_data_->get_data_type() );
  info->set_grid_transform( this->private_->nrrd_data_->get_grid_transform() );
  info->set_file_type( "nrrd" );
  info->set_mask_compatible( true );
  
  // Done
  return true;
}

bool NrrdLayerImporter::get_file_data( LayerImporterFileDataHandle& data )
{
  if ( !this->private_->nrrd_data_ )
  {
    // NOTE: We load the full data set, as Teem does not support reading headers only.
    // Hence we need to read the full file
    std::string error;
    if ( ! ( Core::NrrdData::LoadNrrd( get_filename() , this->private_->nrrd_data_, error ) ) )
    {
      this->set_error( error );
      return false;
    } 
  }
  
  data = LayerImporterFileDataHandle( new LayerImporterFileData );
  data->set_grid_transform( this->private_->nrrd_data_->get_grid_transform() );
  
  data->set_data_block( Core::NrrdDataBlock::New( this->private_->nrrd_data_ ) );
  data->set_name( this->get_file_tag() );
    
  // Done
  return true;
}



bool CopyNrrdFile( const boost::filesystem::path& src, 
  const boost::filesystem::path& dst )
{
  bool has_detached_header = false;
  
  try
  {
    std::ifstream file_header( src.string().c_str(), std::ios::binary );

    // Scan whether file has a detached header
    std::string line;
    while( !file_header.eof() )
    {
      // Grab the next line
      std::getline( file_header, line );
      if ( line.empty() ) break;

      if ( line.size() > 10 && line.substr( 0, 10 ) == "data file:" )
      {
        has_detached_header = true;
        break;
      }
    }
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( std::string( "Could not read nrrd file '" ) + src.string() + "'." );
    return false;
  }

  if ( !has_detached_header )
  {
    try
    {
      boost::filesystem::copy_file( src, dst );
    }
    catch ( ... )
    {
      CORE_LOG_ERROR( std::string( "Could not copy file '" ) + src.string() + "' to '" +
        dst.string() + "'." );
      return false;
    }
    
    return true; 
  }

  std::vector< boost::filesystem::path > data_files;

  // Copy and update header file    
  try
  {
    boost::filesystem::path input_header_name( src.string() );
    boost::filesystem::path output_header_name( dst.string() );
  
    std::ifstream old_file_header( input_header_name.filename().c_str(), std::ios::binary );
    std::ofstream new_file_header( output_header_name.filename().c_str(), std::ios::binary );
  
    bool need_to_write_file_list = false;
    
    while( !old_file_header.eof() )
    {
      // Grab the next line
      std::string line;
      std::getline( old_file_header, line );
      if ( line.empty() ) break;

      if ( line.size() > 10 && line.substr( 0, 10 ) == "data file:" )
      {
        std::string contents = line.substr( 10 );
        std::vector<std::string> components;
        if ( Core::ImportFromString( contents, components ) )
        {
          CORE_LOG_ERROR( std::string( "Cannot interpret the data file in header of nrrd file '" ) 
            + src.string() + "'." );
          return false;
        }

        // Check the three options
        if ( components.size() == 1 ||  components.size() == 2 )
        {
          if ( components[ 0 ] == "LIST" )
          {
            // MetaIO format
            new_file_header << "data file: LIST" << std::endl;
            
            // Copy over every entry and fix the name of the file to be local
            while( !old_file_header.eof() )
            {
              std::getline( old_file_header, line );
              if ( line.empty() ) break;
              boost::filesystem::path filename( line );
              data_files.push_back( filename );
              need_to_write_file_list = true;
            }
            
            // Translation is complete there are no entries after LIST
            break;
          }
          else
          {
            // The most common case, it just refers directly to a file
            boost::filesystem::path filename( components[ 0 ] );
            new_file_header << "data file: " << filename.filename() << std::endl;
            data_files.push_back( filename );
          }
        }
        else if ( components.size() == 4 || components.size() == 5 )
        {
          std::vector<char> buffer( components[ 0 ].size() + 20, '\0' );
          int start, stop, step;
          if ( !Core::ImportFromString( components[ 1 ], start ) ||
            !Core::ImportFromString( components[ 2 ], stop ) ||
            !Core::ImportFromString( components[ 3 ], step ) )
          {
            CORE_LOG_ERROR( std::string( "Could not interpret 'data file' field in header." )
              + src.string() + "'." );
            return false;
          }
          
          for (int index = start; index <= stop; index += step )
          {         
            snprintf( &buffer[ 0 ], buffer.size() - 1, components[ 0 ].c_str(), index );
            boost::filesystem::path filename( &buffer[ 0 ] );
            data_files.push_back( filename );           
          }
          need_to_write_file_list = true;
        }
        else
        {
          CORE_LOG_ERROR( std::string( "Invalid entry in 'data file' field in the header." )
              + src.string() + "'." );            
          return false;
        }
      }
      else
      {
        // Copy the entry to cached file
        new_file_header << line << std::endl;
      }
    }   
  
    // Data file list needs to be at the end of the file
    if ( need_to_write_file_list )
    {
      new_file_header << "data file: LIST" << std::endl;
      for ( size_t j = 0; j < data_files.size(); j++ )
      {
        new_file_header << data_files[ j ].filename() << std::endl;
      }
    }
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "Could not copy and update header file for nhdr file." );
    return false; 
  }

  // Copy the raw data files to the local cache directory
  for ( size_t j = 0; j < data_files.size(); j++ )
  {
    try
    {
      boost::filesystem::copy_file( data_files[ j ], 
        dst.parent_path() / data_files[ j ].filename() );
    }
    catch( ... )
    {
      CORE_LOG_ERROR( std::string( "Could not copy file '" ) + data_files[ j ].string() + "'." );
      return false;
    }
  }

  // Successfully copied data file and generated new header.
  return true;  
}


InputFilesImporterHandle NrrdLayerImporter::get_inputfiles_importer()
{
  InputFilesImporterHandle importer( new InputFilesImporter( this->get_inputfiles_id() ) );
  try
  {
    boost::filesystem::path full_filename( this->get_filename() );
    importer->add_filename( full_filename );
    importer->set_copy_file_function( &CopyNrrdFile );
  }
  catch ( ... )
  {
    this->set_error( std::string( "Could not resolve filename '" ) + 
      this->get_filename() + "'." );
  }
  
  return importer;
}



} // end namespace seg3D
