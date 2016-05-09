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

#ifndef APPLICATION_FILTERS_ITKRESAMPLEFILTER_H
#define APPLICATION_FILTERS_ITKRESAMPLEFILTER_H

#include <Application/Filters/ITKFilter.h>
#include <Application/Filters/Utils/PadFilterInternals.h>
#include <Application/Filters/Utils/PadValues.h>

#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/DataType.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>

namespace Filter {

class ITKResampleFilter : public Seg3D::ITKFilter
{
private:
  bool replace_;
  bool crop_; // Whether to use a different dataset boundary than the original
  std::string padding_;  // default is max
  bool padding_only_;
  std::string interpolator_;
  unsigned int dims_[ 3 ];
  int spline_order_;

  Core::Point range_min_; // resample range in index space of the input data
  Core::Point range_max_; // resample range in index space of the input data

  Core::DataType data_type_;

  std::vector< Seg3D::LayerHandle > src_layers_;
  std::vector< Seg3D::LayerHandle > dst_layers_;
  std::vector< std::string > dst_layer_ids_;

  std::vector< Core::GridTransform > output_transforms_; // Per layer
  Core::GridTransform current_output_transform_;

  PadFilterInternalsHandle pad_internals_;

public:
  ITKResampleFilter( const std::string& interpolator, int spline_order, bool replace, bool crop, const std::string& padding, Core::Point range_min, Core::Point range_max, Seg3D::SandboxID sandbox );
  virtual ~ITKResampleFilter() {}

  bool setup_layers(const std::vector< std::string >& layer_ids,
                    bool match_grid_transform, const Core::GridTransform& grid_transform,
                    unsigned int dimX, unsigned int dimY, unsigned int dimZ);
  void pad_data_layer( Seg3D::DataLayerHandle input, Seg3D::DataLayerHandle output );
  void pad_mask_layer( Seg3D::MaskLayerHandle input, Seg3D::MaskLayerHandle output );

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const override
  {
    return "ITK Resample Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const override
  {
    return "ITKResample";
  }

  const std::vector< std::string >& get_dst_layer_ids() const { return dst_layer_ids_; }

public:
  static const std::string LINEAR_C;
  static const std::string B_SPLINE_C;
  static const std::string NEAREST_NEIGHBOR_C;

  static bool IsITKResample( const std::string& interpolator )
  {
    return ( ( interpolator == LINEAR_C ) ||
             ( interpolator == B_SPLINE_C ) ||
             ( interpolator ==  NEAREST_NEIGHBOR_C ) );
  }

  SCI_TYPED_ITK_RUN_DECL( this->data_type_ )

};

}

#endif
