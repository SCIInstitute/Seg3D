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

// boost includes
#include <boost/algorithm/string/case_conv.hpp>

// ITK Includes
#include <itkRGBPixel.h>
#include <itkPNGImageIO.h>
#include <itkTIFFImageIO.h>
#include <itkVTKImageIO.h>
#include <itkBMPImageIO.h>
#include <itkJPEGImageIO.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageSeriesReader.h>
#include <gdcmException.h>

// Core includes
#include <Core/DataBlock/ITKImageData.h>
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/LayerIO/ITKSeriesLayerImporter.h>

SEG3D_REGISTER_IMPORTER( Seg3D, ITKSeriesLayerImporter );

namespace Seg3D
{

class ITKSeriesLayerImporterPrivate
{

public:
  ITKSeriesLayerImporterPrivate() :
    data_type_( Core::DataType::UCHAR_E ),
    read_header_( false ),
    read_data_( false )
  {
  }

  // Pointer back to the main class
  ITKSeriesLayerImporter* importer_;

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

  // SCAN_SIMPLE_SERIES:
  // Scan the data file
  template< class ItkImporterType >
  bool scan_simple_series();
  
  // IMPORT_SIMPLE_TYPED_SERIES:
  // Read the data in its final format
  template< class DataType, class ItkImporterType >
  bool import_simple_typed_series();

  // IMPORT_SIMPLE_SERIES:
  // Import the series in its final format by choosing the right format
  template< class ItkImporterType >
  bool import_simple_series();  

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
};


Core::DataType ITKSeriesLayerImporterPrivate::convert_data_type( std::string& type )
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

bool ITKSeriesLayerImporterPrivate::read_header()
{
  // If importing already succeeded, don't do it again
  if ( this->read_header_ ) return true;
  
  // Extract the extension from the file name and use this to define
  // which importer to use.
  boost::filesystem::path full_filename( this->importer_->get_filename() );
  std::string extension = boost::to_lower_copy( boost::filesystem::extension( full_filename ) );
  
  if( extension == ".png" )
  {
    this->file_type_ = "png";
    return this->scan_simple_series< itk::PNGImageIO >();
  }
  else if( extension == ".tif" || extension == ".tiff" )
  {
    this->file_type_ = "tiff";
    return this->scan_simple_series< itk::TIFFImageIO >();
  }
  else if( extension == ".jpg" || extension == ".jpeg" )
  {
    this->file_type_ = "jpeg";
    return this->scan_simple_series< itk::JPEGImageIO >();
  }
  else if( extension == ".bmp" )
  {
    this->file_type_ = "bitmap";
    return this->scan_simple_series< itk::BMPImageIO >();
  }
  else if( extension == ".vtk" )
  {
    this->file_type_ = "VTK";
    return this->scan_simple_series< itk::VTKImageIO >();
  }
  else 
  {
    // Assume it is DICOM
    return this->scan_simple_series< itk::GDCMImageIO >();
  }
}

template< class ItkImporterType >
bool ITKSeriesLayerImporterPrivate::scan_simple_series()
{
  // If header was already read, just return
  if ( this->read_header_ ) return true;

  // We read only one file to find out transform and data type
  typedef itk::ImageFileReader< itk::Image< unsigned char, 2 > > ReaderType;
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
  catch ( ... )
  {
    this->importer_->set_error( "ITK reader failed." );
    return false;
  }

  // Grab the information on the data type from the ITK image
  std::string type_string = IO->GetComponentTypeAsString( IO->GetComponentType() );
  
  // Grab the image from the output so we can read its transform
  Core::ITKUCharImage2DDataHandle image_data;
  try
  {
    image_data =  Core::ITKUCharImage2DDataHandle( 
        new typename Core::ITKUCharImage2DData( reader->GetOutput() ) );  
  }
  catch ( ... )
  {
    this->importer_->set_error( "Importer could not unwrap itk object." );
    return false;
  } 
  
  // Store the information we just extracted from the file in this private class
  this->data_type_ = this->convert_data_type( type_string );
  this->grid_transform_ = image_data->get_grid_transform();

  this->read_header_ = true;
  return true;
}

template< class DataType, class ItkImporterType >
bool ITKSeriesLayerImporterPrivate::import_simple_typed_series()
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
  
