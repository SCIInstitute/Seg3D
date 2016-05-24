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

#include <limits>

#include <Core/RenderResources/RenderResources.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Graphics/PixelBufferObject.h>

namespace Core
{

const unsigned int DataVolumeSlice::TEXTURE_DATA_TYPE_C = GL_UNSIGNED_SHORT;

const unsigned int DataVolumeSlice::TEXTURE_FORMAT_C = GL_LUMINANCE16;

DataVolumeSlice::DataVolumeSlice( const DataVolumeHandle& data_volume, 
                 VolumeSliceType type, size_t slice_num ) :
  VolumeSlice( data_volume, type, slice_num )
{
  this->data_block_ = data_volume->get_data_block().get();
  if ( this->data_block_ )
  {
    this->add_connection( this->data_block_->data_changed_signal_.connect( 
      boost::bind( &VolumeSlice::handle_volume_updated, this ) ) );
  }
}

DataVolumeSlice::DataVolumeSlice( const DataVolumeSlice &copy ) :
  VolumeSlice( copy ),
  data_block_( copy.data_block_ )
{
}

DataVolumeSlice::~DataVolumeSlice()
{
  this->disconnect_all();
}

template<class TYPE1, class TYPE2>
void CopyTypedData( DataVolumeSlice* slice, TYPE1* buffer, DataBlock* data_block )
{
  const double numeric_min = static_cast<double>( std::numeric_limits< TYPE1 >::min() );
  const double numeric_max = static_cast<double>( std::numeric_limits< TYPE1 >::max() );
  const double value_min = data_block->get_min();
  const double value_max = data_block->get_max();
  const double value_range = value_max - value_min;
  const double inv_value_range = ( numeric_max - numeric_min ) / value_range;

  const TYPE2 typed_value_min = static_cast<TYPE2>( value_min );

  size_t current_index = slice->to_index( 0, 0 );

  // Index strides in X and Y direction. Use int instead of size_t because strides might be negative.
  const int x_stride = slice->nx() > 1 ? static_cast<int>( slice->to_index( 1, 0 ) - current_index ) : 0;
  const int y_stride = slice->ny() > 1 ? static_cast<int>( slice->to_index( 0, 1 ) - current_index ) : 0;

  const size_t nx = slice->nx();
  const size_t ny = slice->ny();
  
  TYPE2* data = static_cast<TYPE2*>( data_block->get_data() );
  size_t row_start = current_index;
  for ( size_t j = 0; j < ny; j++ )
  {
    current_index = row_start;
    for ( size_t i = 0; i < nx; i++ )
    {
      // NOTE: removed unnecessary addition for unsigned texture types
      // buffer[ j * nx + i ] = static_cast<TYPE1>(   ( data[ current_index ] - typed_value_min ) 
      //  * inv_value_range + numeric_min );
      buffer[ j * nx + i ] = static_cast<TYPE1>(  
        ( data[ current_index ] - typed_value_min ) * inv_value_range );

      current_index += x_stride;
    }
    row_start += y_stride;
  }
}

void DataVolumeSlice::upload_texture()
{
  lock_type lock( this->get_mutex() );

  if ( !this->get_slice_changed() )
    return;

  size_t nx = this->nx();
  size_t ny = this->ny();

  RenderResources::lock_type rr_lock( RenderResources::GetMutex() );

  // Lock the texture
  Texture2DHandle tex = this->get_texture();
  Texture::lock_type tex_lock( tex->get_mutex() );
  tex->bind();


  if ( this->get_size_changed() )
  {
    // Make sure there is no pixel unpack buffer bound
    PixelUnpackBuffer::RestoreDefault();

    tex->set_image( static_cast< int >( nx ), 
      static_cast< int >( ny ), TEXTURE_FORMAT_C );
    this->set_size_changed( false );
  }
  
  // Step 1. copy the data in the slice to a pixel unpack buffer
  PixelBufferObjectHandle pixel_buffer( new PixelUnpackBuffer );
  pixel_buffer->bind();
  pixel_buffer->set_buffer_data( sizeof( texture_data_type ) * nx * ny,
    NULL, GL_STREAM_DRAW );
  texture_data_type* buffer = reinterpret_cast< texture_data_type* >(
    pixel_buffer->map_buffer( GL_WRITE_ONLY ) );

  // Lock the volume
  DataBlock::shared_lock_type volume_lock( this->data_block_->get_mutex() );
  
  switch ( this->data_block_->get_data_type() )
  {
    case DataType::CHAR_E:
      CopyTypedData< texture_data_type, signed char >( this, buffer, this->data_block_ );
      break;
    case DataType::UCHAR_E:
      CopyTypedData< texture_data_type, unsigned char >( this, buffer, this->data_block_ );
      break;
    case DataType::SHORT_E:
      CopyTypedData< texture_data_type, short >( this, buffer, this->data_block_ );
      break;
    case DataType::USHORT_E:
      CopyTypedData< texture_data_type, unsigned short >( this, buffer, this->data_block_ );
      break;
    case DataType::INT_E:
      CopyTypedData< texture_data_type, int >( this, buffer, this->data_block_ );
      break;
    case DataType::UINT_E:
      CopyTypedData< texture_data_type, unsigned int >( this, buffer, this->data_block_ );
      break;
    case DataType::FLOAT_E:
      CopyTypedData< texture_data_type, float >( this, buffer, this->data_block_ );
      break;
    case DataType::DOUBLE_E:
      CopyTypedData< texture_data_type, double >( this, buffer, this->data_block_ );
      break;
  }

  volume_lock.unlock();
  
  // Step 2. copy from the pixel buffer to texture
  pixel_buffer->unmap_buffer();
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  tex->set_sub_image( 0, 0, static_cast<int>( nx ), 
    static_cast<int>( ny ), NULL, GL_LUMINANCE, TEXTURE_DATA_TYPE_C );
  tex->unbind();

  // Step 3. release the pixel unpack buffer
  // NOTE: The texture streaming will still succeed even if the PBO is deleted.
  pixel_buffer->unbind();

  // Use glFinish here to solve synchronization issue when the slice is used in multiple views
  glFinish();

  CORE_CHECK_OPENGL_ERROR();

  this->set_slice_changed( false );
}

VolumeSliceHandle DataVolumeSlice::clone()
{
  lock_type lock( this->get_mutex() );
  return VolumeSliceHandle( new DataVolumeSlice( *this ) );
}

void DataVolumeSlice::set_volume( const VolumeHandle& volume )
{
  lock_type lock( this->get_mutex() );

  VolumeSlice::set_volume( volume );
  DataVolume* data_volume = dynamic_cast< DataVolume* >( volume.get() );
  assert( data_volume != 0 );
  this->data_block_ = data_volume->get_data_block().get();
  if ( this->data_block_ )
  {
    this->add_connection( this->data_block_->data_changed_signal_.connect( 
      boost::bind( &VolumeSlice::handle_volume_updated, this ) ) );
  }
}

double DataVolumeSlice::get_data_at( size_t i, size_t j ) const
{
  lock_type lock( this->get_mutex() );
  return this->data_block_->get_data_at( this->to_index( i, j ) );
}

void DataVolumeSlice::set_data_at( size_t i, size_t j, double value )
{
  lock_type lock( this->get_mutex() );
  this->data_block_->set_data_at( this->to_index( i, j ), value );
}

template<class T >
void ThresholdTypedData( const DataVolumeSlice* slice, DataBlock* data_block,
  unsigned char* buffer, double min_val, double max_val, bool negative_constraint )
{
  size_t current_index = slice->to_index( 0, 0 );

  // Index strides in X and Y direction. Use int instead of size_t because strides might be negative.
  const int x_stride = static_cast<int>( slice->to_index( 1, 0 ) - current_index );
  const int y_stride =  static_cast<int>( slice->to_index( 0, 1 ) - current_index );

  const size_t nx = slice->nx();
  const size_t ny = slice->ny();

  T* data = static_cast< T* >( data_block->get_data() );
  size_t row_start = current_index;
  bool in_range;
  for ( size_t j = 0; j < ny; j++ )
  {
    current_index = row_start;
    for ( size_t i = 0; i < nx; i++ )
    {
      in_range = ( data[ current_index ] >= min_val && 
        data[ current_index ] <= max_val );
      buffer[ j * nx + i ] = negative_constraint ? !in_range : in_range;
      current_index += x_stride;
    }
    row_start += y_stride;
  }
}

void DataVolumeSlice::create_threshold_mask( std::vector< unsigned char >& mask, 
    double min_val, double max_val, bool negative_constraint ) const
{
  lock_type lock( this->get_mutex() );

  mask.resize( this->nx() * this->ny() );
  DataBlock::shared_lock_type volume_lock( this->data_block_->get_mutex() );
  switch ( this->data_block_->get_data_type() )
  {
  case DataType::CHAR_E:
    ThresholdTypedData< signed char >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  case DataType::UCHAR_E:
    ThresholdTypedData< unsigned char >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  case DataType::SHORT_E:
    ThresholdTypedData< short >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  case DataType::USHORT_E:
    ThresholdTypedData< unsigned short >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  case DataType::INT_E:
    ThresholdTypedData< int >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  case DataType::UINT_E:
    ThresholdTypedData< unsigned int >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  case DataType::FLOAT_E:
    ThresholdTypedData< float >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  case DataType::DOUBLE_E:
    ThresholdTypedData< double >( this, this->data_block_, &mask[ 0 ], 
      min_val, max_val, negative_constraint );
    break;
  }
}


} // end namespace Core
