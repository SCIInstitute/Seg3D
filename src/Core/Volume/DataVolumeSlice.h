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

#ifndef CORE_VOLUME_DATAVOLUMESLICE_H
#define CORE_VOLUME_DATAVOLUMESLICE_H

#include <Core/Volume/DataVolume.h>
#include <Core/Volume/VolumeSlice.h>

namespace Core
{

class DataVolumeSlice;
typedef boost::shared_ptr< DataVolumeSlice > DataVolumeSliceHandle;

class DataVolumeSlicePrivate;
typedef boost::shared_ptr< DataVolumeSlicePrivate > DataVolumeSlicePrivateHandle;

class DataVolumeSlice : public VolumeSlice
{
public:
  typedef unsigned short texture_data_type;

  DataVolumeSlice( const DataVolumeHandle& data_volume, 
    VolumeSliceType type = VolumeSliceType::AXIAL_E, size_t slice_num = 0 );
  DataVolumeSlice( const DataVolumeSlice& copy );
  virtual ~DataVolumeSlice();

  double get_data_at ( size_t i, size_t j ) const;

  void set_data_at( size_t i, size_t j, double value );

  /// Upload the data slice to graphics texture.
  /// NOTE: This function allocates resources on the GPU, so the caller should
  /// acquire a lock on the RenderResources before calling this function.
  virtual void upload_texture();

  // CLONE:
  /// Make a copy of the slice, which will share texture object with the original one.
  virtual VolumeSliceHandle clone();

  // SET_VOLUME:
  /// Set the volume out of which the slice will be taken.
  virtual void set_volume( const VolumeHandle& volume );

  // CREATE_THRESHOLD_MASK:
  /// Create a threshold mask based on the data of current slice and the give min/max 
  /// values. The mask will be inverted if negative_constraint is true.
  void create_threshold_mask( std::vector< unsigned char >& mask, 
    double min_val, double max_val, bool negative_constraint ) const;

private:
  DataVolumeSlicePrivateHandle private_;

  /// Pointer to the data block. The base class keeps a handle of the volume,
  /// so it is safe to use a pointer here.
  DataBlock* data_block_;

  /// An array of GLenum's for data types, indexed by data_type values
  const static unsigned int TEXTURE_DATA_TYPE_C;

  /// An array of GLenum's for GL internal texture formats, indexed by data_type values.
  /// These formats are picked to best match the data type.
  const static unsigned int TEXTURE_FORMAT_C;
};

} // end namespace Core

#endif