  typedef itk::Image< DataType, 3 > ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  // Setup specific IO
  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  // Setup file names and IO
  reader->SetImageIO( IO );
  reader->SetFileNames( this->importer_->get_filenames() );

  try
  {
    reader->Update();
  }
  catch ( itk::ExceptionObject &err )
  {
    this->importer_->set_error( err.GetDescription() );
    return false;
  }
  catch ( ... )
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
    this->read_data_ = true;
    return true;
  }

  this->importer_->set_error( "Failed to read data from itk object." );
  return false;
}

template< class ItkImporterType >
bool ITKSeriesLayerImporterPrivate::import_simple_series()
{
  // For each data type instantiate the right reader
  switch( this->data_type_ )
  {
    case Core::DataType::UCHAR_E:
      return this->import_simple_typed_series< unsigned char, ItkImporterType >();
    case Core::DataType::CHAR_E:
      return this->import_simple_typed_series< signed char, ItkImporterType >();
    case Core::DataType::USHORT_E:
      return this->import_simple_typed_series< unsigned short, ItkImporterType >();
    case Core::DataType::SHORT_E:
      return this->import_simple_typed_series< signed short, ItkImporterType >();
    case Core::DataType::UINT_E:
      return this->import_simple_typed_series< unsigned int, ItkImporterType >();
    case Core::DataType::INT_E:
      return this->import_simple_typed_series< int, ItkImporterType >();
    case Core::DataType::FLOAT_E:
      return this->import_simple_typed_series< float, ItkImporterType >();
    case Core::DataType::DOUBLE_E:
      return this->import_simple_typed_series< double, ItkImporterType >();
    default:
      return false;   
  }
}

bool ITKSeriesLayerImporterPrivate::read_data()
{
  // If importing already succeeded, don't do it again
  if ( this->read_data_ ) return true;
  
  // Extract the extension from the file name and use this to define
  // which importer to use.
  boost::filesystem::path full_filename( this->importer_->get_filename() );
  std::string extension = boost::to_lower_copy( boost::filesystem::extension( full_filename ) );
  
  if( extension == ".png" )
  {
    return this->import_simple_series< itk::PNGImageIO >();
  }
  else if( extension == ".tif" || extension == ".tiff" )
  {
    return this->import_simple_series< itk::TIFFImageIO >();
  }
  else if( extension == ".jpg" || extension == ".jpeg" )
  {
    return this->import_simple_series< itk::JPEGImageIO >();
  }
  else if( extension == ".bmp" )
  {
    return this->import_simple_series< itk::BMPImageIO >();
  }
  else if( extension == ".vtk" )
  {
    return this->import_simple_series< itk::VTKImageIO >();
  }
  else // assume it is dicom 
  {
    return this->import_simple_series< itk::GDCMImageIO >();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

ITKSeriesLayerImporter::ITKSeriesLayerImporter() :
  private_( new ITKSeriesLayerImporterPrivate )
{
  this->private_->importer_ = this;
}

ITKSeriesLayerImporter::~ITKSeriesLayerImporter() 
{
}

bool ITKSeriesLayerImporter::get_file_info( LayerImporterFileInfoHandle& info )
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
    this->set_error( "ITK Series Importer crashed while reading file." );
    return false;
  }
    
  return true;
}

bool ITKSeriesLayerImporter::get_file_data( LayerImporterFileDataHandle& data )
{
  try
  { 
    // Read the data from the file
    if ( ! this->private_->read_data() ) return false;
  
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
    this->set_error( "ITK Series Importer crashed when reading file." );
    return false;
  }

  return true;
}

} // end namespace seg3D
