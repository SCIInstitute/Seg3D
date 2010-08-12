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

#include <Core/Application/Application.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Graphics/PixelBufferObject.h>

namespace Core
{

class MaskVolumeSlicePrivate
{
public:
  bool using_cache_;
  std::vector< unsigned char > cache_;
  mutable MaskVolumeSlice::cache_mutex_type cache_mutex_;
};

MaskVolumeSlice::MaskVolumeSlice( const MaskVolumeHandle& mask_volume, 
                 VolumeSliceType type, size_t slice_num ) :
  VolumeSlice( mask_volume, type, slice_num ),
  mask_data_block_( mask_volume->get_mask_data_block().get() ),
  private_( new MaskVolumeSlicePrivate )
{
  this->private_->using_cache_ = false;
  this->add_connection( this->mask_data_block_->mask_updated_signal_.connect( 
    boost::bind( &VolumeSlice::handle_volume_updated, this ), boost::signals2::at_front ) );
  this->add_connection( this->cache_updated_signal_.connect( boost::bind(
    &VolumeSlice::handle_volume_updated, this ) ) );
}

MaskVolumeSlice::MaskVolumeSlice( const MaskVolumeSlice& copy ) :
  VolumeSlice( copy ),
  mask_data_block_( copy.mask_data_block_ ),
  private_( copy.private_ )
{
}

MaskVolumeSlice::~MaskVolumeSlice()
{
  this->disconnect_all();
}

static void CopyMaskData( const MaskVolumeSlice* slice, unsigned char* buffer )
{
  size_t current_index = slice->to_index( 0, 0 );

  // Index strides in X and Y direction. Use int instead of size_t because strides might be negative.
  const int x_stride = static_cast<int>( slice->to_index( 1, 0 ) - current_index );
  const int y_stride =  static_cast<int>( slice->to_index( 0, 1 ) - current_index );

  unsigned char* mask_data = slice->get_mask_data_block()->get_mask_data();
  unsigned char mask_value = slice->get_mask_data_block()->get_mask_value();
  const size_t nx = slice->nx();
  const size_t ny = slice->ny();

  size_t row_start = current_index;
  for ( size_t j = 0; j < ny; j++ )
  {
    current_index = row_start;
    for ( size_t i = 0; i < nx; i++ )
    {
      buffer[ j * nx + i ] = mask_data[ current_index ] & mask_value ;

      current_index += x_stride;
    }
    row_start += y_stride;
  }
}

void MaskVolumeSlice::upload_texture()
{
  if ( !this->slice_changed_ )
    return;

  internal_lock_type lock( this->internal_mutex_ );

  if ( !this->slice_changed_ )
    return;

  RenderResources::lock_type rr_lock( RenderResources::GetMutex() );

  // Lock the texture
  Texture::lock_type tex_lock( this->texture_->get_mutex() );
  this->texture_->bind();

  if ( this->size_changed_ )
  {
    // Make sure there is no pixel unpack buffer bound
    PixelUnpackBuffer::RestoreDefault();
    this->texture_->set_image( static_cast<int>( this->nx_ ), 
      static_cast<int>( this->ny_ ), GL_ALPHA );
    this->size_changed_ = false;
  }
  
  cache_lock_type cache_lock( this->get_cache_mutex() );
  if ( this->private_->using_cache_ )
  {
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    this->texture_->set_sub_image( 0, 0, static_cast<int>( this->nx_ ), 
      static_cast<int>( this->ny_ ), &this->private_->cache_[ 0 ], GL_ALPHA, GL_UNSIGNED_BYTE );
    this->texture_->unbind();
    cache_lock.unlock();
  }
  else
  {
    cache_lock.unlock();

    // Step 1. copy the data in the slice to a pixel unpack buffer
    PixelBufferObjectHandle pixel_buffer( new PixelUnpackBuffer );
    pixel_buffer->bind();
    pixel_buffer->set_buffer_data( sizeof(unsigned char) * this->nx_ * this->ny_,
      NULL, GL_STREAM_DRAW );
    unsigned char* buffer = reinterpret_cast<unsigned char*>(
      pixel_buffer->map_buffer( GL_WRITE_ONLY ) );

    {
      shared_lock_type volume_lock( this->get_mutex() );
      CopyMaskData( this, buffer );
    }
    
    // Step 2. copy from the pixel buffer to texture
    pixel_buffer->unmap_buffer();
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    this->texture_->set_sub_image( 0, 0, static_cast<int>( this->nx_ ), 
      static_cast<int>( this->ny_ ), NULL, GL_ALPHA, GL_UNSIGNED_BYTE );
    this->texture_->unbind();

    // Step 3. release the pixel unpack buffer
    // NOTE: The texture streaming will still succeed even if the PBO is deleted.
    pixel_buffer->unbind();

    // Use glFinish here to solve synchronization issue when the slice is used in multiple views
    glFinish();

  }

  this->slice_changed_ = false;
}

VolumeSliceHandle MaskVolumeSlice::clone()
{
  return VolumeSliceHandle( new MaskVolumeSlice( *this ) );
}

MaskDataBlockHandle MaskVolumeSlice::get_mask_data_block() const
{
  return this->mask_data_block_->shared_from_this();
}

MaskVolumeSlice::cache_mutex_type& MaskVolumeSlice::get_cache_mutex() const
{
  return this->private_->cache_mutex_;
}

unsigned char* MaskVolumeSlice::get_cached_data()
{
  ASSERT_IS_APPLICATION_THREAD();

  cache_lock_type cache_lock( this->get_cache_mutex() );
  if ( !this->private_->using_cache_ )
  {
    this->private_->cache_.resize( this->nx() * this->ny() );
    
    {
      shared_lock_type volume_lock( this->get_mutex() );
      CopyMaskData( this, &this->private_->cache_[ 0 ] );
    }

    this->private_->using_cache_ = true;
  }
  
  return &this->private_->cache_[ 0 ];
}

static void CopyCachedDataBack( MaskVolumeSlice* slice, const unsigned char* buffer )
{
  size_t current_index = slice->to_index( 0, 0 );

  // Index strides in X and Y direction. Use int instead of size_t because strides might be negative.
  const int x_stride = static_cast<int>( slice->to_index( 1, 0 ) - current_index );
  const int y_stride =  static_cast<int>( slice->to_index( 0, 1 ) - current_index );

  unsigned char* mask_data = slice->get_mask_data_block()->get_mask_data();
  unsigned char mask_value = slice->get_mask_data_block()->get_mask_value();
  unsigned char not_mask_value = ~mask_value;
  const size_t nx = slice->nx();
  const size_t ny = slice->ny();

  size_t row_start = current_index;
  for ( size_t j = 0; j < ny; j++ )
  {
    current_index = row_start;
    for ( size_t i = 0; i < nx; i++ )
    {
      bool has_mask = buffer[ j * nx + i ] != 0;
      if ( has_mask )
      {
        mask_data[ current_index ] |= mask_value;
      }
      else
      {
        mask_data[ current_index ] &= not_mask_value;
      }
      
      current_index += x_stride;
    }
    row_start += y_stride;
  }
}

void MaskVolumeSlice::release_cached_data()
{
  // Repost the request to the application thread, so getting and releasing the cache
  // will happen in sync.
  if ( !Application::IsApplicationThread() )
  {
    Application::PostEvent( boost::bind( &MaskVolumeSlice::release_cached_data, this ) );
    return;
  }
  
  cache_lock_type cache_lock( this->get_cache_mutex() );
  if ( !this->private_->using_cache_ )
  {
    return;
  }
  
  {
    lock_type volume_lock( this->get_mutex() );
    CopyCachedDataBack( this, &this->private_->cache_[ 0 ] );
    this->mask_data_block_->increase_generation();
  }
  
  this->private_->cache_.resize( 0 );
  this->private_->using_cache_ = false;
  cache_lock.unlock();
  
  this->mask_data_block_->mask_updated_signal_();
}

} // end namespace Core