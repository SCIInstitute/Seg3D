/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPpwd
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

// boost includes
#include <boost/algorithm/string/case_conv.hpp>

// ITK includes
#include <itkRGBPixel.h>
#include <itkTIFFImageIO.h>
#include <itkVTKImageIO.h>
#include <itkLSMImageIO.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkAnalyzeImageIO.h>
#include <itkNiftiImageIO.h>
#include <itkMetaImageIO.h>
#include <itkImageSeriesReader.h>
#include <gdcmException.h>

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/DataBlock/ITKImageData.h>
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/Volume/DataVolume.h>
#include <Core/Utils/FilesystemUtil.h>

// Application includes
#include <Application/LayerIO/ITKLayerImporter.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

SEG3D_REGISTER_IMPORTER( Seg3D, ITKLayerImporter );

namespace Seg3D
{

class ITKLayerImporterPrivate
{

public:
  ITKLayerImporterPrivate() :
    data_type_( Core::DataType::UCHAR_E ),
    read_header_( false ),
    read_data_( false )
  {
  }
  
  // Pointer back to the main class
  ITKLayerImporter* importer_;

public: 
  // READ_HEADER
  // Read the header of the file
  bool read_header();
  
  // READ_DATA
  // Read the data from the file
  bool read_data();
  
  // CONVERT_DATA_TYPE:
  // Copy the data type we get from itk and convert to a Seg3D enum type
  Core::DataType convert_data_type( std::string& type );

  // SCAN_SIMPLE_VOLUME:
  // Scan the data file
  template< class ItkImporterType >
  bool scan_simple_volume();
  
  // IMPORT_SIMPLE_TYPED_VOLUME:
  // Read the data in its final format
  template< class DataType, class ItkImporterType >
  bool import_simple_typed_volume();

  // IMPORT_SIMPLE_VOLUME:
  // Import the volume in its final format by choosing the right format
  template< class ItkImporterType >
  bool import_simple_volume();

  
public:
  // File type that we are importing
  std::string file_type_;

  // The data type of the data we are importing
  Core::DataType data_type_;

  // The transform of the image we are importing
  Core::GridTransform grid_transform_;

  // The data that was read from the file
  Core::DataBlockHandle data_block_;
  
  // Whether the header was read
  bool read_header_;
  
  // Whether the data was read
  bool read_data_;

public:
  // file type detection helpers
  bool detect_tiff( const std::string& extension )
  {
    return ( extension == ".tif" || extension == ".tiff" || extension == ".stk" );
  }

  bool detect_vtk( const std::string& extension )
  {
    return extension == ".vtk";
  }

  bool detect_lsm( const std::string& extension )
  {
    return extension == ".lsm";
  }

  bool detect_analyze( const std::string& extension )
  {
    return ( extension == ".img" || extension == ".hdr" );
  }
  
  bool detect_nifti( const std::string& extension )
  {
    return ( extension == ".nii" || extension == ".nii.gz" );
  }

