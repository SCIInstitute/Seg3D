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

#ifndef CORE_DATABLOCK_DATABLOCK_H
#define CORE_DATABLOCK_DATABLOCK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/signals2/signal.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/Utils/Lockable.h>
#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/Histogram.h>
#include <Core/DataBlock/DataBlockGeneration.h>


namespace Core
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
typedef boost::weak_ptr< DataBlock > DataBlockWeakHandle;

// Class definition
class DataBlock : public SharedLockable
{
public:
  // index/size types
#ifdef SCI_64BITS
  typedef unsigned long long  size_type;
  typedef long long     index_type;
#else
  typedef unsigned int    size_type;
  typedef int         index_type;
#endif  

  typedef unsigned long long  generation_type;

  // -- Constructor/destructor --
public:
  DataBlock();  
  virtual ~DataBlock();

  // -- Access properties of data block --
public:

  // GET_NX, GET_NY, GET_NZ, GET_SIZE
  // The dimensions of the datablock
  size_t get_nx() const
  {
    return nx_;
  }
  size_t get_ny() const
  {
    return ny_;
  }
  size_t get_nz() const
  {
    return nz_;
  }
  size_t get_size() const
  {
    return nx_ * ny_ * nz_;
  }

  // TO_INDEX:
  // Compute the real index based on the coordinates in index space
  inline size_t to_index( size_t x, size_t y, size_t z ) const
  {
    assert( x < this->nx_ && y < this->ny_ && z < this->nz_ );
    return z * this->nx_ * this->ny_ + y * this->nx_ + x;
  }

  // TYPE
  // The type of the data
  DataType get_type() const
  {
    return this->data_type_;
  }

  // DATA:
  // Pointer to the block of data
  void* get_data()
  {
    return this->data_;
  }

  // GET_DATA_AT:
  // Get data at a certain location in the data block
  inline double get_data_at( size_t x, size_t y, size_t z ) const
  {
    return get_data_at( this->to_index( x, y, z ) );
  }
  
  // GET_DATA_AT:
  // Get data at a certain index location in the data block
  double get_data_at( size_t index ) const;
  
  // SET_DATA_AT:
  // Set data at a certain location in the data block
  inline void set_data_at( size_t x, size_t y, size_t z, double value )
  {
    set_data_at( this->to_index( x, y, z ), value );
  }

  // SET_DATA_AT:
  // Set data at a certain index location in the data block
  void set_data_at( size_t index, double value );

  // GET_MAX:
  // Get the maximum value of the data
  // NOTE: update_histogram needs to be called to ensure the data is accurate
  double get_max() const;

  // GET_MIN:
  // Get the minimum value of the data
  // NOTE: update_histogram needs to be called to ensure the data is accurate
  double get_min() const;

  // GET_HISTOGRAM:
  // Get the histogram of the underlying data
  const Histogram& get_histogram() const;

  // UPDATE_HISTOGRAM:
  // Recompute the histogram. This needs to be triggered each time the data is updated
  void update_histogram();
  
  // GET_GENERATION:
  // Get the current generation number of the data volume.
  generation_type get_generation() const;
  
  // INCREMENT_GENERATION:
  // Increment the generation number to a new unique number.
  void increment_generation();

private:
  friend class DataBlockManager;
  void set_generation( generation_type generation );
  
protected:

  // SET_NX, SET_NY, SET_NZ
  // Set the dimensions of the datablock
  void set_nx( size_t nx );
  void set_ny( size_t ny );
  void set_nz( size_t nz );

  // SET_TYPE
  // Set the type of the data
  void set_type( DataType type );

  // SET_DATA
  // Set the data pointer of the data
  void set_data( void* data );

  // SET_HISTOGRAM:
  // Set the histogram of the dataset
  void set_histogram( const Histogram& histogram );

  // -- Signals and slots --
public:
  // DATA_CHANGED_SIGNAL
  // Triggered when data has been changed
  boost::signals2::signal<void ()> data_changed_signal_;

  // -- internals of the DataBlock --
private:

  // The dimensions of the datablock
  size_t nx_;
  size_t ny_;
  size_t nz_;

  // The type of the data in this data block
  DataType data_type_;

  // Pointer to the data
  void* data_;

  // Histogram information for this data block
  Histogram histogram_;
  
  // Generation number
  generation_type generation_;

public:

  // CONVERTDATATYPE:
  // Convert the data to a specific format
  static bool ConvertDataType( const DataBlockHandle& src_data_block, 
    DataBlockHandle& dst_data_block, DataType new_data_type );
    
  // PERMUTEDATA:
  // Reorder the data by shuffling the axis, the permutation is a vector of three components
  // that specify -1, 1 , and -2, 2, and -3, 3 for each of the axis, where the negative number
  // indicates an inverted axis.
  static bool PermuteData( const DataBlockHandle& src_data_block, 
    DataBlockHandle& dst_data_block, std::vector<int> permutation );
    
  // QUANTIZEDATA:
  // Quantize the data based on its min and max value
  // NOTE: This function assumes that the histogram has been actualized
  static bool QuantizeData( const DataBlockHandle& src_data_block, 
    DataBlockHandle& dst_data_block, DataType new_data_type );

  // CLONE:
  static bool Clone( const DataBlockHandle& src_data_block, 
    DataBlockHandle& dst_data_block ); 
  
  // SETGENERATIONCOUNT:
  // Set the counter for the generation number of datablocks to a new value.
  // NOTE: This function should only be called by the project loader
  static void SetGenerationCount( generation_type generation_count );
  
  // GETGENERATIONCOUNT:
  // Get the latest generation count
  static generation_type GetGenerationCount();
  
};

} // end namespace Core

#endif
