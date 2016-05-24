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

#ifndef APPLICATION_FILTERS_UTILS_PADFILTERINTERNALS_H
#define APPLICATION_FILTERS_UTILS_PADFILTERINTERNALS_H

// Boost includes
#include <boost/smart_ptr.hpp>

#include <Application/Layer/Layer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerAbstractFilter.h>

#include <Core/Geometry/GridTransform.h>

namespace Filter {

class PadFilterInternals;
typedef boost::shared_ptr< PadFilterInternals > PadFilterInternalsHandle;
typedef boost::weak_ptr< PadFilterInternals > PadFilterInternalsWeakHandle;

class PadFilterInternals
{
public:
  PadFilterInternals( Seg3D::LayerHandle src_layer, Seg3D::LayerHandle dst_layer, const std::string& padding);
  ~PadFilterInternals() {}

  bool detect_padding_only();

  Core::DataBlockHandle pad_and_crop_data_layer( Seg3D::DataLayerHandle input, Seg3D::DataLayerHandle output, Seg3D::LayerAbstractFilterHandle layer_filter );
  Core::DataBlockHandle pad_and_crop_mask_layer( Seg3D::MaskLayerHandle input, Seg3D::MaskLayerHandle output, Seg3D::LayerAbstractFilterHandle layer_filter );

private:
  template< class T >
  void pad_and_crop_typed_data( Core::DataBlockHandle src,
                                Core::DataBlockHandle dst,
                                Seg3D::DataLayerHandle output_layer,
                                Seg3D::LayerAbstractFilterHandle layer_filter );

  std::string padding_;

  int mapped_x_start_;
  int mapped_y_start_;
  int mapped_z_start_;

  int overlap_x_start_;
  int overlap_y_start_;
  int overlap_z_start_;
  int overlap_nx_;
  int overlap_ny_;
  int overlap_nz_;

  const double EPSILON;

  Core::GridTransform src_trans_;
  Core::GridTransform dst_trans_;
  Core::Point src_origin_to_dst_index_;
};

}

#endif
