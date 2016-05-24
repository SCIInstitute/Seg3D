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

#include <algorithm>

#include <boost/lambda/lambda.hpp>

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
};

MaskVolumeSlice::MaskVolumeSlice( const MaskVolumeHandle& mask_volume, 
                 VolumeSliceType type, size_t slice_num ) :
  VolumeSlice( mask_volume, type, slice_num ),
  mask_data_block_( mask_volume->get_mask_data_block().get() ),
  private_( new MaskVolumeSlicePrivate )
{
  this->private_->using_cache_ = false;
  if ( this->mask_data_block_ )
  {
    this->add_connection( this->mask_data_block_->mask_updated_signal_.connect( 
      boost::bind( &VolumeSlice::handle_volume_updated, this ), boost::signals2::at_front ) );
    this->add_connection( this->cache_updated_signal_.connect( boost::bind(
      &VolumeSlice::handle_volume_updated, this ) ) );
  }
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

static void CopyMaskData( const MaskVolumeSlice* slice, unsigned char* buffer, bool invert = false )
{
  size_t current_index = slice->to_index( 0, 0 );

  // Index strides in X and Y direction. Use int instead of size_t because strides might be negative.
  const int x_stride = slice->nx() > 1 ? static_cast< int >( slice->to_index( 1, 0 ) - current_index ) : 0;
  const int y_stride = slice->ny() > 1 ? static_cast< int >( slice->to_index( 0, 1 ) - current_index ) : 0;

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

  if ( invert )
  {
    std::for_each( buffer, buffer + nx * ny, boost::lambda::_1 = !boost::lambda::_1 );
  }
}

void MaskVolumeSlice::upload_texture()
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
    tex->set_image( static_cast<int>( nx ), 
      static_cast<int>( ny ), GL_ALPHA );
    this->set_size_changed( false );
  }
  
  if ( this->private_->using_cache_ )
  {
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    tex->set_sub_image( 0, 0, static_cast<int>( nx ), 
      static_cast<int>( ny ), &this->private_->cache_[ 0 ], GL_ALPHA, GL_UNSIGNED_BYTE );
    tex->unbind();
  }
  else
  {
    // Step 1. copy the data in the slice to a pixel unpack buffer
    PixelBufferObjectHandle pixel_buffer( new PixelUnpackBuffer );
    pixel_buffer->bind();
    pixel_buffer->set_buffer_data( sizeof(unsigned char) * nx * ny,
      NULL, GL_STREAM_DRAW );
    unsigned char* buffer = reinterpret_cast<unsigned char*>(
      pixel_buffer->map_buffer( GL_WRITE_ONLY ) );

    {
      MaskDataBlock::shared_lock_type volume_lock( 
        this->mask_data_block_->get_mutex() );
      CopyMaskData( this, buffer );
    }
    
    // Step 2. copy from the pixel buffer to texture
    pixel_buffer->unmap_buffer();
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    tex->set_sub_image( 0, 0, static_cast<int>( nx ), 
      static_cast<int>( ny ), NULL, GL_ALPHA, GL_UNSIGNED_BYTE );
    tex->unbind();

    // Step 3. release the pixel unpack buffer
    // NOTE: The texture streaming will still succeed even if the PBO is deleted.
    pixel_buffer->unbind();

    // Use glFinish here to solve synchronization issue when the slice is used in multiple views
    glFinish();

  }

  this->set_slice_changed( false );
}

VolumeSliceHandle MaskVolumeSlice::clone()
{
  lock_type lock( this->get_mutex() );
  return VolumeSliceHandle( new MaskVolumeSlice( *this ) );
}

MaskDataBlockHandle MaskVolumeSlice::get_mask_data_block() const
{
  lock_type lock( this->get_mutex() );
  return this->mask_data_block_->shared_from_this();
}

void MaskVolumeSlice::set_volume( const VolumeHandle& volume )
{
  lock_type lock( this->get_mutex() );

  VolumeSlice::set_volume( volume );
  MaskVolume* mask_volume = dynamic_cast< MaskVolume* >( volume.get() );
  assert( mask_volume != 0 );
  this->mask_data_block_ = mask_volume->get_mask_data_block().get();
  if ( this->mask_data_block_ )
  {
    this->add_connection( this->mask_data_block_->mask_updated_signal_.connect( 
      boost::bind( &VolumeSlice::handle_volume_updated, this ), boost::signals2::at_front ) );
    this->add_connection( this->cache_updated_signal_.connect( boost::bind(
      &VolumeSlice::handle_volume_updated, this ) ) );
  }
}

unsigned char* MaskVolumeSlice::get_cached_data()
{
  ASSERT_IS_APPLICATION_THREAD();

  lock_type lock( this->get_mutex() );

  if ( !this->private_->using_cache_ )
  {
    this->private_->cache_.resize( this->nx() * this->ny() );
    
    {
      MaskDataBlock::shared_lock_type volume_lock( 
        this->mask_data_block_->get_mutex() );
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
  
  lock_type lock( this->get_mutex() );

  if ( !this->private_->using_cache_ )
  {
    return;
  }
  
  {
    MaskDataBlock::lock_type volume_lock( this->mask_data_block_->get_mutex() );
    CopyCachedDataBack( this, &this->private_->cache_[ 0 ] );
    this->mask_data_block_->increase_generation();
  }
  
  this->private_->cache_.resize( 0 );
  this->private_->using_cache_ = false;

  lock.unlock();

  this->mask_data_block_->mask_updated_signal_();
}

bool MaskVolumeSlice::get_mask_at( size_t i, size_t j ) const
{
  lock_type lock( this->get_mutex() );
  return this->mask_data_block_->get_mask_at( this->to_index( i, j ) );
}

void MaskVolumeSlice::set_mask_at( size_t i, size_t j )
{
  lock_type lock( this->get_mutex() );
  this->mask_data_block_->set_mask_at( this->to_index( i, j ) );
}

void MaskVolumeSlice::clear_mask_at( size_t i, size_t j )
{
  lock_type lock( this->get_mutex() );
  this->mask_data_block_->clear_mask_at( this->to_index( i, j ) );
}

void MaskVolumeSlice::copy_slice_data( std::vector< unsigned char >& buffer, bool invert ) const
{
  lock_type lock( this->get_mutex() );

  buffer.resize( this->nx() * this->ny() );
  {
    MaskDataBlock::shared_lock_type volume_lock( 
      this->mask_data_block_->get_mutex() );
    CopyMaskData( this, &buffer[ 0 ], invert );
  }
}

void MaskVolumeSlice::copy_slice_data( unsigned char* buffer, bool invert ) const
{
  assert( buffer != 0 );
  lock_type lock( this->get_mutex() );

  {
    MaskDataBlock::shared_lock_type volume_lock( 
      this->mask_data_block_->get_mutex() );
    CopyMaskData( this, buffer, invert );
  }
}

void MaskVolumeSlice::set_slice_data( const unsigned char* buffer, bool trigger_update )
{
  {
    lock_type lock( this->get_mutex() );

    {
      MaskDataBlock::lock_type volume_lock( this->mask_data_block_->get_mutex() );
      CopyCachedDataBack( this, buffer );
      if ( trigger_update )
      {
        this->mask_data_block_->increase_generation();
      }
    }
  }

  if ( trigger_update )
  {
    this->mask_data_block_->mask_updated_signal_();
  }
}

} // end namespace Core
