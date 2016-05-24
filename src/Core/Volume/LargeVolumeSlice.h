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

#ifndef CORE_VOLUME_LARGEVOLUMESLICE_H
#define CORE_VOLUME_LARGEVOLUMESLICE_H

#include <Core/Volume/LargeVolume.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/LargeVolumeBrickSlice.h>

namespace Core
{

class LargeVolumeSlice;
typedef boost::shared_ptr< LargeVolumeSlice > LargeVolumeSliceHandle;

class LargeVolumeSlicePrivate;
typedef boost::shared_ptr< LargeVolumeSlicePrivate > LargeVolumeSlicePrivateHandle;

class LargeVolumeSlice : public VolumeSlice
{
public:
  typedef unsigned short texture_data_type;

  LargeVolumeSlice( const LargeVolumeHandle& large_volume, 
    VolumeSliceType type = VolumeSliceType::AXIAL_E, size_t slice_num = 0 );
  LargeVolumeSlice( const LargeVolumeSlice& copy );
  virtual ~LargeVolumeSlice();


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

  /// Load tiles (bricks) for 2D view
  std::vector<LargeVolumeBrickSliceHandle> load_tiles(double left, double right, double bottom, double top, double pixel_size, const std::string& load_key);

  /// Load tiles (bricks) for 3D view
  std::vector<LargeVolumeBrickSliceHandle> load_3d_tiles( const Transform& mvp_trans, int width, int height, const std::string& load_key );

private:
  LargeVolumeSlicePrivateHandle private_;

  /// Schema of the underlying large volume
  LargeVolumeSchemaHandle lv_schema_;
};

} // end namespace Core

#endif
