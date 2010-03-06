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

#include <Utils/DataBlock/DataSlice.h>
#include <Utils/Geometry/Point.h>

namespace Utils
{

DataSlice::DataSlice( const DataBlockHandle& data_block, 
           SliceType slice_type, size_t slice_num ) :
  data_block_( data_block ), slice_type_( slice_type ), slice_number_ ( slice_num ),
  data_changed_( true )
{
  this->index_func_[0] = boost::bind( &DataBlock::to_index, this->data_block_, 
    _1, _2, boost::bind( &DataSlice::slice_number, this ) );
  this->index_func_[1] = boost::bind( &DataBlock::to_index, this->data_block_,
    _2, boost::bind( &DataSlice::slice_number, this ), _1 );
  this->index_func_[2] = boost::bind( &DataBlock::to_index, this->data_block_,
    boost::bind( &DataSlice::slice_number, this ), _1, _2 );

  this->width_func_[0] = boost::bind( &DataBlock::nx, this->data_block_ );
  this->width_func_[1] = boost::bind( &DataBlock::nz, this->data_block_ );
  this->width_func_[2] = boost::bind( &DataBlock::ny, this->data_block_ );

  this->height_func_[0] = boost::bind( &DataBlock::ny, this->data_block_ );
  this->height_func_[1] = boost::bind( &DataBlock::nx, this->data_block_ );
  this->height_func_[2] = boost::bind( &DataBlock::nz, this->data_block_ );
}

DataSlice::~DataSlice()
{
}

void DataSlice::upload_texture()
{
  // To be implemented
}

} // end namespace Utils