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
#include "itkTIFFImageIO.h"
#include "itkVTKImageIO.h"
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
#include <Application/LayerIO/ITKSeriesLayerImporter.h>
#include <Application/Layer/DataLayer.h> 
#include <Application/LayerManager/LayerManager.h>

SCI_REGISTER_IMPORTER( Seg3D, ITKSeriesLayerImporter );

namespace Seg3D
{

class ITKSeriesLayerImporterPrivate
{

public:
  ITKSeriesLayerImporterPrivate() :
    data_type_( Core::DataType::UCHAR_E ),
    read_header_( false )
  {
  }

public: 
  // SET_DATA_TYPE:
  // Copy the data type we get from itk and convert to a Seg3D enum type
  bool set_data_type( std::string& type );

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
  
  // SCAN_DICOM:
  // Scan the data file
  bool scan_dicom();
  
  // IMPORT_DICOM_TYPED_SERIES:
  // Read the data in its final format
  template< class DataType >
  bool import_dicom_typed_series();

  // IMPORT_DICOM_SERIES:
  // Import the series in its final format by choosing the right format
  bool import_dicom_series(); 

public:
  // The type of the file we are importing
  std::string extension_;

  // The data type of the data we are importing
  Core::DataType data_type_;

  // The file name of the file we are reading
  std::vector< std::string > file_list_;

  Core::ITKImageDataHandle image_data_;
  Core::DataBlockHandle data_block_;
  
  bool read_header_;
};

bool ITKSeriesLayerImporterPrivate::set_data_type( std::string& type )
{
  if( type == "unsigned_char" )
  {
    this->data_type_ = Core::DataType::UCHAR_E;
    return true;
  }
  else if( type == "char" )
  {
    this->data_type_ = Core::DataType::CHAR_E;
    return true;
  }
  else if( type == "unsigned_short" )
  {
    this->data_type_ = Core::DataType::USHORT_E;
    return true;
  }
  else if( type == "short" )
  {
    this->data_type_ = Core::DataType::SHORT_E;
    return true;
  }
  else if( type == "unsigned_int" )
  {
    this->data_type_ = Core::DataType::UINT_E;
    return true;
  }
  else if( type == "int" )
  {
    this->data_type_ = Core::DataType::INT_E;
    return true;
  }
  else if( type == "float" )
  {
    this->data_type_ = Core::DataType::FLOAT_E;
    return true;
  }
  else if( type == "double" )
  {
    this->data_type_ = Core::DataType::DOUBLE_E;
    return true;
  }
  else if( type == "unknown" )
  {
    this->data_type_ = Core::DataType::FLOAT_E;
    return true;
  }
  else
  {
    return false;
  }
}


bool ITKSeriesLayerImporterPrivate::scan_dicom()
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
    int bits = boost::lexical_cast< int >( bits_allocated_entry->GetMetaDataObjectValue() );
    int bits_stored = boost::lexical_cast< int >( bits_stored_entry->GetMetaDataObjectValue() );
    int high_bit = boost::lexical_cast< int >( high_bit_entry->GetMetaDataObjectValue() );

        bool signed_data = false; 

    if( bits_stored > high_bit )
    {
      signed_data = true;
    }
    else
    {
      signed_data = false;
    }

    if( ( bits == 8 ) && ( signed_data == false ) )
    {
      this->data_type_ = Core::DataType::UCHAR_E;
    }
    if( ( bits == 8 ) && ( signed_data == true ) )
    {
      this->data_type_ = Core::DataType::CHAR_E;
    }
    if( ( bits == 16 ) && ( signed_data == false ) )
    {
      this->data_type_ = Core::DataType::USHORT_E;
    }
    if( ( bits == 16 ) && ( signed_data == true ) )
    {
      this->data_type_ = Core::DataType::SHORT_E;
    }
    if( ( bits == 32 ) && ( signed_data == false ) )
    {
      this->data_type_ = Core::DataType::UINT_E;
    }
    if( ( bits == 32 ) && ( signed_data == true ) )
    {
      this->data_type_ = Core::DataType::INT_E;
    }
  }
  else
  {
    this->data_type_ = Core::DataType::FLOAT_E;
  }
  
  this->read_header_ = true;
  return true;
}


template< class DataType >
bool ITKSeriesLayerImporterPrivate::import_dicom_typed_series()
{
  // Step 1: setup the image type
  typedef itk::Image< DataType, 3 > ImageType;

  // Step 2: using the image type, create a ITK reader
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  // Step 3: now because we are importing dicoms we create a GDCM IO object
  typedef itk::GDCMImageIO ImageIOType;
  ImageIOType::Pointer dicom_io = ImageIOType::New();

  // Step 4: now we set the io and the file list in the reader
  reader->SetImageIO( dicom_io );
  reader->SetFileNames( this->file_list_ );

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
  this->data_block_ = Core::ITKDataBlock::New< DataType >( 
    typename itk::Image< DataType, 3 >::Pointer( reader->GetOutput() ) );

  // Step 6: here we instantiate a new ITKImageData using the output from the reader
  this->image_data_ = typename Core::ITKImageDataT< DataType >::Handle( 
    new typename Core::ITKImageDataT< DataType >( reader->GetOutput() ) );

  // Step 7: now we check to see if we were successful creating our datablock and image
  // if we were then we set the grid transform.
  if( this->image_data_ && this->data_block_ )
  {
    return true;
  }
  else
  {
    // otherwise we return false
    return false;
  }
}

