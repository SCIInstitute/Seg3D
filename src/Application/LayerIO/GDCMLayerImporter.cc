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
#pragma warning( push )
#pragma warning( disable: 4267 )
#endif

// STL includes
#include <limits>

// GDCM Includes
#include <gdcmImageReader.h>
#include <gdcmImageHelper.h>
#include <gdcmRescaler.h>
#include <gdcmAttribute.h>
#include <gdcmUnpacker12Bits.h>

#ifdef _WIN32
#pragma warning( pop )
#endif

// Core includes
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerIO/GDCMLayerImporter.h>
#include <Application/LayerIO/LayerIO.h>

SCI_REGISTER_IMPORTER( Seg3D, GDCMLayerImporter );

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class GDCMLayerImporterPrivate
//////////////////////////////////////////////////////////////////////////

class GDCMLayerImporterPrivate
{
public:
  GDCMLayerImporterPrivate() : pixel_type_( Core::DataType::UCHAR_E ) {}
  double get_slice_thickness( const gdcm::DataSet& ds );
  bool read_image( const std::string& filename, char* buffer );

  GDCMLayerImporter* importer_;
  std::vector< std::string > file_list_;
  bool file_series_;
  double rescale_slope_;
  double rescale_intercept_;
  unsigned long buffer_length_;
  unsigned long slice_data_size_;

  Core::DataType pixel_type_;
  Core::GridTransform grid_transform_;

  Core::Point origin_;
  Core::Vector row_direction_;
  Core::Vector col_direction_;
  Core::Vector slice_direction_;
  double x_spacing_, y_spacing_, z_spacing_;
};

double GDCMLayerImporterPrivate::get_slice_thickness( const gdcm::DataSet& ds )
{
  gdcm::Tag slice_thickness_tag( 0x0018,0x0050 );

  if( ds.FindDataElement( slice_thickness_tag ) ) // Slice Thickness
  {
    const gdcm::DataElement& de = ds.GetDataElement( slice_thickness_tag );
    if ( de.IsEmpty() )
    {
      return 1.0;
    }
    gdcm::Attribute< 0x0018, 0x0050 > slice_thickness;
    slice_thickness.SetFromDataElement( de );
    double thickness = slice_thickness.GetValue( 0 );
    if ( thickness > 0 )
    {
      return thickness;
    }
  }

  return 1.0;
}

bool GDCMLayerImporterPrivate::read_image( const std::string& filename, char* buffer )
{
  gdcm::ImageReader reader;
  reader.SetFileName( filename.c_str() );
  if ( !reader.Read() )
  {
    this->importer_->set_error( "Failed to read file '" + filename + "'" );
    return false;
  }
  
  gdcm::Image& image = reader.GetImage();
  if ( this->buffer_length_ != image.GetBufferLength() )
  {
    this->importer_->set_error( "Images in the series have different sizes" );
    return false;
  }
  
  image.GetBuffer( buffer );
  const gdcm::PixelFormat& pixeltype = image.GetPixelFormat();
  if ( pixeltype == gdcm::PixelFormat::UINT12 || pixeltype == gdcm::PixelFormat::INT12 )
  {
    if ( this->rescale_slope_ != 1.0 || this->rescale_intercept_ != 0.0 )
    {
      this->importer_->set_error( "Unsupported data format" );
      return false;
    }
    
    std::vector< char > copy( this->buffer_length_ );
    memcpy( &copy[ 0 ], buffer, this->buffer_length_ );
    if ( !gdcm::Unpacker12Bits::Unpack( buffer, &copy[ 0 ], this->buffer_length_ ) )
    {
      this->importer_->set_error( "Failed to unpack 12bit data" );
      return false;
    }
    assert( this->buffer_length_ * 16 / 12 == this->slice_data_size_ );
  }
  
  if ( this->rescale_slope_ != 1.0 || this->rescale_intercept_ != 0.0 )
  {
    gdcm::Rescaler r;
    r.SetIntercept( this->rescale_intercept_ );
    r.SetSlope( this->rescale_slope_ );
    r.SetPixelFormat( pixeltype );
    gdcm::PixelFormat outputpt = r.ComputeInterceptSlopePixelType();
    std::vector< char > copy( this->buffer_length_ );
    memcpy( &copy[ 0 ], buffer, this->buffer_length_ );
    r.Rescale( buffer, &copy[ 0 ], this->buffer_length_ );
    assert( this->buffer_length_ * outputpt.GetPixelSize() / pixeltype.GetPixelSize() 
      == this->slice_data_size_ );
  }
  
  return true;
}


//////////////////////////////////////////////////////////////////////////
// Class GDCMLayerImporter
//////////////////////////////////////////////////////////////////////////

GDCMLayerImporter::GDCMLayerImporter( const std::string& filename ) :
  LayerImporter( filename ),
  private_( new GDCMLayerImporterPrivate )
{
  this->private_->importer_ = this;
}

