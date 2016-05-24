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

#ifndef APPLICATION_FILTERS_PADFILTER_H
#define APPLICATION_FILTERS_PADFILTER_H

#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Utils/PadFilterInternals.h>
#include <Application/Filters/Utils/PadValues.h>

namespace Filter
{

class PadFilter : public Seg3D::LayerFilter
{
private:
  bool replace_;
  std::string padding_; // default is max

  std::vector< Seg3D::LayerHandle > src_layers_;
  std::vector< Seg3D::LayerHandle > dst_layers_;
  std::vector< std::string > dst_layer_ids_;

  std::vector< Core::GridTransform > output_transforms_; // Per layer

  PadFilterInternalsHandle pad_internals_;

public:
  PadFilter( bool replace, const std::string& padding, Seg3D::SandboxID sandbox );
  virtual ~PadFilter() {}

  bool setup_layers( const std::vector< std::string >& layer_ids, const Core::GridTransform& grid_transform );
  void pad_data_layer( Seg3D::DataLayerHandle input, Seg3D::DataLayerHandle output );
  void pad_mask_layer( Seg3D::MaskLayerHandle input, Seg3D::MaskLayerHandle output );

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called
  // when the thread is launched.
  virtual void run_filter();

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Pad Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const
  {
    return "Pad";
  }

  const std::vector< std::string >& get_dst_layer_ids() const { return dst_layer_ids_; }
};

}

#endif
