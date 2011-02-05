/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
 
// STL includes
#include <sstream>
#include <iostream>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/Application/TransferFunctionFeatureWidget.h>

// Automatically generated UI file
#include "ui_TransferFunctionFeatureWidget.h"

namespace Seg3D
{

class TransferFunctionFeatureWidgetPrivate
{
public:
  Ui::TransferFunctionFeatureWidget ui_;
};

TransferFunctionFeatureWidget::TransferFunctionFeatureWidget(
  Core::TransferFunctionFeatureHandle feature, QWidget *parent ) :
  QWidget( parent ),
  private_( new TransferFunctionFeatureWidgetPrivate )
{
  // Set up the private internals of the LayerManagerInterface class
  this->private_->ui_.setupUi( this );
  
  QtUtils::QtBridge::Connect( this->private_->ui_.red_color_slider_, feature->red_color_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.green_color_slider_, feature->green_color_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.blue_color_slider_, feature->blue_color_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.shininess_slider_, feature->shininess_state_ );
}

TransferFunctionFeatureWidget::~TransferFunctionFeatureWidget()
{
}

} // end namespace Seg3D
