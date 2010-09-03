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

// Application includes
#include <Application/Layer/DataLayer.h> 
#include <Application/LayerManager/LayerManager.h>



SCI_REGISTER_IMPORTER( Seg3D, ITKLayerImporter );


namespace Seg3D
{

ITKLayerImporter::ITKLayerImporter( const std::string& filename ) :
  LayerImporter( filename )
{
}

bool ITKLayerImporter::import_header()
{
  if( this->extension_ == ".dcm" )
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
    }
    catch( gdcm::Exception &error )
    {
      std::string format_error = error.getError();
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
      return false;
    }
  }
  return true;  
}

bool ITKLayerImporter::import_layer( LayerImporterMode mode, std::vector<LayerHandle>& layers )
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
    return false;
  }
  else if( this->extension_ == ".tiff" )
  {
    return false;
  }

  if( !this->image_data_ ) return false;

  Core::DataVolumeHandle data_volume( 
    new Core::DataVolume( this->image_data_->get_grid_transform(), this->data_block_ ) );
  data_volume->get_data_block()->update_histogram();

  layers.resize( 1 );
  layers[ 0 ] = LayerHandle( new DataLayer( boost::filesystem::path( 
    this->get_filename() ).parent_path().filename(),
    data_volume ) );

  // If the layer wasn't successfully created, we exit.
  if( !layers[ 0 ] )
  {
    return false;
  }

  CORE_LOG_DEBUG( std::string("Successfully imported: ") + get_base_filename() );
  return true;
}

bool ITKLayerImporter::import_png_series()
{
  return false;
}

Core::GridTransform ITKLayerImporter::get_grid_transform()
{
  if ( this->image_data_ )  return this->image_data_->get_grid_transform();
  else 
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

} // end namespace seg3D