bool GDCMLayerImporter::import_header()
{
  gdcm::ImageHelper::SetForcePixelSpacing( true );
  gdcm::ImageHelper::SetForceRescaleInterceptSlope( true );

  gdcm::ImageReader reader;
  reader.SetFileName( this->private_->file_list_[ 0 ].c_str() );
  if ( !reader.Read() )
  {
    this->set_error( "Can't read file " + this->private_->file_list_[ 0 ] );
    return false;
  }
  
  const gdcm::Image &image = reader.GetImage();
  const gdcm::File &f = reader.GetFile();
  const gdcm::DataSet &ds = f.GetDataSet();
  const unsigned int *dims = image.GetDimensions();
  const gdcm::PixelFormat &pixeltype = image.GetPixelFormat();
  this->private_->buffer_length_ = image.GetBufferLength();

  if ( pixeltype.GetSamplesPerPixel() != 1 )
  {
    this->set_error( "Unsupported pixel format" );
    return false;
  }

  unsigned int num_of_dimensions = image.GetNumberOfDimensions(); 
  if ( num_of_dimensions != 2 && num_of_dimensions != 3 )
  {
    this->set_error( "Unsupported number of dimensions" );
    return false;
  }
  this->private_->grid_transform_.set_nx( dims[ 0 ] );
  this->private_->grid_transform_.set_ny( dims[ 1 ] );
  if ( num_of_dimensions == 2 )
  {
    this->private_->grid_transform_.set_nz( this->private_->file_list_.size() );
    this->private_->file_series_ = true;
  }
  else
  {
    this->private_->grid_transform_.set_nz( dims[ 2 ] );
    this->private_->file_series_ = false;
  }

  this->private_->rescale_intercept_ = image.GetIntercept();
  this->private_->rescale_slope_ = image.GetSlope();

  gdcm::Rescaler r;
  r.SetIntercept( this->private_->rescale_intercept_ );
  r.SetSlope( this->private_->rescale_slope_ );
  r.SetPixelFormat( pixeltype );
  gdcm::PixelFormat::ScalarType outputpt = r.ComputeInterceptSlopePixelType();
  unsigned long pixel_size = 0;

  // Scan the data type
  assert( pixeltype <= outputpt );
  switch( outputpt )
  {
  case gdcm::PixelFormat::INT8:
    this->private_->pixel_type_ = Core::DataType::CHAR_E;
    pixel_size = sizeof( char );
    break;
  case gdcm::PixelFormat::UINT8:
    this->private_->pixel_type_ = Core::DataType::UCHAR_E;
    pixel_size = sizeof( unsigned char );
    break;
  case gdcm::PixelFormat::INT12:
    this->private_->pixel_type_ = Core::DataType::SHORT_E;
    pixel_size = sizeof( short );
    break;
  case gdcm::PixelFormat::UINT12:
    this->private_->pixel_type_ = Core::DataType::USHORT_E;
    pixel_size = sizeof( unsigned short );
    break;
  case gdcm::PixelFormat::INT16:
    this->private_->pixel_type_ = Core::DataType::SHORT_E;
    pixel_size = sizeof( short );
    break;
  case gdcm::PixelFormat::UINT16:
    this->private_->pixel_type_ = Core::DataType::USHORT_E;
    pixel_size = sizeof( unsigned short );
    break;
  case gdcm::PixelFormat::INT32:
    this->private_->pixel_type_ = Core::DataType::INT_E;
    pixel_size = sizeof( int );
    break;
  case gdcm::PixelFormat::UINT32:
    this->private_->pixel_type_ = Core::DataType::UINT_E;
    pixel_size = sizeof( unsigned int );
    break;
  case gdcm::PixelFormat::FLOAT32:
    this->private_->pixel_type_ = Core::DataType::FLOAT_E;
    pixel_size = sizeof( float );
    break;
  case gdcm::PixelFormat::FLOAT64:
    this->private_->pixel_type_ = Core::DataType::DOUBLE_E;
    pixel_size = sizeof( double );
    break;
  default:
    this->set_error( "Unknown data type" );
    return false; 
  }

  double epsilon = std::numeric_limits<double>::epsilon() * 10.0;

  this->private_->slice_data_size_ = pixel_size * dims[ 0 ] * dims[ 1 ];

  // Compute the grid transform
  const double* spacing = image.GetSpacing();
  const double* origin = image.GetOrigin();

  const double* dircos = image.GetDirectionCosines();
  this->private_->row_direction_ = Core::Vector( dircos[ 0 ], dircos[ 1 ], dircos[ 2 ] );
  this->private_->col_direction_ = Core::Vector( dircos[ 3 ], dircos[ 4 ], dircos[ 5 ] );
  this->private_->slice_direction_ = Core::Cross( this->private_->row_direction_, 
    this->private_->col_direction_ );

  this->private_->origin_[ 0 ] = origin[ 0 ];
  this->private_->origin_[ 1 ] = origin[ 1 ];
  this->private_->origin_[ 2 ] = origin[ 2 ];

  this->private_->x_spacing_ = spacing[ 0 ];
  this->private_->y_spacing_ = spacing[ 1 ];
  
  if ( spacing[ 2 ] == 1.0 )
  {
    gdcm::Tag slice_thickness_tag( 0x0018,0x0050 );
    gdcm::Tag patient_position_tag( 0x0020, 0x0032 );

    bool found_thickness = false;
    
    if( ds.FindDataElement( slice_thickness_tag ) ) // Slice Thickness
    {
      const gdcm::DataElement& de = ds.GetDataElement( slice_thickness_tag );
      if ( ! de.IsEmpty() )
      {
        gdcm::Attribute< 0x0018, 0x0050 > slice_thickness;
        slice_thickness.SetFromDataElement( de );
        double thickness = slice_thickness.GetValue( 0 );
        if ( thickness > epsilon )
        {
          this->private_->z_spacing_ = thickness;
          this->private_->slice_direction_ = Core::Vector( 0.0, 0.0, 1.0 );
          found_thickness = true;
        }
      }
    }
    
    if ( this->private_->file_list_.size() > 1 && ds.FindDataElement( patient_position_tag ) )
    {
      gdcm::ImageReader reader2;
      reader2.SetFileName( this->private_->file_list_[ 1 ].c_str() );
      if ( !reader2.Read() )
      {
        this->set_error( "Can't read file " + this->private_->file_list_[ 1 ] );
        return false;
      }
      
      const gdcm::Image &image2 = reader2.GetImage();
      const double* origin2 = image2.GetOrigin();
      
      double spacing = origin2[ 2 ] - origin[ 2 ];
      if ( spacing < -epsilon || spacing > epsilon ) 
      {
        this->private_->z_spacing_ = spacing; 
      }
      else 
      {
        spacing = 1.0;
      }
      this->private_->slice_direction_ = Core::Vector( 0.0, 0.0, 1.0 );
    }
    else if ( found_thickness == false )
    {
      this->private_->z_spacing_ = 1.0;
    }
  }
  else
  {
    this->private_->z_spacing_ = spacing[ 2 ];
  }

  return true;
}

