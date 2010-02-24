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

#ifndef UTILS_DATABLOCK_SHAREDDATABLOCKMANAGER_H
#define UTILS_DATABLOCK_SHAREDDATABLOCKMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <vector>
#include <bitset>

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

// Utils includes
#include <Utils/Singleton/Singleton.h>
#include <Utils/DataBlock/DataBlock.h>
#include <Utils/DataBlock/StdDataBlock.h>
#include <Utils/DataBlock/MaskDataBlock.h>

namespace Utils {

class MaskDataBlockEntry {
  public:
    MaskDataBlockEntry(DataBlockHandle data_block) :
      data_block_(data_block),
      data_masks_(8)
    {}
  
    // The datablock that holds the masks
    DataBlockHandle data_block_;
    
    // Accounting which bits are used
    std::bitset<8> bits_used_;
    
    // Pointers to th MaskDataBlocks that represent these bitplanes
    std::vector<MaskDataBlockWeakHandle> data_masks_;
};

// CLASS SharedDataBlockManager

// Forward Declaration
class MaskDataBlockManager;

// Class definition
class MaskDataBlockManager : public boost::noncopyable {

// -- typedefs --
  public:
    // Lock types
    typedef boost::recursive_mutex          mutex_type;
    typedef boost::unique_lock<mutex_type>  lock_type;

// -- Constructor/destructor --
  public:
    MaskDataBlockManager();    
    virtual ~MaskDataBlockManager();

// -- Create a new mask data block --
  public:
    
    // CREATE:
    // Create a new mask layer
    bool create(size_t nx, size_t ny, size_t nz, MaskDataBlockHandle& mask);

    // COMPACT:
    // Compact the masks into less memory if possible by moving them around
    // to compact the space required.
    bool compact();
    
    
// -- MaskDataBlock callbacks --
  protected:
    friend class MaskDataBlock;
    
    // RELEASE:
    // Function that is called by the destructor of the MaskDataBlock to
    // inform that a bitplane can be reused or that a DataBlock can be 
    // released 
    void release(DataBlockHandle& datablock, unsigned int mask_bit);

// -- Locking of the datablock --
  public:
  
    // LOCK:
    // Lock the datablock
    void lock() { mutex_.lock(); }
    
    // UNLOCK:
    // Unlock the datablock
    void unlock() { mutex_.unlock(); }
    
    // GETMUTEX:
    // Get the mutex that locks the datablock
    mutex_type& get_mutex() { return mutex_;}

// -- internals of the SharedDataBlockManager --
  private:
    // List that maintains a list of which bits are used in 
    typedef std::vector<MaskDataBlockEntry> mask_list_type;
    mask_list_type mask_list_;
    
    // Mutex that protects the list
    mutex_type mutex_;
   
// -- Singleton interface --
  public:
    
    // Instance:
    // Get the singleton interface
    static MaskDataBlockManager* Instance() { return instance_.instance(); }

  private:
    // Internals of the singleton
    static Singleton<MaskDataBlockManager> instance_;
        
};

} // end namespace Utils

#endif
