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

// Specific includes for reading large datafiles
// NOTE: We needed to special case Windows as VS 2008 has an improper 64bit seek function in its
// STL implementation.
#ifdef _WIN32
#include <windows.h>
#else
#include <fstream>
#endif

// Core includes
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/Layer/DataLayer.h> 
#include <Application/LayerIO/VFFLayerImporter.h>
#include <Application/Layer/LayerManager.h>

SEG3D_REGISTER_IMPORTER( Seg3D, VFFLayerImporter );

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class VFFLayerImporterPrivate
//////////////////////////////////////////////////////////////////////////

class VFFLayerImporterPrivate : public boost::noncopyable
{
public:
  VFFLayerImporterPrivate() :
    importer_( 0 ),
    data_type_( Core::DataType::UNKNOWN_E ),
    vff_end_of_header_( 0 ),
    read_header_( false ),
    read_data_( false )
  {
  }
  
  // Pointer back to the main class
  VFFLayerImporter* importer_;
  
public:
  // Datablock that was extracted
  Core::DataBlockHandle data_block_;

  // Grid transform that was extracted
  Core::GridTransform grid_transform_;
  
  // Type of the pixels in the file
  Core::DataType data_type_;

  // Where the actual data is located
  size_t vff_end_of_header_;
  
  // Whether the header has been read
  bool read_header_;

  // Whether the data has been read
  bool read_data_;

public:
  // READ_HEADER
  // Read the header of the file
  bool read_header();
  
  // READ_DATA
  // Read the data from the file
  bool read_data();
};

bool VFFLayerImporterPrivate::read_header()
{
  // If read it before, we do need read it a second time.
  if ( this->read_header_ ) return true;

  // Check the the magic at the top of the file
  // The first line needs to be "ncaa"
  std::ifstream vff_test_peek( this->importer_->get_filename().c_str() );
  std::string ncaa;
  std::getline(  vff_test_peek, ncaa );
  if ( ncaa.substr( 0, 4 ) != "ncaa" ) 
  {
    this->importer_->set_error( "File is not a VFF file." );
    return false;
  }
  vff_test_peek.close();

  // Now that we know that we are good to go, we read in and store the header information in a map
  std::ifstream file_data( this->importer_->get_filename().c_str(), std::ios::binary );

  // Store the key/value pairs in a map
  std::map< std::string, std::string > vff_values;

  // Read the header line by line
  std::string line;
  while( !file_data.eof() )
  {
    // Grab the next line
    std::getline( file_data, line );

    // If it is the last one, break out of the loop
    if ( "\f" == line ) 
    {
      this->vff_end_of_header_ = static_cast<size_t>( file_data.tellg() );
      break;
    }
    
    if ( line[ 0 ] == '#' ) continue; // here we skip comments
    if ( line.find_first_of( "=" ) == std::string::npos ) continue; // we skip invalid lines
    line.erase( line.find( ";" ) ); // we clean out the semicolon's from the lines
    
    // now we split each line using '=' as the delimiter and store the pair in our map
    std::vector< std::string > key_value_pairs = Core::SplitString( line, "=" );
    vff_values[ key_value_pairs[ 0 ] ] = key_value_pairs[ 1 ];
  }

  // Check if any entries were given
  if ( vff_values.empty() ) 
  {
    this->importer_->set_error( "This vff file has incomplete header." );
    return false;
  }
  
  // We need the bits field
  if ( vff_values.find( "bits" ) == vff_values.end() ) 
  {
    this->importer_->set_error( "This vff file misses information on the number of bits of the data." );
    return false;
  }
  
  // Check which data type is contained in the file
  if ( vff_values[ "bits" ] == "16" )
  {
    this->data_type_ = Core::DataType::SHORT_E;
  }
  else if ( vff_values[ "bits" ] == "8" )
  {
    this->data_type_ = Core::DataType::UCHAR_E;
  }
  else
  {
    this->importer_->set_error( "This vff file contains an unknown data type." );
    return false;
  }
  
  // We check to see if the header contained the size, if so we use them, otherwise 
  // we return false
  if ( vff_values.find( "size" ) == vff_values.end() )
  {
    this->importer_->set_error( "No size is given in the vff header." );
    return false;
  }

  // Get the dimensions of the data
  std::vector<size_t> dim;
  Core::ImportFromString( vff_values[ "size" ], dim );

  if ( dim.size() != 3 )
  {
    this->importer_->set_error( "Vff file is not a 3D volume." );
    return false;
  }
  
  // We check to see if the header contained the origin, if so we use it, otherwise
  // we use default value. 
  Core::Point origin; 
  if( vff_values.find( "origin" ) != vff_values.end() ) 
  {
    if (!( Core::ImportFromString( vff_values[ "origin" ], origin ) ) )
    {
      origin = Core::Point( 0.0, 0.0, 0.0 );
    }
  }

  // Similar check for spacing.
  Core::Vector spacing;
  if( vff_values.find( "spacing" ) != vff_values.end() ) 
  {
    if (!( Core::ImportFromString( vff_values[ "spacing" ], spacing ) ) )
    {
      spacing = Core::Vector( 1.0, 1.0, 1.0 );
    }
  }

  // Generate the grid transform that describes the data
  Core::Transform transform( origin, Core::Vector( spacing.x(), 0.0 , 0.0 ), 
    Core::Vector( 0.0, spacing.y(), 0.0 ), Core::Vector( 0.0, 0.0, spacing.z() ) );

  this->grid_transform_ = Core::GridTransform( dim[ 0 ], dim[ 1 ], dim[ 2 ], transform );
  this->grid_transform_.set_originally_node_centered( false );

  // Indicate that we read the header.
  this->read_header_ = true;

  return true;
}

