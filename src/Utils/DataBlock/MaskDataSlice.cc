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

#include <Utils/DataBlock/MaskDataSlice.h>

namespace Utils
{

MaskDataSlice::MaskDataSlice( const MaskDataBlockHandle &mask_data_block, 
                          SliceType slice_type, size_t slice_num ) :
  mask_data_block_( mask_data_block ), 
  slice_type_( slice_type ), slice_number_ ( slice_num ),
  slice_changed_( true ), size_changed_( true )
{
  this->index_func_[0] = boost::bind( &MaskDataBlock::to_index, this->mask_data_block_, 
    _1, _2, boost::bind( &MaskDataSlice::slice_number, this ) );
  this->index_func_[1] = boost::bind( &MaskDataBlock::to_index, this->mask_data_block_,
    _2, boost::bind( &MaskDataSlice::slice_number, this ), _1 );
  this->index_func_[2] = boost::bind( &MaskDataBlock::to_index, this->mask_data_block_,
    boost::bind( &MaskDataSlice::slice_number, this ), _1, _2 );

  this->width_func_[0] = boost::bind( &MaskDataBlock::nx, this->mask_data_block_ );
  this->width_func_[1] = boost::bind( &MaskDataBlock::nz, this->mask_data_block_ );
  this->width_func_[2] = boost::bind( &MaskDataBlock::ny, this->mask_data_block_ );

  this->height_func_[0] = boost::bind( &MaskDataBlock::ny, this->mask_data_block_ );
  this->height_func_[1] = boost::bind( &MaskDataBlock::nx, this->mask_data_block_ );
  this->height_func_[2] = boost::bind( &MaskDataBlock::nz, this->mask_data_block_ );

  this->add_connection( this->mask_data_block_->mask_updated_signal_.connect( 
    boost::bind(&MaskDataSlice::set_changed, this, true) ) );

  this->width_ = this->width_func_[ this->slice_type_ ]();
  this->height_ = this->height_func_[ this->slice_type_ ]();
}

MaskDataSlice::~MaskDataSlice()
{
  this->disconnect_all();
}

void MaskDataSlice::upload_texture()
{
  if ( !this->texture_.get() )
  {
    this->texture_ = Texture2DHandle( new Texture2D );
    // It doesn't make sense to use linear interpolation for mask texture
    this->texture_->set_mag_filter( GL_NEAREST );
    this->texture_->set_min_filter( GL_NEAREST );
  }

  if ( this->slice_changed_ )
  {
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
    for ( size_t j = 0; j < this->height_; j++ )
    {
      for ( size_t i = 0; i < this->width_; i++ )
      {
        size_t index = this->to_index( i, j );
        buffer[ j * this->width_ + i ] = this->mask_data_block_->get_mask_at( index );
      }
    }
    
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
}

} // end namespace Utils