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

// ITK Includes
#include "itkRGBPixel.h"
#include "itkPNGImageIO.h"
#include "itkBMPImageIO.h"
#include "itkJPEGImageIO.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "gdcmException.h"

#include <teem/nrrd.h>

// Boost Includes
#include <boost/lexical_cast.hpp>

// Core includes
#include <Core/DataBlock/ITKImageData.h>
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/LayerIO/ITKLayerImporter.h>
#include <Application/Layer/DataLayer.h> 
#include <Application/LayerManager/LayerManager.h>

SCI_REGISTER_IMPORTER( Seg3D, ITKLayerImporter );

namespace Seg3D
{


//////////////////////////////////////////////////////////////////////////////////////
/////////////////////// - templated files and their helpers - ////////////////////////  
bool set_pixel_type( std::string& type, Core::DataType& pixel_type )
{
  if( type == "unsigned_char" )
  {
    pixel_type = Core::DataType::UCHAR_E;
    return true;
  }
  else if( type == "char" )
  {
    pixel_type = Core::DataType::CHAR_E;
    return true;
  }
  else if( type == "unsigned_short" )
  {
    pixel_type = Core::DataType::USHORT_E;
    return true;
  }
  else if( type == "short" )
  {
    pixel_type = Core::DataType::SHORT_E;
    return true;
  }
  else if( type == "unsigned_int" )
  {
    pixel_type = Core::DataType::UINT_E;
    return true;
  }
  else if( type == "int" )
  {
    pixel_type = Core::DataType::INT_E;
    return true;
  }
  else if( type == "float" )
  {
    pixel_type = Core::DataType::FLOAT_E;
    return true;
  }
  else if( type == "double" )
  {
    pixel_type = Core::DataType::DOUBLE_E;
    return true;
  }
  else if( type == "unknown" )
  {
    pixel_type = Core::DataType::FLOAT_E;
    return true;
  }
  else
  {
    return false;
  }
}

template< class PixelType >
bool import_dicom_series( Core::ITKImageDataHandle& image_data, 
  Core::DataBlockHandle& data_block, std::vector< std::string >& file_list )
{
  // Step 1: setup the image type
  const unsigned int dimension = 3;
  typedef itk::Image< PixelType, dimension > ImageType;

  // Step 2: using the image type, create a ITK reader
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  // Step 3: now because we are importing dicoms we create a GDCM IO object
  typedef itk::GDCMImageIO ImageIOType;
  ImageIOType::Pointer dicom_io = ImageIOType::New();

  // Step 4: now we set the io and the file list in the reader
  reader->SetImageIO( dicom_io );
  reader->SetFileNames( file_list );

  // Step 5: now we attempt to actually read in the file and catch potential errors
  try
  {
    reader->Update();
  }
  catch( ... )
  {
    return false;
  }

  // Step 6: here we instantiate a new DataBlock using the output from the reader
  data_block = Core::ITKDataBlock::New< PixelType >( 
    typename itk::Image< PixelType, 3 >::Pointer( reader->GetOutput() ) );

  // Step 6: here we instantiate a new ITKImageData using the output from the reader
  image_data = typename Core::ITKImageDataT< PixelType >::Handle( 
    new typename Core::ITKImageDataT< PixelType >( reader->GetOutput() ) );

  // Step 7: now we check to see if we were successful creating our datablock and image
  // if we were then we set the grid transform.
  if( image_data && data_block )
  {
    return true;
  }
  else
  {
    // otherwise we return false
    return false;
  }
}


template< class ItkImporterType >
bool scan_simple_series( const std::string& filename, Core::DataType& pixel_type )
{
  typedef itk::ImageFileReader< itk::Image< unsigned short, 2 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileName( filename );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    return false;
  }

  std::string type_string = IO->GetComponentTypeAsString( IO->GetComponentType() );
  if( type_string == "" ) return false;
  
  return set_pixel_type( type_string, pixel_type );
}

// templated function for scanning any itk supported type 
template< class ItkImporterType >
bool scan_simple_volume( const std::string& filename, Core::DataType& pixel_type )
{
  typedef itk::ImageFileReader< itk::Image< float, 3 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileName( filename );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    //return false;
  }

  std::string type_string = IO->GetComponentTypeAsString( IO->GetComponentType() );
  if( type_string == "" ) return false;

  return set_pixel_type( type_string, pixel_type );
}

