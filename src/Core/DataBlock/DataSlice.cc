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
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/DataSlice.h>

namespace Core
{

DataSlice::DataSlice( DataBlockHandle slice, SliceType type, index_type index ) :
  slice_( slice ),
  type_( type ),
  index_( index )
{
}

DataSlice::~DataSlice()
{
}

size_t DataSlice::get_nx() const
{
  if ( this->slice_ ) return this->slice_->get_nx();
  return 0;
}
  
size_t  DataSlice::get_ny() const
{
  if ( this->slice_ ) return this->slice_->get_ny();
  return 0;
}

size_t  DataSlice::get_nz() const
{
  if ( this->slice_ ) return this->slice_->get_nz();
  return 0;
}
  
size_t DataSlice::get_size() const
{
  if ( this->slice_ ) return this->slice_->get_size();
  return 0;
}

size_t DataSlice::get_elem_size() const
{
  if ( this->slice_ ) return this->slice_->get_elem_size();
  return 0;
}
  
size_t DataSlice::get_byte_size() const
{
  if ( this->slice_ ) return this->slice_->get_byte_size();
  return 0;
}

DataType DataSlice::get_data_type() const
{
  if ( this->slice_ ) return this->slice_->get_data_type();
  return DataType::UNKNOWN_E;
}

SliceType DataSlice::get_slice_type() const
{
  return this->type_;
} 

DataSlice::generation_type DataSlice::get_generation() const
{
  if ( this->slice_ ) return this->slice_->get_generation();
  return -1;
}
  
DataBlockHandle DataSlice::get_data_block() const
{
  return this->slice_;
}

void* DataSlice::get_data() const
{
  if ( this->slice_ ) return this->slice_->get_data();
  return 0;
}
  
DataSlice::index_type DataSlice::get_index() const
{
  return this->index_;
} 

void DataSlice::set_index( DataSlice::index_type index)
{
  this->index_ = index;
} 
  
bool DataSlice::Duplicate( const DataSliceHandle& src_data_slice, DataSliceHandle& dst_data_slice )
{
  Core::DataBlockHandle dst_slice;
  Core::DataBlock::Duplicate( src_data_slice->slice_, dst_slice );
  dst_data_slice = Core::DataSliceHandle( new Core::DataSlice( dst_slice, src_data_slice->type_,
    src_data_slice->index_ ) );
  
  return (bool)dst_data_slice;
} 
  
} // end namespace Core
