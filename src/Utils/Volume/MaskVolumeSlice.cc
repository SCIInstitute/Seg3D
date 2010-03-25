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

#include <Utils/Volume/MaskVolumeSlice.h>

namespace Utils
{

MaskVolumeSlice::MaskVolumeSlice( const MaskVolumeHandle& mask_volume, 
                 VolumeSliceType type, size_t slice_num ) :
  VolumeSlice( mask_volume, type, slice_num )
{
  this->mask_data_block_ = mask_volume->mask_data_block().get();
  this->add_connection( this->mask_data_block_->mask_updated_signal_.connect( 
    boost::bind( &VolumeSlice::volume_updated_slot, this ) ) );
}

MaskVolumeSlice::MaskVolumeSlice( const MaskVolumeSlice& copy ) :
  VolumeSlice( copy ),
  mask_data_block_( copy.mask_data_block_ )
{
}

MaskVolumeSlice::~MaskVolumeSlice()
{
  this->disconnect_all();
}

void MaskVolumeSlice::initialize_texture()
{
  if ( !this->texture_ )
  {
    internal_lock_type lock( this->internal_mutex_ );
    if ( !this->texture_ )
    {
      this->texture_ = Texture2DHandle( new Texture2D );
      // It doesn't make sense to use linear interpolation for mask texture
      this->texture_->set_mag_filter( GL_NEAREST );
      this->texture_->set_min_filter( GL_NEAREST );
    }
  }
}

void MaskVolumeSlice::upload_texture()
{
  if ( !this->slice_changed_ )
    return;

  internal_lock_type lock( this->internal_mutex_ );

  if ( !this->slice_changed_ )
    return;

  // Lock the texture
  Texture::lock_type tex_lock( this->texture_->get_mutex() );

  if ( this->size_changed_ )
  {
    // Make sure there is no pixel unpack buffer bound
    PixelUnpackBuffer::RestoreDefault();

    this->texture_->set_image( static_cast<int>( this->nx_ ), 
      static_cast<int>( this->ny_ ), GL_ALPHA );
    this->size_changed_ = false;
  }
  
  // Step 1. copy the data in the slice to a pixel unpack buffer
  PixelBufferObjectHandle pixel_buffer( new PixelUnpackBuffer );
  pixel_buffer->bind();
  pixel_buffer->set_buffer_data( sizeof(unsigned char) * this->nx_ * this->ny_,
    NULL, GL_STREAM_DRAW );
  unsigned char* buffer = reinterpret_cast<unsigned char*>(
    pixel_buffer->map_buffer( GL_WRITE_ONLY ) );

  // Lock the volume
  lock_type volume_lock( this->get_mutex() );

  size_t current_index = this->to_index( 0, 0 );

  // Index strides in X and Y direction. Use int instead of size_t because strides might be negative.
  int x_stride = static_cast<int>( this->to_index( 1, 0 ) - current_index );
  int y_stride =  static_cast<int>( this->to_index( 0, 1 ) - current_index );

  size_t row_start = current_index;
  for ( size_t j = 0; j < this->ny_; j++ )
  {
    current_index = row_start;
    for ( size_t i = 0; i < this->nx_; i++ )
    {
      buffer[ j * this->nx_ + i ] = this->mask_data_block_->get_mask_at( current_index );
      current_index += x_stride;
    }
    row_start += y_stride;
  }

  volume_lock.unlock();
  
  // Step 2. copy from the pixel buffer to texture
  pixel_buffer->unmap_buffer();
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  this->texture_->set_sub_image( 0, 0, static_cast<int>( this->nx_ ), 
    static_cast<int>( this->ny_ ), NULL, GL_ALPHA, GL_UNSIGNED_BYTE );

  // Step 3. release the pixel unpack buffer
  // NOTE: The texture streaming will still succeed even if the PBO is deleted.
  pixel_buffer->unbind();

  this->slice_changed_ = false;
}

} // end namespace Utils