  bool detect_metaimage( const std::string& extension )
  {
    return ( extension == ".mha" || extension == ".mhd" );
  }
};

Core::DataType ITKLayerImporterPrivate::convert_data_type( std::string& type )
{
  // Convert ITK types into our enum
  if( type == "unsigned_char" )
  {
    return Core::DataType::UCHAR_E;
  }
  else if( type == "char" )
  {
    return Core::DataType::CHAR_E;
  }
  else if( type == "unsigned_short" )
  {
    return Core::DataType::USHORT_E;
  }
  else if( type == "short" )
  {
    return Core::DataType::SHORT_E;
  }
  else if( type == "unsigned_int" )
  {
    return Core::DataType::UINT_E;
  }
  else if( type == "int" )
  {
    return Core::DataType::INT_E;
  }
  else if( type == "float" )
  {
    return Core::DataType::FLOAT_E;
  }
  else if( type == "double" )
  {
    return Core::DataType::DOUBLE_E;
  }
  else
  {
    // NOTE: Defaults to float if not known
    return Core::DataType::FLOAT_E;
  }
}

template< class ItkImporterType >
bool ITKLayerImporterPrivate::scan_simple_volume()
{
  // If header was already read, just return
  if ( this->read_header_ ) return true;
  
  typedef itk::ImageFileReader< itk::Image< unsigned char, 3 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  // We explicitly spell out the importer to use
  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  // Setup the importer
  reader->SetImageIO( IO );
  reader->SetFileName( this->importer_->get_filename() );

  try
  {
    reader->Update();
  }
  catch ( itk::ExceptionObject &err )
  {
    this->importer_->set_error( err.GetDescription() );
    return false;
  }
  catch( ... )
  {
    this->importer_->set_error( "ITK crashed while reading file." );
    return false;
  }

  // Grab the information on the data type from the ITK image
  std::string type_string = IO->GetComponentTypeAsString( IO->GetComponentType() );
  
  // Grab the image from the output so we can read its transform
  Core::ITKUCharImageDataHandle image_data;
  try
  {
    image_data =  Core::ITKUCharImageDataHandle( 
        new typename Core::ITKUCharImageData( reader->GetOutput() ) );  
  }
  catch ( ... )
  {
    this->importer_->set_error( "Importer could not unwrap itk object." );
    return false;
  }

  // Store the information we just extracted from the file in this private class
  this->data_type_ = this->convert_data_type( type_string );
  this->grid_transform_ = image_data->get_grid_transform();
  
  // Header was read, hence mark it so we will not read it again.
  this->read_header_ = true;
  return true;
}

bool ITKLayerImporterPrivate::read_header()
{
  // If importing already succeeded, don't do it again
  if ( this->read_header_ ) return true;

  // Get the extension to see which reader to use
  // NOTE: We spell them out so we can read the header data returned by the IO class.
  boost::filesystem::path full_filename( this->importer_->get_filename() );
  std::string extension = Core::GetFullExtension( full_filename );

  // Set file type and scan the file for data type and transform
  if ( detect_tiff(extension) )
  {
    this->file_type_ = "tiff";
    return this->scan_simple_volume< itk::TIFFImageIO >();
  }
  else if ( detect_vtk( extension ) )
  {
    this->file_type_ = "VTK";
    return this->scan_simple_volume< itk::VTKImageIO >();
  }
  else if ( detect_lsm( extension ) ) 
  {
    this->file_type_ = "LSM";
    return this->scan_simple_volume< itk::LSMImageIO >();
  }
  else if ( detect_analyze( extension ) ) 
  {
    this->file_type_ = "Analyze";
    return this->scan_simple_volume< itk::AnalyzeImageIO >();
  } 
  else if ( detect_nifti( extension ) ) 
  {
    this->file_type_ = "NIfTY";
    return this->scan_simple_volume< itk::NiftiImageIO >();
  } 
  else if ( detect_metaimage( extension ) )
  {
    this->file_type_ = "MetaIO";
    return this->scan_simple_volume< itk::MetaImageIO >();
  }   
  this->importer_->set_error( "Unknown file format." );
  return false; 
}

template< class DataType, class ItkImporterType >
bool ITKLayerImporterPrivate::import_simple_typed_volume()
{
  // Importer for a specific data type
  // Setup the reader to read the right type.
  // NOTE: Although itk supports reading data in any format, this functionality is broken
  // as it will do a simple cast from original data to image. The problem is that precision
  // and data that is out of range is lost. Hence the only format that is reasonable is float
  // as it is hard to go out of range and most image data has no more than 16 bits of gray
  // levels. However converting everything to float is inefficient. Hence we prefer actual
  // datatypes if we can obtain them. Hence this class is templated with the right type in
  // mind.
  typedef itk::ImageFileReader< itk::Image< DataType, 3 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  // Setup specific IO
  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  // Setup file name and IO
  reader->SetImageIO( IO );
  reader->SetFileName( this->importer_->get_filename() );

  try
  {
    reader->Update();
  }
  catch ( itk::ExceptionObject &err )
  {
    this->importer_->set_error( err.GetDescription() );
    return false;
  }
  catch( ... )
  {
    this->importer_->set_error( "ITK crashed while reading file." );
    return false;
  }

  // Wrap a class around ITK object that makes it easier to extract data from ITK object
  typename Core::ITKImageDataT< DataType >::Handle image_data;
  try
  {
     image_data = typename Core::ITKImageDataT< DataType >::Handle( 
        new typename Core::ITKImageDataT< DataType >( reader->GetOutput() ) );  
  }
  catch ( ... )
  {
    this->importer_->set_error( "Importer could not unwrap itk object." );
    return false;
  }

  // Get grid transform and data block from the wrapped itk object
  this->data_block_ = Core::ITKDataBlock::New( image_data );
  this->grid_transform_ = image_data->get_grid_transform();

  if( this->data_block_ )
  {
    // Mask as done
    this->read_data_ = true;
    return true;
  }

  this->importer_->set_error( "Failed to read data from itk object." );
  return false;
}

template< class ItkImporterType >
bool ITKLayerImporterPrivate::import_simple_volume()
{ 
  // For each data type instantiate the right reader
  switch( this->data_type_ )
  {
    case Core::DataType::UCHAR_E:
      return this->import_simple_typed_volume< unsigned char, ItkImporterType >();
    case Core::DataType::CHAR_E:
      return this->import_simple_typed_volume< signed char, ItkImporterType >();
    case Core::DataType::USHORT_E:
      return this->import_simple_typed_volume< unsigned short, ItkImporterType >();
    case Core::DataType::SHORT_E:
      return this->import_simple_typed_volume< signed short, ItkImporterType >();
    case Core::DataType::UINT_E:
      return this->import_simple_typed_volume< unsigned int, ItkImporterType >();
    case Core::DataType::INT_E:
      return this->import_simple_typed_volume< int, ItkImporterType >();
    case Core::DataType::FLOAT_E:
      return this->import_simple_typed_volume< float, ItkImporterType >();
    case Core::DataType::DOUBLE_E:
      return this->import_simple_typed_volume< double, ItkImporterType >();
    default:
      return false;   
  }
}

bool ITKLayerImporterPrivate::read_data()
{
  // Check whether data was already read
  if ( this->read_data_ ) return true;

  if ( ! this->read_header_ )
  {
    if ( ! this->read_header() ) return false;
  }
  
  // Get the extension of the file
  boost::filesystem::path full_filename( this->importer_->get_filename() );
  std::string extension = Core::GetFullExtension( full_filename );

  if ( detect_tiff(extension) )
  {
    return this->import_simple_volume<itk::TIFFImageIO>();
  }
  else if ( detect_vtk( extension ) )
  {
    return this->import_simple_volume<itk::VTKImageIO>();
  } 
  else if ( detect_lsm( extension ) ) 
  {
    return this->import_simple_volume<itk::LSMImageIO>();
  } 
  else if ( detect_analyze( extension ) ) 
  {
    return this->import_simple_volume<itk::AnalyzeImageIO>();
  } 
  else if ( detect_nifti( extension ) ) 
  {
    return this->import_simple_volume<itk::NiftiImageIO>();
  } 
  else if ( detect_metaimage( extension ) )
  {
    return this->import_simple_volume<itk::MetaImageIO>();    
  }

  // In case no file extension matched
  this->importer_->set_error( "Unknown file format." );
  return false; 
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

ITKLayerImporter::ITKLayerImporter() :
  private_( new ITKLayerImporterPrivate )
{
  this->private_->importer_ = this;
}

ITKLayerImporter::~ITKLayerImporter()
{
}

bool ITKLayerImporter::get_file_info( LayerImporterFileInfoHandle& info )
{
  try
  { 
    // Try to read the header
    if ( ! this->private_->read_header() ) return false;
  
    // Generate an information structure with the information.
    info = LayerImporterFileInfoHandle( new LayerImporterFileInfo );
    info->set_data_type( this->private_->data_type_ );
    info->set_grid_transform( this->private_->grid_transform_ );
    info->set_file_type( this->private_->file_type_ ); 
    info->set_mask_compatible( true );
  }
  catch ( ... )
  {
    // In case something failed, recover from here and let the user
    // deal with the error. 
    this->set_error( "ITK Importer crashed while reading file." );
    return false;
  }
    
  return true;
}

bool ITKLayerImporter::get_file_data( LayerImporterFileDataHandle& data )
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
    this->set_error( "ITK Importer crashed when reading file." );
    return false;
  }

