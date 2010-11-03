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

#ifdef _WIN32
// Windows includes
#include <windows.h>
#else
// STL includes
#include <fstream>
#endif



// Boost Includes
#include <boost/lexical_cast.hpp>

// Core includes
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/LayerIO/VFFLayerImporter.h>

// Application includes
#include <Application/Layer/DataLayer.h> 
#include <Application/LayerManager/LayerManager.h>

SCI_REGISTER_IMPORTER( Seg3D, VFFLayerImporter );


namespace Seg3D
{

VFFLayerImporter::VFFLayerImporter( const std::string& filename ) :
  LayerImporter( filename ),
  pixel_type_( Core::DataType::SHORT_E )
{
  // set a default value for the grid transform
  this->grid_transform_ = Core::GridTransform( 1, 1, 1 );
}

bool VFFLayerImporter::import_header()
{
  // First thing we do is set the extension
  this->set_extension();

  if( this->extension_ == ".vff" )
  {
    return this->scan_vff();
  }
  return false; 
}

Core::GridTransform VFFLayerImporter::get_grid_transform()
{
  return this->grid_transform_;
}

Core::DataType VFFLayerImporter::get_data_type()
{
  return this->pixel_type_;
}

int VFFLayerImporter::get_importer_modes()
{
  return LayerImporterMode::DATA_E;
}

bool VFFLayerImporter::scan_vff()
{
  // First we will peek at the file to see if it has the proper header.
  std::ifstream vff_test_peek( this->get_filename().c_str() );
  std::string ncaa;
  std::getline(  vff_test_peek, ncaa );
  if( ncaa.substr( 0, 4 ) != "ncaa" ) return false;
  vff_test_peek.close();

  // Now that we know that we are good to go, we read in and store the header information in a map
  std::ifstream file_data( this->get_filename().c_str(), std::ios::binary );
  std::string line;
  std::string end_of_header;
  end_of_header.push_back(12);

  while( !file_data.eof() )
  {
    if ( end_of_header == line ) 
    {
      this->vff_end_of_header_ = 1 + static_cast<size_t>( file_data.tellg() );
      break;
    }
    std::getline( file_data, line );
    if ( line[ 0 ] == '#' ) continue; // here we skip comments
    if ( line.find_first_of( "=" ) == std::string::npos ) continue; // we skip invalid lines
    line.erase( line.find( ";" ) ); // we clean out the semicolon's from the lines
    
    // now we split each line using '=' as the delimiter and store the pair in our map
    std::vector< std::string > key_value_pairs = Core::SplitString( line, "=" );
    this->vff_values_[ key_value_pairs[ 0 ] ] = key_value_pairs[ 1 ];
  }

  if( this->vff_values_.empty() ) return false;
  if( this->vff_values_.find( "bits" ) == this->vff_values_.end() ) return false;
  if( this->vff_values_[ "bits" ] == "16" ) this->pixel_type_ = Core::DataType::SHORT_E;
  if( this->vff_values_[ "bits" ] == "8" ) this->pixel_type_ = Core::DataType::UCHAR_E;

  return true;
}

bool VFFLayerImporter::import_vff()
{
  // First we check to see if the file was scanned and the map of values was filled
  if( this->vff_values_.empty() ) return false;

  // Step 1: we check to see if the header contained the size, if so we use them, otherwise 
  // we return false
  if( this->vff_values_.find( "size" ) == this->vff_values_.end() ) return false;
  std::vector< std::string > size = Core::SplitString( this->vff_values_[ "size" ], " " );
  size_t nx = boost::lexical_cast< size_t >( size[ 0 ] );
  size_t ny = boost::lexical_cast< size_t >( size[ 1 ] );
  size_t nz = boost::lexical_cast< size_t >( size[ 2 ] );

  // Step 2: we check to see if the header contained the origin, if so we use it, otherwise
  // we return false
  if( this->vff_values_.find( "origin" ) == this->vff_values_.end() ) return false;
  std::vector< std::string > origin_vector = 
    Core::SplitString( this->vff_values_[ "origin" ], " " );
  Core::Point point = Core::Point( 
    boost::lexical_cast< double >( origin_vector[ 0 ] ), 
    boost::lexical_cast< double >( origin_vector[ 1 ] ), 
    boost::lexical_cast< double >( origin_vector[ 2 ] ) );

  // Step 3: we check to see if the header contained the spacing, if so we use them, otherwise
  // we return false
  if( this->vff_values_.find( "spacing" ) == this->vff_values_.end() ) return false;
  std::vector< std::string > spacing_vector = 
    Core::SplitString( this->vff_values_[ "spacing" ], " " );
  Core::Transform transform = Core::Transform( point, 
    Core::Vector( boost::lexical_cast< double >( spacing_vector[ 0 ] ), 0.0 , 0.0 ), 
    Core::Vector( 0.0, boost::lexical_cast< double >( spacing_vector[ 1 ] ), 0.0 ), 
    Core::Vector( 0.0, 0.0, boost::lexical_cast< double >( spacing_vector[ 2 ] ) ) );


  size_t data_type_size = 1;

  // Step 4: now we instantiate a DataBlock based on the type of pixel that we expect
  if( this->pixel_type_ == Core::DataType::SHORT_E )
  {
    this->data_block_ = Core::StdDataBlock::New( nx, ny, nz, Core::DataType::SHORT_E );
    data_type_size = 2;
  }
  else if( this->pixel_type_ == Core::DataType::UCHAR_E )
  {
    this->data_block_ = Core::StdDataBlock::New( nx, ny, nz, Core::DataType::UCHAR_E );
  }
  else
  {
    return false;
  }

  // Step 4: now we read in the file
#ifdef _WIN32
  HANDLE file_desc = CreateFileA( this->get_filename().c_str(), GENERIC_READ,
                             FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if ( file_desc == INVALID_HANDLE_VALUE ) return false;

#else
  std::ifstream data_file( this->get_filename().c_str(), std::ios::in | std::ios::binary );
  if( !data_file ) return false;
#endif


  size_t length = this->data_block_->get_size() * data_type_size;
  
  // Step 4a: we start by getting the length of the entire file

#ifdef _WIN32
  LARGE_INTEGER offset; offset.QuadPart = 0;
    SetFilePointerEx( file_desc, offset, NULL, FILE_END);
#else
  data_file.seekg( 0, std::ios::end );
#endif

  // Step 4b: next we compute the length of the data by subtracting the size of the header

  // TODO: Need to make sure this does the right thing under 32bits
  size_t file_size = 0;

#ifdef _WIN32
  offset.QuadPart = 0;
    LARGE_INTEGER cur_pos;
  SetFilePointerEx( file_desc, offset, &cur_pos, FILE_CURRENT );
  file_size = cur_pos.QuadPart + 1; 
#else
  file_size = static_cast<size_t>( data_file.tellg() ) + 1;
#endif
  
  if ( file_size - ( this->vff_end_of_header_ ) < length )
  {
    // CORRUPT FILE
    return false;
  }

    
  // Step 4c: then, after we make place for the data, we move the reader's position back to the
  // front of the file and then to the start of the data;
  char* data = reinterpret_cast<char *>( this->data_block_->get_data() );

#ifdef _WIN32
  offset.QuadPart = this->vff_end_of_header_;
    SetFilePointerEx( file_desc, offset, NULL, FILE_BEGIN );
#else
  data_file.seekg( this->vff_end_of_header_, std::ios::beg );
#endif

  // Step 4d: finally, we do the actual reading and then save the data to the data block.
#ifdef _WIN32

  char* data_ptr = data;
    DWORD dwReadBytes;
  size_t read_length = length;
  size_t chunk = 1UL<<30;
  while ( read_length > chunk )
  {
    ReadFile( file_desc, data_ptr, DWORD(chunk), &dwReadBytes, NULL );
    read_length -= static_cast<size_t>( dwReadBytes );
    data_ptr += static_cast<size_t>( dwReadBytes );
  }
  ReadFile( file_desc, data_ptr, DWORD( read_length ), &dwReadBytes, NULL );

#else
  data_file.read( data, length );
#endif
  
#ifdef _WIN32
  CloseHandle( file_desc );
#else
  data_file.close();
#endif

  // Step 5: now we set our grid transform.
  this->grid_transform_ = Core::GridTransform( nx, ny, nz, transform );
  return true;
}

bool VFFLayerImporter::load_data( Core::DataBlockHandle& data_block, 
                 Core::GridTransform& grid_trans )
{
  if( this->extension_ == ".vff" )
  {
    if( !this->import_vff() ) return false;
  }

  if( !this->data_block_ ) return false;

  data_block = this->data_block_;
  grid_trans = this->grid_transform_;

  return true;
}

} // end namespace seg3D
