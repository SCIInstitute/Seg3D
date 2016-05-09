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

// Application includes
#include <Application/Layer/LayerFWD.h>

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
  /// Create a new mask layer
  bool create( GridTransform grid_transform, MaskDataBlockHandle& mask );

  // CREATE:
  /// Create a new mask layer with a given generation number and bit
  bool create( DataBlock::generation_type generation, unsigned int bit, 
    GridTransform& grid_transform, MaskDataBlockHandle& mask );

  // CLEAR:
  /// Remove all the MaskDataBlock.
  void clear();

  // TODO:
  // Do these need to be private ?
  // --JS
  void register_data_block( DataBlockHandle data_block, const GridTransform& grid_transform );
    
  // COMPACT:
  /// Compact the masks into less memory if possible by moving them around
  /// to compact the space required.
  bool compact();

  // -- MaskDataBlock callbacks --
protected:
  friend class MaskDataBlock;

  // RELEASE:
  /// Function that is called by the destructor of the MaskDataBlock to
  /// inform that a bitplane can be reused or that a DataBlock can be
  /// released
  void release( DataBlockHandle& datablock, unsigned int mask_bit );

  // -- internals of this class --
private:
   MaskDataBlockManagerInternalHandle private_;
   
   // -- static functions --
public:

  // CREATE:
  /// Create a new MaskDataBlock with a given GridTransform
  static bool Create( GridTransform grid_transform, MaskDataBlockHandle& mask );

  // INSCRIBE:
  /// Inscribe a mask into a datablock with a certain label value. Both mask and data need
  /// to have been created before.
  static bool Inscribe( MaskDataBlockHandle mask, DataBlockHandle data, double label, 
    bool invert = false );

  // CONVERT:
  /// Convert a MaskDataBlock into a DataBlock
  static bool Convert( MaskDataBlockHandle mask, DataBlockHandle& data, 
    DataType data_type, bool invert = false );  

  // CONVERTLABEL:
  /// Convert a MaskDataBlock into a DataBlock
  static bool ConvertLabel( MaskDataBlockHandle mask, DataBlockHandle& data, 
    DataType data_type, double label = 1.0 ); 

  // CONVERT:
  /// Convert a DataBlock into a MaskDataBlock
  static bool Convert( DataBlockHandle data, GridTransform grid_transform, 
    MaskDataBlockHandle& mask, bool invert = false );

  // CONVERTLARGERTHAN:
  /// Convert a DataBlock into a MaskDataBlock
  /// Only values larger than zero are considered part of the mask.
  static bool ConvertLargerThan( DataBlockHandle data, GridTransform grid_transform, 
    MaskDataBlockHandle& mask, bool invert = false );

  // CONVERTLABEL:
  /// Convert a DataBlock into a MaskDataBlock by checking for a certain value
  static bool ConvertLabel( DataBlockHandle data, GridTransform grid_transform, 
    MaskDataBlockHandle& mask, double label );
  

  // CREATEMASKFROMNONZERODATA:
  /// Create a mask from the non zero data contained in a datablock
  static bool CreateMaskFromNonZeroData( const DataBlockHandle& data, 
    const GridTransform& grid_transform, MaskDataBlockHandle& mask );

  // CREATEMASKFROMBITPLANEDATA:
  /// Create a mask from each bitplane in integer data
  static bool CreateMaskFromBitPlaneData( const DataBlockHandle& data, 
    const GridTransform& grid_transform, std::vector<MaskDataBlockHandle>& masks );

  // CREATEMASKFROMLABELDATA:
  // Create a mask from each label in integer data
  static bool CreateMaskFromLabelData( const DataBlockHandle& data, 
    const GridTransform& grid_transform, std::vector<MaskDataBlockHandle>& masks );

  // DUPLICATE:
  /// Duplicate a MaskDataBlock into a DataBlock
  static bool Duplicate( MaskDataBlockHandle src_mask_data_block, 
    const GridTransform& grid_transform, MaskDataBlockHandle& dst_mask_data_block );
};

} // end namespace Core

#endif
