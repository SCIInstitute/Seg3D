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

#ifndef CORE_VOLUMERENDERER_VOLUMERENDERERBASE_H
#define CORE_VOLUMERENDERER_VOLUMERENDERERBASE_H

#include <boost/utility.hpp>

#include <Core/Geometry/View3D.h>
#include <Core/Volume/DataVolume.h>
#include <Core/VolumeRenderer/TransferFunction.h>

namespace Core
{

class VolumeRendererBase;
typedef boost::shared_ptr< VolumeRendererBase > VolumeRendererBaseHandle;

class VolumeRendererBasePrivate;
typedef boost::shared_ptr< VolumeRendererBasePrivate > VolumeRendererBasePrivateHandle;

class VolumeRenderingParam
{
public:
  View3D view_;
  double znear_;
  double zfar_;
  double sampling_rate_;
  bool enable_lighting_;
  bool enable_fog_;
  bool orthographic_;
  TransferFunctionHandle transfer_function_;
  float clip_plane_[ 6 ][ 4 ];
  int enable_clip_plane_[ 6 ];
  bool enable_clipping_;

  int grid_resolution_;
  double occlusion_angle_;
};

class BrickEntry
{
public:
  DataVolumeBrickHandle brick_;
  double distance_;
};

class VolumeRendererBase : public boost::noncopyable
{
public:
  VolumeRendererBase();
  virtual ~VolumeRendererBase();

  virtual void initialize();
  virtual void render( DataVolumeHandle volume, const VolumeRenderingParam& param ) = 0; 

protected:

  void process_volume( DataVolumeHandle volume, double sample_rate,
    const View3D& view, bool orthographic, bool front_to_back, 
    std::vector< BrickEntry >& sorted_bricks );
  void slice_brick( DataVolumeBrickHandle brick,
    std::vector< PointF >& polygon_vertices, 
    std::vector< int >& first_vec, std::vector< int >& count_vec );
  Vector get_voxel_size();
  double get_normalized_sample_distance();
  double get_sample_distance();

private:
  VolumeRendererBasePrivateHandle private_;
};

} // end namespace Core

#endif
