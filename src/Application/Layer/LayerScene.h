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

#ifndef APPLICATION_LAYER_LAYERSCENE_H
#define APPLICATION_LAYER_LAYERSCENE_H

#include <vector>
#include <boost/shared_ptr.hpp>

#include <Application/Layer/Layer.h>

#include <Core/Volume/DataVolume.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Volume/LargeVolumeBrickSlice.h>
#include <Core/Volume/MaskVolume.h>
#include <Core/Volume/MaskVolumeSlice.h>

namespace Seg3D
{

class LayerSceneItem
{
public:
  LayerSceneItem() {}
  virtual ~LayerSceneItem() {}

  virtual Core::VolumeType type() = 0;

public:
  std::string layer_id_;
  double opacity_;
  LayerHandle layer_;
  Core::GridTransform grid_transform_;
  Core::VolumeSliceHandle volume_slice_; // This value is set and used by Renderer
};

class DataLayerSceneItem : public LayerSceneItem
{
public:
  DataLayerSceneItem() {}
  virtual ~DataLayerSceneItem() {}

  virtual Core::VolumeType type()
  {
    return Core::VolumeType::DATA_E;
  }

public:
  int color_;
  double data_min_;
  double data_max_;
  double display_min_;
  double display_max_;
  bool volume_rendered_;
  bool pick_color_;
};

class LargeVolumeLayerSceneItem : public LayerSceneItem
{
public:
  LargeVolumeLayerSceneItem() {}
  virtual ~LargeVolumeLayerSceneItem() {}

  virtual Core::VolumeType type()
  {
    return Core::VolumeType::LARGE_DATA_E;
  }

public:
  int color_;
  double data_min_;
  double data_max_;
  double display_min_;
  double display_max_;
  bool pick_color_;
  std::vector<Core::LargeVolumeBrickSliceHandle> tiles_;
};

class MaskLayerSceneItem : public LayerSceneItem
{
public:
  MaskLayerSceneItem() {}
  virtual ~MaskLayerSceneItem() {}

  virtual Core::VolumeType type()
  {
    return Core::VolumeType::MASK_E;
  }

public:
  int color_;
  int border_;
  int fill_;
  bool show_isosurface_;
};

} // end namespace Seg3D

#endif
