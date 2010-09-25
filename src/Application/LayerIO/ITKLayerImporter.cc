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
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "gdcmException.h"

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
  if( this->extension_ == ".dcm" )
  {
    return this->scan_dicom();
  }
  else if( this->extension_ == ".png" )
  {
    return this->scan_png();
  }
  else if( this->extension_ == ".tiff" )
  {
    return false;
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
  catch ( itk::ExceptionObject &err )
  {
    std::string itk_error = err.GetDescription();
    return false;
  }
  catch( gdcm::Exception &error )
  {
    std::string format_error = error.getError();
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

bool ITKLayerImporter::scan_png()
{
  return true;
}

bool ITKLayerImporter::scan_tiff()
{
  return false;
}

bool ITKLayerImporter::load_data( Core::DataBlockHandle& data_block, 
                 Core::GridTransform& grid_trans )
{
  if( this->extension_ == ".dcm" )
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
  else if( this->extension_ == ".png" )
  {
    if( !this->import_png_series< float >() ) return false; 
    return false;
  }
  else if( this->extension_ == ".tiff" )
  {
    return false;
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
