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

#ifndef APPLICATION_FILTERS_LAYERRESAMPLER_H
#define APPLICATION_FILTERS_LAYERRESAMPLER_H

#include <Core/State/StateHandler.h>

#include <Application/Layer/LayerFWD.h>

namespace Seg3D
{

class LayerResampler;
typedef boost::shared_ptr< LayerResampler > LayerResamplerHandle;

class LayerResamplerPrivate;
typedef boost::shared_ptr< LayerResamplerPrivate > LayerResamplerPrivateHandle;

class LayerResampler : public Core::StateHandler
{
  // -- constructor/destructor --
public:
  LayerResampler( LayerHandle src_layer, LayerHandle dst_layer );
  virtual ~LayerResampler();

public:

  /// EXECUTE:
  /// Execute the tool and dispatch the action
  void execute( Core::ActionContextHandle context );

  // -- state --
public:
  Core::StateLabeledOptionHandle padding_value_state_;
  Core::StateLabeledOptionHandle kernel_state_;

  Core::StateRangedDoubleHandle gauss_sigma_state_;
  Core::StateRangedDoubleHandle gauss_cutoff_state_;
  Core::StateRangedIntHandle spline_order_state_;

  Core::StateBoolHandle has_gaussian_params_state_;
  Core::StateBoolHandle has_bspline_params_state_;

private:
  LayerResamplerPrivateHandle private_;

};

} // end namespace

#endif
