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

#ifndef CORE_VOLUME_MASKVOLUME_H
#define CORE_VOLUME_MASKVOLUME_H

#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/Volume/DataVolume.h>

namespace Core
{

class MaskVolume;
typedef boost::shared_ptr< MaskVolume > MaskVolumeHandle;
typedef boost::weak_ptr< MaskVolume > MaskVolumeWeakHandle;


class MaskVolume : public Volume
{
  // -- constructor / destructor --
public:
  MaskVolume( const GridTransform& grid_transform, const MaskDataBlockHandle& mask_data_block );
  virtual ~MaskVolume();

public:
  // GET_TYPE:
  /// Get the type of the data volume
  virtual VolumeType get_type() const;
  
  // GET_MIN:
  /// Get the minimum value
  virtual double get_min() const;

  // GET_MAX:
  /// Get the maximum value
  virtual double get_max() const;

  // GET_CUM_VALUE:
  /// Get the value at a specific point in the histogram
  virtual double get_cum_value( double fraction ) const;

  // IS_VALID:
  /// Check whether the volume has a valid data block
  virtual bool is_valid() const;

  // MASK_DATA_BLOCK:
  /// Get the datablock that contains the mask
  MaskDataBlockHandle get_mask_data_block() const;

  // GET_MUTEX:
  /// Get access to the mutex protecting this MaskVolume
  virtual mutex_type& get_mutex();
  
  // GET_GENERATION:
  /// Get the  generation number of the data volume
  virtual DataBlock::generation_type get_generation() const;

  // REGISTER_DATA:
  /// Register the underlying data with the DataBlockManager.
  virtual DataBlock::generation_type register_data( DataBlock::generation_type generation = -1 );

  // UNREGISTER_DATA:
  /// Unregister the underlying data with DataBlockManager.
  virtual void unregister_data();

  // GET_BYTE_SIZE:
  /// Get the size of the data in bytes
  virtual size_t get_byte_size() const;
  
  // -- slice handling --
public: 
  // INSERT_SLICE:
  /// Insert a slice into the volume
  bool insert_slice( const MaskDataSliceHandle slice );
  
  // EXTRACT_SLICE:
  /// Extract a slice from the volume
  bool extract_slice( SliceType type, MaskDataBlock::index_type index, MaskDataSliceHandle& slice );
    
  // -- functions for creating MaskVolumes --
public: 

  // CREATEEMPTYMASK:
  /// Create an empty mask with given dimensions.
  static bool CreateEmptyMask( GridTransform grid_transform, MaskVolumeHandle& mask );

  // CREATEINVALIDMASK:
  /// Create a mask with given dimensions, but no data container associated with it.
  static bool CreateInvalidMask( GridTransform grid_transform, MaskVolumeHandle& mask );
  
  // DUPLICATEMASK:
  /// Duplicate the mask volume
  static bool DuplicateMask( const MaskVolumeHandle& src_mask, MaskVolumeHandle& dst_mask );

private:
  /// Handle to where the mask volume is really stored
  MaskDataBlockHandle mask_data_block_;
  
  /// Mutex for a volume without a data block associated with it
  /// NOTE: This is use to set up a new layer that is still constructing its data
  mutex_type invalid_mutex_;
  
};

} // end namespace Core

#endif
