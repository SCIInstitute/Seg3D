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

// ITK includes
#include "itkRGBPixel.h"
#include "itkTIFFImageIO.h"
#include "itkVTKImageIO.h"
#include "itkLSMImageIO.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkAnalyzeImageIO.h"
#include "itkNiftiImageIO.h"
#include "itkMetaImageIO.h"
#include "itkImageSeriesReader.h"
#include "gdcmException.h"

// Teem includes
#include <teem/nrrd.h>

// Boost includes
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

class ITKLayerImporterPrivate
{

public:
  ITKLayerImporterPrivate() :
    data_type_( Core::DataType::UCHAR_E ),
    read_header_( false )
  {
  }

public: 
  // SET_DATA_TYPE:
  // Copy the data type we get from itk and convert to a Seg3D enum type
  bool set_data_type( std::string& type );

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
  // The type of the file we are importing
  std::string extension_;

  // The data type of the data we are importing
  Core::DataType data_type_;

  // The file name of the file we are reading
  std::string file_name_;

  // The data that was read from the file
  Core::ITKImageDataHandle image_data_;
  Core::DataBlockHandle data_block_;
  
  // Whether the header was read
  bool read_header_;
  
};

bool ITKLayerImporterPrivate::set_data_type( std::string& type )
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

template< class ItkImporterType >
bool ITKLayerImporterPrivate::scan_simple_volume()
{
  typedef itk::ImageFileReader< itk::Image< float, 3 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileName( this->file_name_ );

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
bool ITKLayerImporterPrivate::import_simple_typed_volume()
{
  typedef itk::ImageFileReader< itk::Image< DataType, 3 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  typedef ItkImporterType ImageIOType;
  typename ImageIOType::Pointer IO = ImageIOType::New();

  reader->SetImageIO( IO );
  reader->SetFileName( this->file_name_ );

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
bool ITKLayerImporterPrivate::import_simple_volume()
{
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


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


ITKLayerImporter::ITKLayerImporter( const std::string& filename ) :
  LayerImporter( filename ),
  private_( new ITKLayerImporterPrivate )
{
}

bool ITKLayerImporter::set_file_list( const std::vector< std::string >& file_list )
{
  if ( file_list.size() )
  {
    std::string extension = boost::filesystem::path( file_list[ 0 ] ).extension();;
    boost::to_lower( extension );

    this->private_->file_name_ = file_list[ 0 ];
    this->private_->extension_ = extension;
    return true;
  }
  else
  {
    return false;
  }
}

bool ITKLayerImporter::import_header()
{
  if ( this->private_->read_header_ ) return true;

  if ( this->private_->extension_ == ".tif" || this->private_->extension_ == ".tiff" 
    || this->private_->extension_ == ".stk" )
  {
    return this->private_->scan_simple_volume< itk::TIFFImageIO >();
  }

  if( this->private_->extension_ == ".vtk"  )
  {
    return this->private_->scan_simple_volume< itk::VTKImageIO >();
  }

  if( this->private_->extension_ == ".lsm"  )
  {
    return this->private_->scan_simple_volume< itk::LSMImageIO >();
  }

  if( this->private_->extension_ == ".img" || this->private_->extension_ == ".hdr" )
  {
    return this->private_->scan_simple_volume< itk::AnalyzeImageIO >();
  } 
  
  if( this->private_->extension_ == ".nii" )
  {
    return this->private_->scan_simple_volume< itk::NiftiImageIO >();
  } 
  
  if( this->private_->extension_ == ".mha" || this->private_->extension_ == ".mhd" )
  {
    return this->private_->scan_simple_volume< itk::MetaImageIO >();
  }   
  
  return false; 
}

Core::GridTransform ITKLayerImporter::get_grid_transform()
{
  if( this->private_->image_data_ ) return this->private_->image_data_->get_grid_transform();
  return Core::GridTransform( 1, 1, 1 );
}

Core::DataType ITKLayerImporter::get_data_type()
{
  if( this->private_->image_data_ ) return this->private_->image_data_->get_data_type();
  return Core::DataType::UNKNOWN_E;
}

int ITKLayerImporter::get_importer_modes()
{
  return LayerImporterMode::DATA_E;
}
  
bool ITKLayerImporter::load_data( Core::DataBlockHandle& data_block, 
  Core::GridTransform& grid_trans, LayerMetaData& meta_data )
{
  if( this->private_->extension_ == ".tif" || this->private_->extension_ == ".tiff" ||
    this->private_->extension_ == ".stk" )
  {
    this->private_->import_simple_volume<itk::TIFFImageIO>();
  }
  else if( this->private_->extension_ == ".vtk"  )
  {
    this->private_->import_simple_volume<itk::VTKImageIO>();
  } 
  else if( this->private_->extension_ == ".lsm"  )  
  {
    this->private_->import_simple_volume<itk::LSMImageIO>();
  } 
  else if( this->private_->extension_ == ".img"  || this->private_->extension_ == ".hdr" )  
  {
    this->private_->import_simple_volume<itk::AnalyzeImageIO>();
  } 
  else if( this->private_->extension_ == ".nii" ) 
  {
    this->private_->import_simple_volume<itk::NiftiImageIO>();
  } 
  else if( this->private_->extension_ == ".mha" || this->private_->extension_ == ".mhd" )
  {
    this->private_->import_simple_volume<itk::MetaImageIO>();   
  }
  else
  {
    return false;
  }

  if( ( !this->private_->data_block_ ) || ( !this->private_->image_data_ ) ) return false;

  data_block = this->private_->data_block_;
  grid_trans = this->private_->image_data_->get_grid_transform();

  return true;
}

std::string ITKLayerImporter::get_layer_name()
{
  return boost::filesystem::path( this->get_filename() ).parent_path().filename();
}

} // end namespace seg3D
