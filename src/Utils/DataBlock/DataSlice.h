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

#ifndef UTILS_DATABLOCK_DATASLICE_H
#define UTILS_DATABLOCK_DATASLICE_H

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <Utils/Core/EnumClass.h>
#include <Utils/DataBlock/DataBlock.h>

namespace Utils
{

SCI_ENUM_CLASS
(
  SliceType,
  AXIAL_E = 0,
  CORONAL_E = 1,
  SAGITTAL_E = 2
)

// Forward Declaration
class DataSlice;
typedef boost::shared_ptr< DataSlice > DataSliceHandle;

// CLASS DataSlice
// This is a helper class for accessing data within a slice of a DataBlock

class DataSlice : public boost::noncopyable
{
public:

  DataSlice( const DataBlockHandle& data_block, 
    SliceType slice_type = SliceType::AXIAL_E, size_t slice_num = 0 );
  ~DataSlice();

  inline void set_slice_type( SliceType type )
  {
    if ( this->slice_type_ != type )
    {
      this->slice_type_ = type;
      this->slice_changed_ = true;
    }
  }

  inline void set_slice_number( size_t slice_num )
  {
    if ( this->slice_number_ != slice_num )
    {
      this->slice_number_ = slice_num;
      this->slice_changed_ = true;
    }
  }

  template<class DATA_TYPE>
  inline DATA_TYPE get_data_at ( size_t i, size_t j ) const
  {
    return this->data_block_->get_data_at<DATA_TYPE>( 
      this->index_func_[ this->slice_type_ ]( i, j ) );
  }

  template<class DATA_TYPE>
  inline void set_data_at( size_t i, size_t j, DATA_TYPE value ) 
  {
    this->data_block_->set_data_at( 
      this->index_func_[ this->slice_type_ ]( i, j ), value );
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
    this->slice_changed_ = changed;
  }

public:

  typedef DataBlock::mutex_type mutex_type;
  typedef DataBlock::lock_type lock_type;

  inline void lock()
  {
    this->data_block_->lock();
  }

  inline void unlock()
  {
    this->data_block_->unlock();
  }

  mutex_type& get_mutex()
  {
    return this->data_block_->get_mutex();
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

  DataBlockHandle data_block_;
  SliceType slice_type_;
  size_t slice_number_;
  bool slice_changed_;

};

} // end namespace Utils

#endif