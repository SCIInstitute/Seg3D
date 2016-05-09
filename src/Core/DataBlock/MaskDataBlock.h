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

#ifndef CORE_DATABLOCK_MASKDATABLOCK_H
#define CORE_DATABLOCK_MASKDATABLOCK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/DataBlock/MaskDataBlockFWD.h>
#include <Core/DataBlock/MaskDataSlice.h>
#include <Core/DataBlock/DataBlock.h>

namespace Core
{

// CLASS MaskDataBlock
/// This class represents a datablock for masks that are collapsed in several
/// bitplanes in a common datablock. It runs through the SharedDataBlockManager
/// to allocate a new datablock or share the datablock with another mask.
/// When a new datablock is created, it is checked whether a similar mask with
/// the same dimensions exists with an unassigned bit and used if possible,
/// otherwise a new one is generated.
class MaskDataBlock : public boost::noncopyable, 
  public boost::enable_shared_from_this< MaskDataBlock >
{

  // -- typedefs --
public:
  // Lock types
  typedef DataBlock::mutex_type mutex_type;
  typedef DataBlock::lock_type lock_type;
  typedef DataBlock::shared_lock_type shared_lock_type;

  typedef DataBlock::index_type index_type;

  // -- Constructor/destructor --
public:
  MaskDataBlock( DataBlockHandle data_block, unsigned int mask_bit );
  virtual ~MaskDataBlock();

  // -- Access properties of data block --
public:

  // GET_NX, GET_NY, GET_NZ, GET_SIZE
  // The dimensions of the datablock
  inline size_t get_nx() const
  {
    return this->nx_;
  }
  
  inline size_t get_ny() const
  {
    return this->ny_;
  }
  
  inline size_t get_nz() const
  {
    return this->nz_;
  }
  
  inline size_t get_size() const
  {
    return this->nx_ * this->ny_ * this->nz_;
  }

  inline size_t get_byte_size() const
  {
    return ( this->get_size() >> 3 ) + 1;
  }

  inline size_t to_index( size_t x, size_t y, size_t z ) const
  {
    return this->data_block_->to_index( x, y, z );
  }

  // DATA
  /// Pointer to the block of data
  inline unsigned char* get_mask_data()
  {
    return  this->data_;
  }

  // GET_MASK_BIT:
  /// Get the bit that describes the mask
  inline unsigned int get_mask_bit()
  {
    return this->mask_bit_;
  }

  // GET_MASK_VALUE
  /// Get the value at which the mask is stored
  inline unsigned char get_mask_value()
  {
    return this->mask_value_;
  }

  // GET_DATA_BLOCK
  /// Retrieve the pointer to the data block
  /// NOTE: This one is needed for loading the textures onto the
  /// graphics card. As masks are shared the Texture will be shared
  /// hence access to the datablock is needed to see whether this one
  /// has already been uploaded
  DataBlockHandle get_data_block();

  // GET_GENERATION:
  /// Get the current generation number of the data volume.
  DataBlock::generation_type get_generation() const;

  // INCREASE_GENERATION:
  /// Increase the generation number to a new unique number.
  void increase_generation();

  // GET_MASK_AT:
  /// Get the mask value at a certain coordinate
  inline bool get_mask_at( size_t x, size_t y, size_t z ) const
  {
    return this->get_mask_at( this->to_index( x, y, z ) );
  }

  // GET_MASK_AT:
  /// Get the mask value at a certain index
  inline bool get_mask_at( size_t index ) const
  {
    // range check
    // TODO: exception would be clearer, but would there be a performance impact?
    if (index >= get_size())
    {
        return false;
    }

    return ( this->data_[ index ] & this->mask_value_ ) != 0;
  }

  // SET_MASK_AT:
  /// Set the mask value at a certain coordinate
  inline void set_mask_at( size_t x, size_t y, size_t z )
  {
    this->set_mask_at( this->to_index( x, y, z ) );
  }
  
  // SET_MASK_AT:
  /// Set the mask value at a certain index
  inline void set_mask_at( size_t index )
  {
    this->data_[ index ] |= this->mask_value_;
  }
  
  // Clear_MASK_AT:
  /// Clear the mask value at a certain index
  inline void clear_mask_at( size_t x, size_t y, size_t z )
  {
    this->clear_mask_at( this->to_index( x, y, z ) );
  }

  inline void clear_mask_at( size_t index )
  {
    this->data_[ index ] &= this->not_mask_value_;
  }

// -- Locking of the datablock --
public:

  // GET_MUTEX:
  /// Get the mutex that locks the datablock
  mutex_type& get_mutex() const
  { 
    return data_block_->get_mutex();
  }

  // -- Signals and slots --
public:

  // MASK_UPDATED_SIGNAL
  /// Triggered when mask has been update
  //
  /// NOTE: This signal is never triggered inside the class. 
  /// Any object that makes change to the mask data is responsible for triggering
  /// this signal after modification is done.
  boost::signals2::signal<void ()> mask_updated_signal_;

  // -- extracting slices and inserting slices
public:
  // INSERT_SLICE:
  /// Insert slice into the datablock
  bool insert_slice( const MaskDataSliceHandle slice );

  // EXTRACT_SLICE:
  /// Extract a slice from the datablock
  bool extract_slice( SliceType type, index_type index, MaskDataSliceHandle& slice  );

  // -- internals of the DataBlock --
private:
  /// The dimensions of the datablock
  size_t nx_;
  size_t ny_;
  size_t nz_;

  /// The datablock that is shared
  DataBlockHandle data_block_;

  /// The bit that is used for this mask
  const unsigned int mask_bit_;

  /// Values that have the maskbit set or all the other bits
  const unsigned char mask_value_;
  const unsigned char not_mask_value_;

  /// Cached data pointer of the underlying DataBlock
  unsigned char* data_;

};

} // end namespace Core

#endif