bool VFFLayerImporterPrivate::read_data()
{
  // Check if we already read the data.
  if ( this->read_data_ ) return true;

  // Ensure that we read the header of this file.
  if ( ! this->read_header() ) 
  {
    this->importer_->set_error( "Failed to read header of vff file." );
    return false;
  }

  // Generate a new data block
  this->data_block_ = Core::StdDataBlock::New( this->grid_transform_.get_nx(), 
    this->grid_transform_.get_ny(), this->grid_transform_.get_nz(), this->data_type_ );

  // We need to check if we could allocate the destination datablock
  if ( !this->data_block_ )
  {
    this->importer_->set_error( "Could not allocate enough memory to read vff file." );
    return false;
  }

  // Now we read in the file
  // NOTE: We had to split this out due to a bug in Visual Studio's implementation for filestreams
  // These are unfortunately not 64bit compatible and hence use 32bit integers to denote offsets
  // into a file. Hence this breaks large data support. Hence in the next piece of code we deal
  // with windows separately.
  
#ifdef _WIN32
  HANDLE file_desc = CreateFileA( this->importer_->get_filename().c_str(), GENERIC_READ,
    FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if ( file_desc == INVALID_HANDLE_VALUE ) 
  {
    this->importer_->set_error( "Could not open file." );
    return false;
  }
#else
  std::ifstream data_file( this->importer_->get_filename().c_str(), 
    std::ios::in | std::ios::binary );
  if( !data_file )
  {
    this->importer_->set_error( "Could not open file." );
    return false;
  }
#endif

  // We start by getting the length of the entire file
#ifdef _WIN32
  LARGE_INTEGER offset; offset.QuadPart = 0;
    SetFilePointerEx( file_desc, offset, NULL, FILE_END);
#else
  data_file.seekg( 0, std::ios::end );
#endif

  // Next we compute the length of the data by subtracting the size of the header
  size_t file_size = 0;

#ifdef _WIN32
  offset.QuadPart = 0;
    LARGE_INTEGER cur_pos;
  SetFilePointerEx( file_desc, offset, &cur_pos, FILE_CURRENT );
  file_size = cur_pos.QuadPart + 1; 
#else
  file_size = static_cast<size_t>( data_file.tellg() ) + 1;
#endif
  
  // Ensure that the vff file is of the right length
  size_t length = this->data_block_->get_size() * Core::GetSizeDataType( this->data_type_ );  
  if ( file_size - ( this->vff_end_of_header_ ) < length )
  {
    this->importer_->set_error( "Incorrect length of file." );
    return false;
  }

  // We move the reader's position back to the front of the file and then to the start of the data
  char* data = reinterpret_cast<char *>( this->data_block_->get_data() );

#ifdef _WIN32
  offset.QuadPart = this->vff_end_of_header_;
    SetFilePointerEx( file_desc, offset, NULL, FILE_BEGIN );
#else
  data_file.seekg( this->vff_end_of_header_, std::ios::beg );
#endif

  // Finally, we do the actual reading and then save the data to the data block.
#ifdef _WIN32
  // NOTE: For windows we need to divide the read in chuncks due to limitations in the
  // sizes we can store in the ReadFile function.
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
  
  // Close the file
#ifdef _WIN32
  CloseHandle( file_desc );
#else
  data_file.close();
#endif

  // VFF data is always stored as big endian data
  // Hence we need to swap if we are on a little endian system.
  if ( Core::DataBlock::IsLittleEndian() )
  {
    this->data_block_->swap_endian();
  }

  // Mark that we have read the data.
  this->read_data_ = true;

  // Done
  return true;
}

//////////////////////////////////////////////////////////////////////////
// Class VFFLayerImporter
//////////////////////////////////////////////////////////////////////////

VFFLayerImporter::VFFLayerImporter() :
  private_( new VFFLayerImporterPrivate )
{
  // Ensure that the private class has a pointer back into this class.
  this->private_->importer_ = this;
}

VFFLayerImporter::~VFFLayerImporter()
{
}

bool VFFLayerImporter::get_file_info( LayerImporterFileInfoHandle& info )
{
  try
  { 
    // Try to read the header
    if ( ! this->private_->read_header() ) return false;
  
    // Generate an information structure with the information.
    info = LayerImporterFileInfoHandle( new LayerImporterFileInfo );
    info->set_data_type( this->private_->data_type_ );
    info->set_grid_transform( this->private_->grid_transform_ );
    info->set_file_type( "vff" ); 
    info->set_mask_compatible( true );
  }
  catch ( ... )
  {
    // In case something failed, recover from here and let the user
    // deal with the error. 
    this->set_error( "VFF Importer crashed while reading file." );
    return false;
  }
    
  return true;
}


bool VFFLayerImporter::get_file_data( LayerImporterFileDataHandle& data )
{
  try
  { 
    // Read the data from the file
    if ( !this->private_->read_data() ) return false;
  
    // Create a data structure with handles to the actual data in this file 
    data = LayerImporterFileDataHandle( new LayerImporterFileData );
    data->set_data_block( this->private_->data_block_ );
    data->set_grid_transform( this->private_->grid_transform_ );
    data->set_name( this->get_file_tag() );
  }
  catch ( ... )
  {
    // In case something failed, recover from here and let the user
    // deal with the error. 
    this->set_error( "VFF Importer crashed when reading file." );
    return false;
  }

  return true;
}

} // end namespace seg3D