  return true;
}

bool CopyITKFile( const boost::filesystem::path& src, 
  const boost::filesystem::path& dst )
{
  boost::filesystem::path full_filename = src;
  std::string extension = boost::to_lower_copy( boost::filesystem::extension( full_filename ) );

  if ( extension != ".mhd" )
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

  // Copy and update header file for ITK's MetaIO format  
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

      if ( line.size() > 17 && line.substr( 0, 17 ) == "ElementDataFile" )
      {
        size_t index;
        while ( index < line.size() && line[ index ] != '=' ) index++;
        
        std::string contents = line.substr( index + 1 );
        std::vector<std::string> components;
        if ( Core::ImportFromString( contents, components ) )
        {
          CORE_LOG_ERROR( std::string( "Cannot interpret the data file in header of mhd file '" )
            + src.string() + "'." );
          return false;
        }

        // Check the three options
        if ( components.size() == 1 )
        {
          if ( components[ 0 ] == "LIST" )
          {
            // MetaIO format
            new_file_header << "ElementDataFile = LIST" << std::endl;
            
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
            new_file_header << "ElementDataFile = " << filename.filename() << std::endl;
            data_files.push_back( filename );
          }
        }
        else if ( components.size() == 4 )
        {
          std::vector<char> buffer( components[ 0 ].size() + 20, '\0' );
          int start, stop, step;
          if ( !Core::ImportFromString( components[ 1 ], start ) ||
            !Core::ImportFromString( components[ 2 ], stop ) ||
            !Core::ImportFromString( components[ 3 ], step ) )
          {
            CORE_LOG_ERROR( std::string( "Could not interpret 'ElementDataFile' field in header of file '" ) +
              src.string() + "'." );
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
          CORE_LOG_ERROR( std::string( "Invalid entry in 'ElementDataFile' field in the header of file'" ) +
            src.string() + "'." );
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
      new_file_header << "ElementDataFile = LIST" << std::endl;
      for ( size_t j = 0; j < data_files.size(); j++ )
      {
        new_file_header << data_files[ j ].filename() << std::endl;
      }
    }
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( std::string( "Could not copy and update header file '" ) +
      src.string() + "'." );
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



InputFilesImporterHandle ITKLayerImporter::get_inputfiles_importer()
{
  InputFilesImporterHandle importer( new InputFilesImporter( this->get_inputfiles_id() ) );
  try
  {
    boost::filesystem::path full_filename( this->get_filename() );
    importer->add_filename( full_filename );
    importer->set_copy_file_function( &CopyITKFile );
  }
  catch ( ... )
  {
    this->set_error( std::string( "Could not resolve filename '" ) + 
      this->get_filename() + "'." );
  }
  
  return importer;
}
   
} // end namespace seg3D
