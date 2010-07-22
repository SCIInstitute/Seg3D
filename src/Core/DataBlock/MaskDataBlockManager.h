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
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>
#include <Core/Geometry/GridTransform.h>
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
  bool create( const GridTransform& grid_transform, MaskDataBlockHandle& mask );

  bool create( DataBlock::generation_type generation, unsigned int bit, 
    GridTransform& grid_transform, MaskDataBlockHandle& mask );

  void register_data_block( DataBlockHandle data_block, const GridTransform& grid_transform );

  bool save_data_blocks( boost::filesystem::path data_save_path );

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

};

} // end namespace Core

#endif
