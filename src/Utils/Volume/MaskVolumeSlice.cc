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

    this->texture_->set_image( static_cast<int>( this->width_ ), 
      static_cast<int>( this->height_ ), GL_ALPHA );
    this->size_changed_ = false;
  }
  
  // Step 1. copy the data in the slice to a pixel unpack buffer
  this->pixel_buffer_ = PixelBufferObjectHandle( new PixelUnpackBuffer );
  this->pixel_buffer_->bind();
  this->pixel_buffer_->set_buffer_data( sizeof(unsigned char) * this->width_ * this->height_,
    NULL, GL_STREAM_DRAW );
  unsigned char* buffer = reinterpret_cast<unsigned char*>(
    this->pixel_buffer_->map_buffer( GL_WRITE_ONLY ) );

  // Lock the volume
  lock_type volume_lock( this->get_mutex() );
  for ( size_t j = 0; j < this->height_; j++ )
  {
    for ( size_t i = 0; i < this->width_; i++ )
    {
      size_t index = this->to_index( i, j );
      buffer[ j * this->width_ + i ] = this->mask_data_block_->get_mask_at( index );
    }
  }
  volume_lock.unlock();
  
  // Step 2. copy from the pixel buffer to texture
  this->pixel_buffer_->unmap_buffer();
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  this->texture_->set_sub_image( 0, 0, static_cast<int>( this->width_ ), 
    static_cast<int>( this->height_ ), NULL, GL_ALPHA, GL_UNSIGNED_BYTE );

  // Step 3. release the pixel unpack buffer
  // NOTE: The texture streaming will still succeed even if the PBO is deleted.
  this->pixel_buffer_->unbind();
  this->pixel_buffer_.reset();

  this->slice_changed_ = false;
}

} // end namespace Utils