template< class PixelType, class ItkImporterType >
bool import_simple_volume( Core::ITKImageDataHandle& image_data, 
  Core::DataBlockHandle& data_block, const std::string& file_name )
{
  typedef itk::ImageFileReader< itk::Image< PixelType, 3 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileName( file_name );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    return false;
  }

  data_block = Core::ITKDataBlock::New< PixelType >( 
    typename itk::Image< PixelType, 3 >::Pointer( reader->GetOutput() ) );

  image_data = typename Core::ITKImageDataT< PixelType >::Handle( 
    new typename Core::ITKImageDataT< PixelType >( reader->GetOutput() ) );

  if( image_data && data_block )
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Templated function for importing any simple itk importer type
template< class PixelType, class ItkImporterType >
bool import_simple_series( Core::ITKImageDataHandle& image_data, 
  Core::DataBlockHandle& data_block, std::vector< std::string >& file_list )
{
  const unsigned int dimension = 3;
  typedef itk::Image< PixelType, dimension > ImageType;

  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileNames( file_list );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    return false;
  }

  data_block = Core::ITKDataBlock::New< PixelType >( 
    typename itk::Image< PixelType, 3 >::Pointer( reader->GetOutput() ) );

  image_data = typename Core::ITKImageDataT< PixelType >::Handle( 
    new typename Core::ITKImageDataT< PixelType >( reader->GetOutput() ) );

  if( image_data && data_block )
  {
    return true;
  }
  else
  {
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


ITKLayerImporter::ITKLayerImporter( const std::string& filename ) :
  LayerImporter( filename ),
  pixel_type_( Core::DataType::UCHAR_E )
{
}

bool ITKLayerImporter::import_header()
{
  if( this->is_dicom() )
  {
    return this->scan_dicom();
  }
  else if( this->is_png() )
  {
    return scan_simple_series< itk::PNGImageIO >( this->get_filename(), this->pixel_type_ );
  }
  else if( this->is_tiff() )
  {
    return scan_simple_series< itk::TIFFImageIO >( this->get_filename(), this->pixel_type_ );
  }
  else if( this->is_jpeg() )
  {
    return scan_simple_series< itk::JPEGImageIO >( this->get_filename(), this->pixel_type_ );
  }
  else if( this->is_bmp() )
  {
    return scan_simple_series< itk::BMPImageIO >( this->get_filename(), this->pixel_type_ );
  }
  else if( this->is_vtk() )
  {
    return scan_simple_volume< itk::VTKImageIO >( this->get_filename(), this->pixel_type_ );
  }
  
  return false; 
}

Core::GridTransform ITKLayerImporter::get_grid_transform()
{
  if( this->image_data_ ) return this->image_data_->get_grid_transform();
  return Core::GridTransform( 1, 1, 1 );
}

Core::DataType ITKLayerImporter::get_data_type()
{
  if( this->image_data_ ) return this->image_data_->get_data_type();
  return Core::DataType::UNKNOWN_E;
}

int ITKLayerImporter::get_importer_modes()
{
  return LayerImporterMode::DATA_E;
}

bool ITKLayerImporter::scan_dicom()
{
  typedef itk::ImageFileReader< itk::Image< signed short, 2 > > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  typedef itk::GDCMImageIO ImageIOType;
  ImageIOType::Pointer dicomIO = ImageIOType::New();

  reader->SetImageIO( dicomIO );
  reader->SetFileName( this->file_list_[ 0 ] );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    return false;
  }

  typedef itk::MetaDataDictionary DictionaryType;
  typedef itk::MetaDataObject< std::string > MetaDataStringType;
  const DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();

  MetaDataStringType::Pointer bits_allocated_entry =
    dynamic_cast<MetaDataStringType *>( dictionary.Find( "0028|0100")->second.GetPointer() ) ;

  MetaDataStringType::Pointer bits_stored_entry =
    dynamic_cast<MetaDataStringType *>( dictionary.Find( "0028|0101")->second.GetPointer() ) ;

  MetaDataStringType::Pointer high_bit_entry =
    dynamic_cast<MetaDataStringType *>( dictionary.Find( "0028|0102")->second.GetPointer() ) ;

  if( bits_allocated_entry && bits_stored_entry && high_bit_entry )
  {
    this->bits_ = boost::lexical_cast< int >( bits_allocated_entry->GetMetaDataObjectValue() );
    int bits_stored = boost::lexical_cast< int >( bits_stored_entry->GetMetaDataObjectValue() );
    int high_bit = boost::lexical_cast< int >( high_bit_entry->GetMetaDataObjectValue() );

    if( bits_stored > high_bit )
    {
      this->signed_data_ = true;
    }
    else
    {
      this->signed_data_ = false;
    }

    if( ( this->bits_ == 8 ) && ( this->signed_data_ == false ) )
    {
      this->pixel_type_ = Core::DataType::UCHAR_E;
    }
    if( ( this->bits_ == 8 ) && ( this->signed_data_ == true ) )
    {
      this->pixel_type_ = Core::DataType::CHAR_E;
    }
    if( ( this->bits_ == 16 ) && ( this->signed_data_ == false ) )
    {
      this->pixel_type_ = Core::DataType::USHORT_E;
    }
    if( ( this->bits_ == 16 ) && ( this->signed_data_ == true ) )
    {
      this->pixel_type_ = Core::DataType::SHORT_E;
    }
    if( ( this->bits_ == 32 ) && ( this->signed_data_ == false ) )
    {
      this->pixel_type_ = Core::DataType::UINT_E;
    }
    if( ( this->bits_ == 32 ) && ( this->signed_data_ == true ) )
    {
      this->pixel_type_ = Core::DataType::INT_E;
    }
  }
  else
  {
    this->pixel_type_ = Core::DataType::FLOAT_E;
  }
  return true;
}
  
