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

#ifndef APPLICATION_FILTERS_NRRDRESAMPLEFILTER_H
#define APPLICATION_FILTERS_NRRDRESAMPLEFILTER_H

// teem includes
#include <teem/nrrd.h>
#include <privateNrrd.h>

#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Utils/PadFilterInternals.h>
#include <Application/Filters/Utils/PadValues.h>

namespace Filter
{

class NrrdResampleFilter : public Seg3D::LayerFilter
{
private:
  bool replace_;
  bool crop_; // Whether to use a different dataset boundary than the original
  std::string padding_; // default is max
  bool padding_only_;
  unsigned int dims_[ 3 ];

  Core::Point range_min_; // resample range in index space of the input data
  Core::Point range_max_; // resample range in index space of the input data

  NrrdKernelSpec* mask_kernel_;
  NrrdKernelSpec* data_kernel_;

  std::vector< Seg3D::LayerHandle > src_layers_;
  std::vector< Seg3D::LayerHandle > dst_layers_;
  std::vector< std::string > dst_layer_ids_;

  std::vector< NrrdResampleContext* > resample_contexts_; // Per layer
  std::vector< Core::GridTransform > output_transforms_; // Per layer
  NrrdResampleContext* current_resample_context_;
  Core::GridTransform current_output_transform_;

  PadFilterInternalsHandle pad_internals_;

public:
  NrrdResampleFilter( const std::string& kernel, double gauss_sigma, double gauss_cutoff, bool replace, bool crop, const std::string& padding, Core::Point range_min, Core::Point range_max, Seg3D::SandboxID sandbox );
  virtual ~NrrdResampleFilter();

  bool setup_layers(const std::vector< std::string >& layer_ids,
                    bool match_grid_transform, const Core::GridTransform& grid_transform,
                    unsigned int dimX, unsigned int dimY, unsigned int dimZ);

  // COMPUTE_OUTPUT_GRID_TRANSFORM:
  // Compute the output grid transform of the input layer.
  bool compute_output_grid_transform( Seg3D::LayerHandle layer,
                                      NrrdResampleContext* resample_context,
                                      Core::GridTransform& grid_transform );

  // DETECT_PADDING_ONLY:
  // Detect cases where sample positions are not changed so we only need to do padding/cropping.
  void detect_padding_only();

  // NRRD_RESAMPLE:
  // Resample the nrrd data.
  bool nrrd_resample( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk );

  // RESAMPLE_DATA_LAYER:
  // Resample a  data layer.
  void resample_data_layer( Seg3D::DataLayerHandle input, Seg3D::DataLayerHandle output );

  // RESAMPLE_MASK_LAYER:
  // Resample a mask layer.
  void resample_mask_layer( Seg3D::MaskLayerHandle input, Seg3D::MaskLayerHandle output );

  // PAD_AND_CROP_DATA_LAYER:
  // Pad/crop data layer for cases where sample positions are not changed.
  void pad_and_crop_data_layer( Seg3D::DataLayerHandle input, Seg3D::DataLayerHandle output );

  // PAD_AND_CROP_TYPED_DATA:
  // Templated implementation for pad_and_crop_data_layer.
  template< class T >
  void pad_and_crop_typed_data( Core::DataBlockHandle src, Core::DataBlockHandle dst,
                                Seg3D::DataLayerHandle output_layer );

  // PAD_AND_CROP_MASK_LAYER:
  // Pad/crop mask layer for cases where sample positions are not changed.
  void pad_and_crop_mask_layer( Seg3D::MaskLayerHandle input, Seg3D::MaskLayerHandle output );

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called
  // when the thread is launched.
  virtual void run_filter();

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Resample Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Resample";  
  } 

  const std::vector< std::string >& get_dst_layer_ids() const { return dst_layer_ids_; }

public:
  static const std::string BOX_C;
  static const std::string TENT_C;
  static const std::string CUBIC_CR_C;
  static const std::string CUBIC_BS_C;
  static const std::string QUARTIC_C;
  static const std::string GAUSSIAN_C;

  static bool IsNrrdResample( const std::string& kernel )
  {
    return ( ( kernel == BOX_C ) ||
             ( kernel == TENT_C ) ||
             ( kernel == CUBIC_CR_C ) ||
             ( kernel == CUBIC_BS_C ) ||
             ( kernel == QUARTIC_C ) ||
             ( kernel == GAUSSIAN_C ) );
  }
};

}

#endif
