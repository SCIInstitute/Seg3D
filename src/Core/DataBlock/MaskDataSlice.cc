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

#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/MaskDataSlice.h>

namespace Core
{

MaskDataSlice::MaskDataSlice( MaskDataBlockHandle slice, SliceType type, index_type index ) :
  slice_( slice ),
  type_( type ),
  index_( index )
{
}

MaskDataSlice::~MaskDataSlice()
{
}


size_t MaskDataSlice::get_nx() const
{
  if ( this->slice_ ) return this->slice_->get_nx();
  return 0;
}
  
size_t MaskDataSlice::get_ny() const
{
  if ( this->slice_ ) return this->slice_->get_ny();
  return 0;
}

size_t MaskDataSlice::get_nz() const
{
  if ( this->slice_ ) return this->slice_->get_nz();
  return 0;
}
  
size_t MaskDataSlice::get_size() const
{
  if ( this->slice_ ) return this->slice_->get_size();
  return 0;
}

size_t MaskDataSlice::get_byte_size() const
{
  if ( this->slice_ ) return this->slice_->get_byte_size();
  return 0;
}

SliceType MaskDataSlice::get_slice_type() const
{
  return this->type_;
} 

MaskDataSlice::generation_type MaskDataSlice::get_generation() const
{
  if ( this->slice_ ) return this->slice_->get_generation();
  return -1;
}
  
MaskDataBlockHandle MaskDataSlice::get_mask_data_block() const
{
  return this->slice_;
}
  
MaskDataSlice::index_type MaskDataSlice::get_index() const
{
  return this->index_;
} 
  
} // end namespace Core
