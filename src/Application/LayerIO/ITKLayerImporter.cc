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
    return this->scan_simple_series< itk::PNGImageIO >();
  }
  else if( this->is_tiff() )
  {
    return this->scan_simple_series< itk::TIFFImageIO >();
  }
  else if( this->is_jpeg() )
  {
    return this->scan_simple_series< itk::JPEGImageIO >();
  }
  else if( this->is_bmp() )
  {
    return this->scan_simple_series< itk::BMPImageIO >();
  }
  else if( this->is_vtk() )
  {
    return this->scan_simple_volume< itk::VTKImageIO >();
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
      if( !this->import_dicom_series< unsigned char >() ) return false;
      break;
    case Core::DataType::CHAR_E:
      if( !this->import_dicom_series< signed char >() ) return false;
      break;
    case Core::DataType::USHORT_E:
      if( !this->import_dicom_series< unsigned short >() ) return false;
      break;
    case Core::DataType::SHORT_E:
      if( !this->import_dicom_series< signed short >() ) return false;
      break;
    case Core::DataType::UINT_E:
      if( !this->import_dicom_series< signed int >() ) return false;
      break;
    case Core::DataType::INT_E:
      if( !this->import_dicom_series< unsigned int >() ) return false;
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
        if( !this->import_simple_series< unsigned char, itk::PNGImageIO >() ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !this->import_simple_series< signed char, itk::PNGImageIO >() ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !this->import_simple_series< unsigned short, itk::PNGImageIO >() ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !this->import_simple_series< signed short, itk::PNGImageIO >() ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !this->import_simple_series< signed int, itk::PNGImageIO >() ) return false;
        break;
      case Core::DataType::INT_E:
        if( !this->import_simple_series< unsigned int, itk::PNGImageIO >() ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !this->import_simple_series< float, itk::PNGImageIO >() ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !this->import_simple_series< double, itk::PNGImageIO >() ) return false;
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
        if( !this->import_simple_series< unsigned char, itk::TIFFImageIO >() ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !this->import_simple_series< signed char, itk::TIFFImageIO >() ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !this->import_simple_series< unsigned short, itk::TIFFImageIO >() ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !this->import_simple_series< signed short, itk::TIFFImageIO >() ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !this->import_simple_series< signed int, itk::TIFFImageIO >() ) return false;
        break;
      case Core::DataType::INT_E:
        if( !this->import_simple_series< unsigned int, itk::TIFFImageIO >() ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !this->import_simple_series< float, itk::TIFFImageIO >() ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !this->import_simple_series< double, itk::TIFFImageIO >() ) return false;
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
        if( !this->import_simple_series< unsigned char, itk::JPEGImageIO >() ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !this->import_simple_series< signed char, itk::JPEGImageIO >() ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !this->import_simple_series< unsigned short, itk::JPEGImageIO >() ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !this->import_simple_series< signed short, itk::JPEGImageIO >() ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !this->import_simple_series< signed int, itk::JPEGImageIO >() ) return false;
        break;
      case Core::DataType::INT_E:
        if( !this->import_simple_series< unsigned int, itk::JPEGImageIO >() ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !this->import_simple_series< float, itk::JPEGImageIO >() ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !this->import_simple_series< double, itk::JPEGImageIO >() ) return false;
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
        if( !this->import_simple_series< unsigned char, itk::BMPImageIO >() ) return false;
        break;
      case Core::DataType::CHAR_E:
        if( !this->import_simple_series< signed char, itk::BMPImageIO >() ) return false;
        break;
      case Core::DataType::USHORT_E:
        if( !this->import_simple_series< unsigned short, itk::BMPImageIO >() ) return false;
        break;
      case Core::DataType::SHORT_E:
        if( !this->import_simple_series< signed short, itk::BMPImageIO >() ) return false;
        break;
      case Core::DataType::UINT_E:
        if( !this->import_simple_series< signed int, itk::BMPImageIO >() ) return false;
        break;
      case Core::DataType::INT_E:
        if( !this->import_simple_series< unsigned int, itk::BMPImageIO >() ) return false;
        break;
      case Core::DataType::FLOAT_E:
        if( !this->import_simple_series< float, itk::BMPImageIO >() ) return false;
        break;
      case Core::DataType::DOUBLE_E:
        if( !this->import_simple_series< double, itk::BMPImageIO >() ) return false;
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
      if( !this->import_simple_volume< unsigned char, itk::VTKImageIO >() ) return false;
      break;
    case Core::DataType::CHAR_E:
      if( !this->import_simple_volume< signed char, itk::VTKImageIO >() ) return false;
      break;
    case Core::DataType::USHORT_E:
      if( !this->import_simple_volume< unsigned short, itk::VTKImageIO >() ) return false;
      break;
    case Core::DataType::SHORT_E:
      if( !this->import_simple_volume< signed short, itk::VTKImageIO >() ) return false;
      break;
    case Core::DataType::UINT_E:
      if( !this->import_simple_volume< signed int, itk::VTKImageIO >() ) return false;
      break;
    case Core::DataType::INT_E:
      if( !this->import_simple_volume< unsigned int, itk::VTKImageIO >() ) return false;
      break;
    case Core::DataType::FLOAT_E:
      if( !this->import_simple_volume< float, itk::VTKImageIO >() ) return false;
      break;
    case Core::DataType::DOUBLE_E:
      if( !this->import_simple_volume< double, itk::VTKImageIO >() ) return false;
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

bool ITKLayerImporter::set_pixel_type( std::string& type )
{
  if( type == "unsigned_char" )
  {
    this->pixel_type_ = Core::DataType::UCHAR_E;
    return true;
  }
  else if( type == "char" )
  {
    this->pixel_type_ = Core::DataType::CHAR_E;
    return true;
  }
  else if( type == "unsigned_short" )
  {
    this->pixel_type_ = Core::DataType::USHORT_E;
    return true;
  }
  else if( type == "short" )
  {
    this->pixel_type_ = Core::DataType::SHORT_E;
    return true;
  }
  else if( type == "unsigned_int" )
  {
    this->pixel_type_ = Core::DataType::UINT_E;
    return true;
  }
  else if( type == "int" )
  {
    this->pixel_type_ = Core::DataType::INT_E;
    return true;
  }
  else if( type == "float" )
  {
    this->pixel_type_ = Core::DataType::FLOAT_E;
    return true;
  }
  else if( type == "double" )
  {
    this->pixel_type_ = Core::DataType::DOUBLE_E;
    return true;
  }
  else if( type == "unknown" )
  {
    this->pixel_type_ = Core::DataType::FLOAT_E;
    return true;
  }
  else
  {
    return false;
  }
}


} // end namespace seg3D
