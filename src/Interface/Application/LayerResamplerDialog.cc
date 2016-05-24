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

#include <QtUtils/Bridge/QtBridge.h>

#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/LayerResamplerDialog.h>

#include "ui_LayerResamplerDialog.h"

namespace Seg3D
{

class LayerResamplerDialogPrivate
{
public:
  Ui::LayerResamplerDialog ui_;
};

LayerResamplerDialog::LayerResamplerDialog( 
  const LayerResamplerHandle& layer_resampler, 
  QWidget* parent ) :
  QDialog( parent ),
  private_( new LayerResamplerDialogPrivate )
{
  this->private_->ui_.setupUi( this );
  this->setStyleSheet( StyleSheet::LAYER_RESAMPLER_C );

  QtUtils::QtBridge::Connect( this->private_->ui_.padding_combobox_, layer_resampler->padding_value_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.kernel_combobox_, layer_resampler->kernel_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.sigma_spinbox_, layer_resampler->gauss_sigma_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.cutoff_spinbox_, layer_resampler->gauss_cutoff_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.spline_order_, layer_resampler->spline_order_state_ );

  this->private_->ui_.spline_order_->set_description( "Spline Order" );

  QtUtils::QtBridge::Show( this->private_->ui_.param_widget_, layer_resampler->has_gaussian_params_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.bspline_widget_, layer_resampler->has_bspline_params_state_ );
}

} // end namespace Seg3D