bool ITKLayerImporter::load_data( Core::DataBlockHandle& data_block, 
  Core::GridTransform& grid_trans )
{
  if( this->is_dicom() )
  { 
    switch( this->pixel_type_ )
    {
    case Core::DataType::UCHAR_E:
      if( !import_dicom_series< unsigned char >( this->image_data_, 
        this->data_block_, this->file_list_ ) ) return false;
      break;
    case Core::DataType::CHAR_E:
      if( !import_dicom_series< signed char >( this->image_data_, 
        this->data_block_, this->file_list_ ) ) return false;
      break;
    case Core::DataType::USHORT_E:
      if( !import_dicom_series< unsigned short >( this->image_data_, 
        this->data_block_, this->file_list_ ) ) return false;
      break;
    case Core::DataType::SHORT_E:
      if( !import_dicom_series< signed short >( this->image_data_, 
        this->data_block_, this->file_list_ ) ) return false;
      break;
    case Core::DataType::UINT_E:
      if( !import_dicom_series< signed int >( this->image_data_, 
        this->data_block_, this->file_list_ ) ) return false;
      break;
    case Core::DataType::INT_E:
      if( !import_dicom_series< unsigned int >( this->image_data_, 
        this->data_block_, this->file_list_ ) ) return false;
      break;
    default:
      break;
    }
  }
  else if( this->is_png() )
  {
    switch( this->pixel_type_ )
    {
      case Core::DataType::UCHAR_E:
        if( !import_simple_series< unsigned char, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !import_simple_series< signed char, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !import_simple_series< unsigned short, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !import_simple_series< signed short, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !import_simple_series< signed int, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::INT_E:
        if( !import_simple_series< unsigned int, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !import_simple_series< float, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !import_simple_series< double, itk::PNGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      default:
        break;    
    }
  }
  else if( this->is_tiff() )
  {
    switch( this->pixel_type_ )
    {
      case Core::DataType::UCHAR_E:
        if( !import_simple_series< unsigned char, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !import_simple_series< signed char, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !import_simple_series< unsigned short, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !import_simple_series< signed short, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !import_simple_series< signed int, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::INT_E:
        if( !import_simple_series< unsigned int, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !import_simple_series< float, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !import_simple_series< double, itk::TIFFImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      default:
        break;
    }
  }
  else if( this->is_jpeg() )
  { 
    switch( this->pixel_type_ )
    {
      case Core::DataType::UCHAR_E:
        if( !import_simple_series< unsigned char, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !import_simple_series< signed char, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !import_simple_series< unsigned short, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !import_simple_series< signed short, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !import_simple_series< signed int, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::INT_E:
        if( !import_simple_series< unsigned int, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !import_simple_series< float, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !import_simple_series< double, itk::JPEGImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      default:
        break;  
    }
  }
  else if( this->is_bmp() )
  {
    switch( this->pixel_type_ )
    {
      case Core::DataType::UCHAR_E:
        if( !import_simple_series< unsigned char, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !import_simple_series< signed char, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !import_simple_series< unsigned short, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !import_simple_series< signed short, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !import_simple_series< signed int, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::INT_E:
        if( !import_simple_series< unsigned int, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !import_simple_series< float, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !import_simple_series< double, itk::BMPImageIO >( this->image_data_, 
          this->data_block_, this->file_list_ ) ) return false;
        break;
      default:
        break;
    }
  }
  
  else if( this->is_vtk() )
  {
    switch( this->pixel_type_ )
    {
    case Core::DataType::UCHAR_E:
      if( !import_simple_volume< unsigned char, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    case Core::DataType::CHAR_E:
      if( !import_simple_volume< signed char, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    case Core::DataType::USHORT_E:
      if( !import_simple_volume< unsigned short, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    case Core::DataType::SHORT_E:
      if( !import_simple_volume< signed short, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    case Core::DataType::UINT_E:
      if( !import_simple_volume< signed int, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    case Core::DataType::INT_E:
      if( !import_simple_volume< unsigned int, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    case Core::DataType::FLOAT_E:
      if( !import_simple_volume< float, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    case Core::DataType::DOUBLE_E:
      if( !import_simple_volume< double, itk::VTKImageIO >( this->image_data_, 
        this->data_block_, this->get_filename() ) ) return false;
      break;
    default:
      break;
    }
  }
  else
  {
    return false;
  }

  if( ( !this->data_block_ ) || ( !this->image_data_ ) ) return false;

  data_block = this->data_block_;
  grid_trans = this->image_data_->get_grid_transform();

  return true;
}
  

std::string ITKLayerImporter::get_layer_name()
{
  return boost::filesystem::path( this->get_filename() ).parent_path().filename();
}




} // end namespace seg3D
