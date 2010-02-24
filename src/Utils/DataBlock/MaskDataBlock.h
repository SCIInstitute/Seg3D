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

#ifndef UTILS_DATABLOCK_SHAREDDATABLOCK_H
#define UTILS_DATABLOCK_SHAREDDATABLOCK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

// Utils includes
#include <Utils/DataBlock/DataBlock.h>

namespace Utils {

// CLASS MaskDataBlock
// This class represents a datablock for masks that are collapsed in several
// bitplanes in a common datablock. It runs through the SharedDataBlockManager
// to allocate a new datablock or share the datablock with another mask.
// When a new datablock is created, it is checked whether a similar mask with
// the same dimensions exists with an unassigned bit and used if possible,
// otherwise a new one is generated.

// Forward Declaration
class MaskDataBlock;
typedef boost::shared_ptr<MaskDataBlock> MaskDataBlockHandle;
typedef boost::weak_ptr<MaskDataBlock> MaskDataBlockWeakHandle;

// Class definition
class MaskDataBlock : public boost::noncopyable {

// -- typedefs --
  public:
    // Lock types
    typedef DataBlock::mutex_type  mutex_type;
    typedef DataBlock::lock_type   lock_type;

// -- Constructor/destructor --
  public:
    MaskDataBlock(DataBlockHandle& data_block, unsigned int mask_bit);    
    virtual ~MaskDataBlock();

// -- Access properties of data block --
  public:
    
    // NX, NY, NZ, SIZE
    // The dimensions of the datablock
    size_t nx() const { return nx_; }
    size_t ny() const { return ny_; }
    size_t nz() const { return nz_; }
    size_t size() const { return nx_*ny_*nz_; }

    // DATA
    // Pointer to the block of data
    unsigned char* data() const 
    { 
      return reinterpret_cast<unsigned char *>(data_block_->data()); 
    }

    // MASK_BIT
    // Get the bit that describes the mask
    unsigned int mask_bit() 
    { 
      return mask_bit_; 
    }

    // DATA_BLOCK
    // Retrieve the pointer to the data block
    // NOTE: This one is needed for loading the textures onto the
    // graphics card. As masks are shared the Texture will be shared
    // hence access to the datablock is needed to see whether this one
    // has already been uploaded
    DataBlockHandle data_block() 
    {
      return data_block_; 
    }

// -- Locking of the datablock --
  public:
  
    // LOCK:
    // Lock the datablock
    void lock() { data_block_->lock(); }
    
    // UNLOCK:
    // Unlock the datablock
    void unlock() { data_block_->unlock(); }
    
    // GETMUTEX:
    // Get the mutex that locks the datablock
    mutex_type& get_mutex() { return data_block_->get_mutex();}

// -- internals of the DataBlock --
  private:
    // The dimensions of the datablock
    size_t nx_;
    size_t ny_;
    size_t nz_;
      
    // The datablock that is shared
    DataBlockHandle data_block_;
  
    // The bit that is used for this mask
    unsigned int mask_bit_;
};

} // end namespace Utils

#endif
