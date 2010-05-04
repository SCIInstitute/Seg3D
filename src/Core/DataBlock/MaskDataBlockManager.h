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

#ifndef CORE_DATABLOCK_SHAREDDATABLOCKMANAGER_H
#define CORE_DATABLOCK_SHAREDDATABLOCKMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <vector>

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>

#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/MaskDataBlock.h>

namespace Core
{

// CLASS SharedDataBlockManager

// Forward Declaration
class MaskDataBlockManager;
class MaskDataBlockManagerInternal;
typedef boost::shared_ptr<MaskDataBlockManagerInternal> MaskDataBlockManagerInternalHandle;

// Class definition
class MaskDataBlockManager : public RecursiveLockable
{
  CORE_SINGLETON( MaskDataBlockManager );

  // -- Constructor/destructor --
private:
  MaskDataBlockManager();
  virtual ~MaskDataBlockManager();

  // -- Create a new mask data block --
public:

  // CREATE:
  // Create a new mask layer
  bool create( size_t nx, size_t ny, size_t nz, MaskDataBlockHandle& mask );

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
  void release( DataBlockHandle& datablock, unsigned int mask_bit );

private:
   MaskDataBlockManagerInternalHandle private_;

  // -- functions for creating MaskDataBlocks --
public: 
  
  // CREATEMASKFROMNONZERODATA:
  // Create a mask from the non zero data contained in a datablock
  static bool CreateMaskFromNonZeroData( const DataBlockHandle data, 
    MaskDataBlockHandle& mask );

  // CREATEMASKFROMBITPLANEDATA:
  // Create a mask from each bitplane in integer data
  static bool CreateMaskFromBitPlaneData( const DataBlockHandle data, 
    std::vector<MaskDataBlockHandle>& mask );

  // CREATEMASKFROMLABELDATA:
  // Create a mask from each label in integer data
  static bool CreateMaskFromLabelData( const DataBlockHandle data, 
    std::vector<MaskDataBlockHandle>& mask, bool reuse_data = false );

  // CREATEEMPTYMASK:
  // Create an empty mask with given dimensions.
  static bool CreateEmptyMask( size_t nx, size_t ny, size_t nz, MaskDataBlockHandle& mask );
};

} // end namespace Core

#endif