bool ITKSeriesLayerImporterPrivate::import_dicom_series()
{
  switch( this->data_type_ )
  {
    case Core::DataType::UCHAR_E:
      return this->import_dicom_typed_series< unsigned char >();
    case Core::DataType::CHAR_E:
      return this->import_dicom_typed_series< signed char >();
    case Core::DataType::USHORT_E:
      return this->import_dicom_typed_series< unsigned short >();
    case Core::DataType::SHORT_E:
      return this->import_dicom_typed_series< signed short >();
    case Core::DataType::UINT_E:
      return this->import_dicom_typed_series< unsigned int >();
    case Core::DataType::INT_E:
      return this->import_dicom_typed_series< int >();
    case Core::DataType::FLOAT_E:
      return this->import_dicom_typed_series< float >();
    case Core::DataType::DOUBLE_E:
      return this->import_dicom_typed_series< double >();
    default:
      return false;   
  }
}

template< class ItkImporterType >
bool ITKSeriesLayerImporterPrivate::scan_simple_series()
{
  typedef itk::ImageFileReader< itk::Image< unsigned short, 2 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileName( this->file_list_[ 0 ] );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    return false;
  }

  std::string type_string = IO->GetComponentTypeAsString( IO->GetComponentType() );
  if ( ! this->set_data_type( type_string ) ) return false;
  
  this->read_header_ = true;
  return true;
}


template< class DataType, class ItkImporterType >
bool ITKSeriesLayerImporterPrivate::import_simple_typed_series()
{
  typedef itk::Image< DataType, 3 > ImageType;
  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileNames( this->file_list_ );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    return false;
  }

  this->data_block_ = Core::ITKDataBlock::New< DataType >( 
    typename itk::Image< DataType, 3 >::Pointer( reader->GetOutput() ) );

  this->image_data_ = typename Core::ITKImageDataT< DataType >::Handle( 
    new typename Core::ITKImageDataT< DataType >( reader->GetOutput() ) );

  if( this->image_data_ && this->data_block_ )
  {
    return true;
  }
  else
  {
    return false;
  }
}

template< class ItkImporterType >
bool ITKSeriesLayerImporterPrivate::import_simple_series()
{
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

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


ITKSeriesLayerImporter::ITKSeriesLayerImporter( const std::string& filename ) :
  LayerImporter( filename ),
  private_( new ITKSeriesLayerImporterPrivate )
{
}

bool ITKSeriesLayerImporter::set_file_list( const std::vector< std::string >& file_list )
{
  if ( file_list.size() )
  {
    std::string extension = boost::filesystem::path( file_list[ 0 ] ).extension();;
    boost::to_lower( extension );

    this->private_->file_list_ = file_list;
    this->private_->extension_ = extension;
    return true;
  }
  else
  {
    return false;
  }
}

bool ITKSeriesLayerImporter::import_header()
{
  if ( this->private_->read_header_ ) return true;

  if ( this->private_->extension_ == ".dcm" || 
    this->private_->extension_ == ".dicom" ||
    this->private_->extension_ == "" )
  {
    return this->private_->scan_dicom();
  }
  else if( this->private_->extension_ == ".png" )
  {
    return this->private_->scan_simple_series< itk::PNGImageIO >();
  }
  else if( this->private_->extension_ == ".tif" ||
    this->private_->extension_ == ".tiff" )
  {
    return this->private_->scan_simple_series< itk::TIFFImageIO >();
  }
  else if( this->private_->extension_ == ".jpg" ||
    this->private_->extension_ == ".jpeg" )
  {
    return this->private_->scan_simple_series< itk::JPEGImageIO >();
  }
  else if( this->private_->extension_ == ".bmp" )
  {
    return this->private_->scan_simple_series< itk::BMPImageIO >();
  }
  else if( this->private_->extension_ == ".vtk" )
  {
    return this->private_->scan_simple_series< itk::VTKImageIO >();
  }
  
  return false; 
}


Core::GridTransform ITKSeriesLayerImporter::get_grid_transform()
{
  if( this->private_->image_data_ ) return this->private_->image_data_->get_grid_transform();
  return Core::GridTransform( 1, 1, 1 );
}

Core::DataType ITKSeriesLayerImporter::get_data_type()
{
  if( this->private_->image_data_ ) return this->private_->image_data_->get_data_type();
  return Core::DataType::UNKNOWN_E;
}

int ITKSeriesLayerImporter::get_importer_modes()
{
  return LayerImporterMode::DATA_E;
}

  
bool ITKSeriesLayerImporter::load_data( Core::DataBlockHandle& data_block, 
  Core::GridTransform& grid_trans )
{
  if ( this->private_->extension_ == ".dcm" || 
    this->private_->extension_ == ".dicom" ||
    this->private_->extension_ == "" )
  {
    this->private_->import_dicom_series();
  }
  else if( this->private_->extension_ == ".png" )
  {
    this->private_->import_simple_series< itk::PNGImageIO >();
  }
  else if( this->private_->extension_ == ".tif" ||
    this->private_->extension_ == ".tiff" )
  {
    this->private_->import_simple_series< itk::TIFFImageIO >();
  }
  else if( this->private_->extension_ == ".jpg" ||
    this->private_->extension_ == ".jpeg" )
  {
    this->private_->import_simple_series< itk::JPEGImageIO >();
  }
  else if( this->private_->extension_ == ".bmp" )
  {
    this->private_->import_simple_series< itk::BMPImageIO >();
  }
  else if( this->private_->extension_ == ".vtk" )
  {
    this->private_->import_simple_series< itk::VTKImageIO >();
  }
  
  if( ( !this->private_->data_block_ ) || ( !this->private_->image_data_ ) ) 
  {
    return false;
  }
  
  data_block = this->private_->data_block_;
  grid_trans = this->private_->image_data_->get_grid_transform();

  return true;
}

std::string ITKSeriesLayerImporter::get_layer_name()
{
  return boost::filesystem::path( this->get_filename() ).parent_path().filename();
}

} // end namespace seg3D
