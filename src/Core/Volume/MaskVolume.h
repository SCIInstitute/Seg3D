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

#ifndef CORE_VOLUME_MASKVOLUME_H
#define CORE_VOLUME_MASKVOLUME_H

#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/NrrdData.h>
#include <Core/Volume/DataVolume.h>

namespace Core
{

class MaskVolume;
typedef boost::shared_ptr< MaskVolume > MaskVolumeHandle;
typedef boost::weak_ptr< MaskVolume > MaskVolumeWeakHandle;

class MaskVolume : public Volume
{
public:
  MaskVolume( const GridTransform& grid_transform, const MaskDataBlockHandle& mask_data_block );
  MaskVolume( const GridTransform& grid_transform );
  virtual ~MaskVolume() {}

  // GET_TYPE:
  // Get the type of the data volume
  virtual VolumeType get_type() const;
  
  // MASK_DATA_BLOCK:
  // Get the datablock that contains the mask
  MaskDataBlockHandle mask_data_block() const;

  // GET_MUTEX:
  // Get access to the mutex protecting this MaskVolume
  virtual mutex_type& get_mutex() const
  {
    return this->mask_data_block_->get_mutex();
  }

  // -- functions for creating MaskVolumeHandle's --
public: 

  // CREATEMASKFROMNONZERODATA:
  // Create a mask from the non zero data contained in a datablock
  static bool CreateMaskFromNonZeroData( const DataVolumeHandle data, 
    MaskDataBlockHandle& mask );

  // CREATEMASKFROMBITPLANEDATA:
  // Create a mask from each bitplane in integer data
  static bool CreateMaskFromBitPlaneData( const DataVolumeHandle data, 
    std::vector<MaskDataBlockHandle>& masks );

  // CREATEMASKFROMLABELDATA:
  // Create a mask from each label in integer data
  static bool CreateMaskFromLabelData( const DataVolumeHandle data, 
    std::vector<MaskDataBlockHandle>& masks, bool reuse_data = false );

  // CREATEEMPTYMASK:
  // Create an empty mask with given dimensions.
  static bool CreateEmptyMask( GridTransform grid_transform, MaskVolumeHandle& mask );

private:
  // Handle to where the mask volume is really stored
  MaskDataBlockHandle mask_data_block_;
  
};

} // end namespace Core

#endif