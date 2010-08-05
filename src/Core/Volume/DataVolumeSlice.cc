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

#include <limits>

#include <Core/RenderResources/RenderResources.h>
#include <Core/Volume/DataVolumeSlice.h>

namespace Core
{

const unsigned int DataVolumeSlice::TEXTURE_DATA_TYPE_C = GL_UNSIGNED_SHORT;

const unsigned int DataVolumeSlice::TEXTURE_FORMAT_C = GL_LUMINANCE16;

const DataVolumeSlice::texture_data_type DataVolumeSlice::TEXTURE_VALUE_MAX_C =
  std::numeric_limits< DataVolumeSlice::texture_data_type >::max();

DataVolumeSlice::DataVolumeSlice( const DataVolumeHandle& data_volume, 
                 VolumeSliceType type, size_t slice_num ) :
  VolumeSlice( data_volume, type, slice_num )
{
  this->data_block_ = data_volume->data_block().get();
  this->add_connection( this->data_block_->data_changed_signal_.connect( 
    boost::bind( &VolumeSlice::handle_volume_updated, this ) ) );
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

template<class DATA1, class DATA2>
void CopyTypedData( DataVolumeSlice* slice, DATA1* buffer )
{
  const DATA1 numeric_min = std::numeric_limits< DATA1 >::min();
  const DATA1 numeric_max = std::numeric_limits< DATA1 >::max();
  const DATA1 numeric_range = numeric_max - numeric_min;
  const double value_min = slice->get_data_block()->get_min();
  const double value_max = slice->get_data_block()->get_max();
  const double value_range = value_max - value_min;
  const double inv_value_range = 1.0 / value_range;

  size_t current_index = slice->to_index( 0, 0 );

  // Index strides in X and Y direction. Use int instead of size_t because strides might be negative.
  const int x_stride = static_cast<int>( slice->to_index( 1, 0 ) - current_index );
  const int y_stride =  static_cast<int>( slice->to_index( 0, 1 ) - current_index );

  const size_t nx = slice->nx();
  const size_t ny = slice->ny();
  
  DATA2* data = static_cast<DATA2*>( slice->get_data_block()->get_data() );
  size_t row_start = current_index;
  for ( size_t j = 0; j < ny; j++ )
  {
    current_index = row_start;
    for ( size_t i = 0; i < nx; i++ )
    {
      buffer[ j * nx + i ] = static_cast<DATA1>(  ( data[ current_index ] - value_min ) 
        * inv_value_range * numeric_range + numeric_min );
      current_index += x_stride;
    }
    row_start += y_stride;
  }
}

void DataVolumeSlice::upload_texture()
{
  if ( !this->slice_changed_ )
    return;

  internal_lock_type lock( this->internal_mutex_ );

  if ( !this->slice_changed_ )
    return;

  // Lock the texture
  Texture::lock_type tex_lock( this->texture_->get_mutex() );
  this->texture_->bind();

  if ( this->size_changed_ )
  {
    // Make sure there is no pixel unpack buffer bound
    PixelUnpackBuffer::RestoreDefault();

    this->texture_->set_image( static_cast< int >( this->nx_ ), 
      static_cast< int >( this->ny_ ), TEXTURE_FORMAT_C );
    this->size_changed_ = false;
  }
  
  // Step 1. copy the data in the slice to a pixel unpack buffer
  PixelBufferObjectHandle pixel_buffer( new PixelUnpackBuffer );
  pixel_buffer->bind();
  pixel_buffer->set_buffer_data( sizeof( texture_data_type ) * this->nx_ * this->ny_,
    NULL, GL_STREAM_DRAW );
  texture_data_type* buffer = reinterpret_cast< texture_data_type* >(
    pixel_buffer->map_buffer( GL_WRITE_ONLY ) );

  //std::vector< texture_data_type > buffer_vector( this->nx_ * this->ny_ );
  //texture_data_type* buffer = &buffer_vector[0];

  // Lock the volume
  shared_lock_type volume_lock( this->get_mutex() );
  
  switch ( this->data_block_->get_type() )
  {
    case DataType::CHAR_E:
      CopyTypedData< texture_data_type, signed char >( this, buffer );
      break;
    case DataType::UCHAR_E:
      CopyTypedData< texture_data_type, unsigned char >( this, buffer );
      break;
    case DataType::SHORT_E:
      CopyTypedData< texture_data_type, short >( this, buffer );
      break;
    case DataType::USHORT_E:
      CopyTypedData< texture_data_type, unsigned short >( this, buffer );
      break;
    case DataType::INT_E:
      CopyTypedData< texture_data_type, int >( this, buffer );
      break;
    case DataType::UINT_E:
      CopyTypedData< texture_data_type, unsigned int >( this, buffer );
      break;
    case DataType::FLOAT_E:
      CopyTypedData< texture_data_type, float >( this, buffer );
      break;
    case DataType::DOUBLE_E:
      CopyTypedData< texture_data_type, double >( this, buffer );
      break;
  }

  volume_lock.unlock();
  
  // Step 2. copy from the pixel buffer to texture
  pixel_buffer->unmap_buffer();
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  this->texture_->set_sub_image( 0, 0, static_cast<int>( this->nx_ ), 
    static_cast<int>( this->ny_ ), NULL, GL_LUMINANCE, TEXTURE_DATA_TYPE_C );
  //PixelUnpackBuffer::RestoreDefault();
  //this->texture_->set_sub_image( 0, 0, static_cast<int>( this->nx_ ), 
  //    static_cast<int>( this->ny_ ), buffer, GL_LUMINANCE, TEXTURE_DATA_TYPE_C );
  this->texture_->unbind();

  // Step 3. release the pixel unpack buffer
  // NOTE: The texture streaming will still succeed even if the PBO is deleted.
  pixel_buffer->unbind();

  // Use glFinish here to solve synchronization issue when the slice is used in multiple views
  glFinish();

  CORE_CHECK_OPENGL_ERROR();

  this->slice_changed_ = false;
}

VolumeSliceHandle DataVolumeSlice::clone()
{
  return VolumeSliceHandle( new DataVolumeSlice( *this ) );
}

} // end namespace Core