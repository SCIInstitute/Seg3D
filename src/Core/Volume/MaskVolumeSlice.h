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

#ifndef CORE_VOLUME_MASKVOLUMESLICE_H
#define CORE_VOLUME_MASKVOLUMESLICE_H

#include <Core/Volume/MaskVolume.h>
#include <Core/Volume/VolumeSlice.h>

namespace Core
{

class MaskVolumeSlice;
class MaskVolumeSlicePrivate;
typedef boost::shared_ptr< MaskVolumeSlice > MaskVolumeSliceHandle;
typedef boost::shared_ptr< MaskVolumeSlicePrivate > MaskVolumeSlicePrivateHandle;

class MaskVolumeSlice : public VolumeSlice
{
public:
  MaskVolumeSlice( const MaskVolumeHandle& mask_volume, 
    VolumeSliceType type = VolumeSliceType::AXIAL_E, size_t slice_num = 0 );

  // COPY CONSTRUCTOR:
  /// NOTE: This is only used by the renderer to take a snapshot of the original slice.
  /// The copy constructed object shares the same texture object and cache with the original one.
  MaskVolumeSlice( const MaskVolumeSlice& copy );

  virtual ~MaskVolumeSlice();

  bool get_mask_at( size_t i, size_t j ) const;

  void set_mask_at( size_t i, size_t j );

  void clear_mask_at( size_t i, size_t j );

  // UPLOAD_TEXTURE:
  /// Upload the mask slice to graphics texture.
  /// NOTE: This function allocates resources on the GPU, so the caller should
  /// acquire a lock on the RenderResources before calling this function. 
  virtual void upload_texture();

  // CLONE:
  /// Make a copy of the slice, which will share texture object with the original one.
  virtual VolumeSliceHandle clone();

  // SET_VOLUME:
  /// Set the volume out of which the slice will be taken.
  virtual void set_volume( const VolumeHandle& volume );

  // GET_MASK_DATA_BLOCK:
  /// Return a handle to the underlying mask data block.
  MaskDataBlockHandle get_mask_data_block() const;

  // GET_CACHED_DATA:
  /// Return a pointer to the cached data of the current slice.
  unsigned char* get_cached_data();

  // RELEASE_CACHED_DATA:
  /// Sync the cached data back to the mask volume and then release the memory.
  void release_cached_data();

  // COPY_SLICE_DATA:
  /// Copy the mask data of current slice into the buffer.
  void copy_slice_data( std::vector< unsigned char >& buffer, bool invert = false ) const;

  // COPY_SLICE_DATA:
  /// Copy the mask data of current slice into the buffer.
  void copy_slice_data( unsigned char* buffer, bool invert = false ) const;

  // SET_SLICE_DATA:
  /// Replace the data of the slice with the content of the buffer.
  void set_slice_data( const unsigned char* buffer, bool trigger_update = false );

public:
  // CACHE_UPDATED_SIGNAL:
  /// Triggered when the cache has been updated.
  typedef boost::signals2::signal< void () > cache_updated_signal_type;
  cache_updated_signal_type cache_updated_signal_;

private:
  ///  Pointer to the mask data block. The base class keeps a handle of the volume,
  /// so it's safe to use a pointer here.
  MaskDataBlock* mask_data_block_;

  MaskVolumeSlicePrivateHandle private_;
};

} // end namespace Core

#endif