Core::GridTransform GDCMLayerImporter::get_grid_transform()
{
  return this->private_->grid_transform_;
}

Core::DataType GDCMLayerImporter::get_data_type()
{
  return this->private_->pixel_type_;
}

int GDCMLayerImporter::get_importer_modes()
{
  return LayerImporterMode::DATA_E;
}

bool GDCMLayerImporter::load_data( Core::DataBlockHandle& data_block, 
  Core::GridTransform& grid_trans, LayerMetaData& meta_data )
{
  if ( this->get_swap_xy_spacing() )
  {
    std::swap( this->private_->x_spacing_, this->private_->y_spacing_ );
  }
  this->private_->row_direction_ *= this->private_->x_spacing_;
  this->private_->col_direction_ *= this->private_->y_spacing_;
  this->private_->slice_direction_ *= this->private_->z_spacing_;
  this->private_->grid_transform_.load_basis( this->private_->origin_, 
    this->private_->row_direction_, this->private_->col_direction_, 
    this->private_->slice_direction_ );
  this->private_->grid_transform_.set_originally_node_centered( false );

  data_block = Core::StdDataBlock::New( this->private_->grid_transform_,
    this->private_->pixel_type_ );
  grid_trans = this->private_->grid_transform_;
  char* data = reinterpret_cast< char* >( data_block->get_data() );

  if( !this->private_->read_image( this->private_->file_list_[ 0 ], data ) )
  {
    data_block.reset();
    return false;
  }

  for ( size_t i = 1; this->private_->file_series_ && i < this->private_->file_list_.size(); ++i )
  {
    if ( !this->private_->read_image( this->private_->file_list_[ i ], 
      data + this->private_->slice_data_size_ * i ) )
    {
      data_block.reset();
      return false;
    }
  }

  if ( this->private_->file_list_.size() )
  {
    meta_data.meta_data_ = Core::ExportToString( this->private_->file_list_ );
    meta_data.meta_data_info_ = "dicom_filename"; 
  }
  return true;
}

std::string GDCMLayerImporter::get_layer_name()
{
  return boost::filesystem::path( this->get_filename() ).parent_path().filename();
}

std::vector< std::string > GDCMLayerImporter::get_file_list()
{
  return this->private_->file_list_;
}

bool GDCMLayerImporter::set_file_list( const std::vector< std::string >& file_list )
{
  this->private_->file_list_ = file_list;
  return true;
}

} // end namespace seg3D
