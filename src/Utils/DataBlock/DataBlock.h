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

#ifndef UTILS_DATABLOCK_DATABLOCK_H
#define UTILS_DATABLOCK_DATABLOCK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/function.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>

#include <Utils/DataBlock/DataType.h>

namespace Utils
{

// CLASS DataBlock
// This class is an abstract representation of a block of volume data in
// memory. It stores the pointer to where the data is located as well as 
// its dimensions, and the type of the data

// NOTE: This is a base class that does not do any memory allocation, use one
// of the derived classes to generate a datablock. The implementation in this
// class is just the common access to the data.

// Forward Declaration
class DataBlock;
typedef boost::shared_ptr< DataBlock > DataBlockHandle;

// Class definition
class DataBlock : public boost::noncopyable
{

  // -- typedefs --
public:
  // Lock types
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  // -- Constructor/destructor --
public:
  DataBlock();
  virtual ~DataBlock();

  // -- Access properties of data block --
public:

  // NX, NY, NZ, SIZE
  // The dimensions of the datablock
  size_t nx() const
  {
    return nx_;
  }
  size_t ny() const
  {
    return ny_;
  }
  size_t nz() const
  {
    return nz_;
  }
  size_t size() const
  {
    return nx_ * ny_ * nz_;
  }

  inline size_t to_index( size_t x, size_t y, size_t z ) const
  {
    assert( x < this->nx_ && y < this->ny_ && z < this->nz_ );
    return z * this->nx_ * this->ny_ + y * this->nx_ + x;
  }

  // TYPE
  // The type of the data
  DataType type() const
  {
    return data_type_;
  }

  // DATA:
  // Pointer to the block of data
  void* data()
  {
    return data_;
  }

  inline float* float_data()
  {
    if ( this->data_type_ == DataType::FLOAT_E )
    {
      return reinterpret_cast<float*>( this->data_ );
    }
    return 0;
  }

  inline unsigned char* uchar_data()
  {
    if ( this->data_type_ == DataType::UCHAR_E )
    {
      return reinterpret_cast<unsigned char*>( this->data_ );
    }
    return 0;
  }

  double get_data_at( size_t x, size_t y, size_t z ) const
  {
    return this->get_data_at( this->to_index( x, y, z ) );
  }

  double get_data_at( size_t index ) const
  {
    return this->get_data_func_( index );
  }

  void set_data_at( size_t x, size_t y, size_t z, double value )
  {
    this->set_data_at( this->to_index( x, y, z ), value );
  }

  void set_data_at( size_t index, double value )
  {
    this->set_data_func_( index, value );
  }

  // -- Pointer to where the data is stored
protected:

  // SET_NX, SET_NY, SET_NZ
  // Set the dimensions of the datablock
  void set_nx( size_t nx )
  {
    nx_ = nx;
  }
  void set_ny( size_t ny )
  {
    ny_ = ny;
  }
  void set_nz( size_t nz )
  {
    nz_ = nz;
  }

  // SET_TYPE
  // Set the type of the data
  void set_type( DataType type );

  // SET_DATA
  // Set the data pointer of the data
  void set_data( void* data )
  {
    data_ = data;
  }

  // -- Locking of the datablock --
public:

  // GET_MUTEX:
  // Get the mutex that locks the datablock
  mutex_type& get_mutex()
  {
    return mutex_;
  }

  // -- Signals and slots --
public:
  // DATA_CHANGED_SIGNAL
  // Triggered when data has been changed
  boost::signals2::signal<void ()> data_changed_signal_;

  // -- internals of the DataBlock --
private:

  // Access the data at the specified index, and return the value in a void pointer.
  // NOTE: This will only work for integer types
  template<class DATA_TYPE>
  inline DATA_TYPE internal_get_data( size_t index ) const
  {
    DATA_TYPE* data_ptr = reinterpret_cast<DATA_TYPE*>( this->data_ );
    return data_ptr[ index ];
  }

  template<class DATA_TYPE>
  inline void internal_set_data( size_t index, double value )
  {
    DATA_TYPE* data_ptr = reinterpret_cast<DATA_TYPE*>( this->data_ );
    data_ptr[ index ] = static_cast<DATA_TYPE>( value );
  }

  // The dimensions of the datablock
  size_t nx_;
  size_t ny_;
  size_t nz_;

  // The type of the data in this data block
  DataType data_type_;

  // The mutex that protects the data
  // As data is shared between the renderer and the application threads, we
  // need locking.
  mutex_type mutex_;

  // Pointer to the data
  void* data_;

  boost::function<double (size_t)> get_data_func_;
  boost::function<void (size_t, double)> set_data_func_;

};

} // end namespace Utils

#endif
