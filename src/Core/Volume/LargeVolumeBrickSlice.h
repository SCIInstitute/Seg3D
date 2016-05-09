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

#ifndef CORE_VOLUME_LARGEVOLUMEBRICKSLICE_H
#define CORE_VOLUME_LARGEVOLUMEBRICKSLICE_H

#include <Core/Volume/LargeVolume.h>
#include <Core/Volume/VolumeSlice.h>

namespace Core
{

class LargeVolumeBrickSlice;
typedef boost::shared_ptr< LargeVolumeBrickSlice > LargeVolumeBrickSliceHandle;

class LargeVolumeBrickSlicePrivate;
typedef boost::shared_ptr< LargeVolumeBrickSlicePrivate > LargeVolumeBrickSlicePrivateHandle;

class LargeVolumeBrickSlice
{
public:
  typedef unsigned short texture_data_type;

  LargeVolumeBrickSlice( LargeVolumeHandle volume, LargeVolumeSchemaHandle schema, const BrickInfo& bi );
  ~LargeVolumeBrickSlice();


  /// Upload the data slice to graphics texture if necessary, and return the texture handle.
  /// NOTE: This function allocates resources on the GPU, so the caller should
  /// acquire a lock on the RenderResources before calling this function.
  Texture2DHandle get_texture( VolumeSliceType slice_type, double slice_pos, int& width, int& height, const std::string& load_key );

  /// GET_INNER_BRICK_BBOX
  BBox get_inner_brick_bbox() const;

  /// GET_OUTER_BRICK_BBOX
  BBox get_outer_brick_bbox() const;

private:
  LargeVolumeBrickSlicePrivateHandle private_;
};

} // end namespace Core

#endif
