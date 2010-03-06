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

#ifndef UTILS_DATABLOCK_MASKDATASLICE_H
#define UTILS_DATABLOCK_MASKDATASLICE_H

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <Utils/Core/ConnectionHandler.h>
#include <Utils/DataBlock/MaskDataBlock.h>
#include <Utils/DataBlock/DataSlice.h>
#include <Utils/Graphics/Texture.h>

namespace Utils
{

// Forward Declaration
class MaskDataSlice;
typedef boost::shared_ptr< MaskDataSlice > MaskDataSliceHandle;

// CLASS MaskDataSlice
// This is a helper class for accessing data within a slice of a MaskDataBlock

class MaskDataSlice : private ConnectionHandler
{
public:

  MaskDataSlice( const MaskDataBlockHandle& mask_data_block,
    SliceType slice_type = SliceType::AXIAL_E, size_t slice_num = 0 );
  ~MaskDataSlice();

  inline void set_slice_type( SliceType type )
  {
    if ( this->slice_type_ != type )
    {
      this->slice_type_ = type;
      this->data_changed_ = true;
    }
  }

  inline void set_slice_number( size_t slice_num )
  {
    if ( this->slice_number_ != slice_num )
    {
      this->slice_number_ = slice_num;
      this->data_changed_ = true;
    }
  }

  inline unsigned char get_mask_at( size_t i, size_t j ) const
  {
    return this->mask_data_block_->get_mask_at( 
      this->index_func_[ this->slice_type_ ]( i, j ) );
  }

  void set_mask_at( size_t i, size_t j )
  {
    this->mask_data_block_->set_mask_at( 
      this->index_func_[ this->slice_type_ ]( i, j ) );
  }

  void clear_mask_at( size_t i, size_t j )
  {
    this->mask_data_block_->clear_mask_at( 
      this->index_func_[ this->slice_type_ ]( i, j ) );
  }

  inline size_t to_index( size_t i, size_t j ) const
  {
    return this->index_func_[ this->slice_type_ ]( i, j );
  }

  inline size_t width() const
  {
    return this->width_func_[ this->slice_type_ ]();
  }

  inline size_t height() const
  {
    return this->height_func_[ this->slice_type_ ]();
  }

  inline void set_changed( bool changed = true )
  {
    this->data_changed_ = changed;
  }

  void upload_texture();

public:

  typedef MaskDataBlock::mutex_type mutex_type;
  typedef MaskDataBlock::lock_type lock_type;

  inline void lock()
  {
    this->mask_data_block_->lock();
  }

  inline void unlock()
  {
    this->mask_data_block_->unlock();
  }

  inline mutex_type& get_mutex()
  {
    return this->mask_data_block_->get_mutex();
  }

private:

  size_t slice_number() const
  {
    return this->slice_number_;
  }

  // Functions for computing the index of the spcified point in the DataBlock
  boost::function<size_t ( size_t, size_t )> index_func_[3];

  boost::function<size_t ()> width_func_[3];
  boost::function<size_t ()> height_func_[3];

private:

  MaskDataBlockHandle mask_data_block_;
  SliceType slice_type_;
  size_t slice_number_;
  bool data_changed_;
  TextureHandle texture_;

};

} // end namespace Seg3D

#endif