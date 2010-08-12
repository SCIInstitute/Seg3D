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

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Interface Includes
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/BinaryDilateErodeFilterInterface.h>
#include "ui_BinaryDilateErodeFilterInterface.h"

//Application Includes
#include <Application/Tools/BinaryDilateErodeFilter.h>
//#include <Application/Filters/Actions/ActionBinaryDilateErode.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, BinaryDilateErodeFilterInterface )

namespace Seg3D
{

class BinaryDilateErodeFilterInterfacePrivate
{
public:
  Ui::BinaryDilateErodeFilterInterface ui_;
  
    QtUtils::QtSliderIntCombo *erode_;
  QtUtils::QtSliderIntCombo *dilate_;
};

// constructor
BinaryDilateErodeFilterInterface::BinaryDilateErodeFilterInterface() :
  private_( new BinaryDilateErodeFilterInterfacePrivate )
{
}

// destructor
BinaryDilateErodeFilterInterface::~BinaryDilateErodeFilterInterface()
{
}

// build the interface and connect it to the state manager
bool BinaryDilateErodeFilterInterface::build_widget( QFrame* frame )
{
  // Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  // add sliderspinnercombo's
  this->private_->erode_ = new QtUtils::QtSliderIntCombo();
  this->private_->ui_.erodeHLayout_bottom->addWidget( this->private_->erode_ );

  this->private_->dilate_ = new QtUtils::QtSliderIntCombo();
  this->private_->ui_.dialateHLayout_bottom->addWidget( this->private_->dilate_ );
  
  // Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  BinaryDilateErodeFilter* tool = dynamic_cast< BinaryDilateErodeFilter* > ( base_tool_.get() );
  
  // Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->erode_, 
    tool->erode_state_ );
  QtUtils::QtBridge::Connect( this->private_->dilate_, 
    tool->dilate_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, 
    tool->replace_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton,
    tool->valid_target_state_ );

  // Step 4 - Qt connections
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    this->private_->ui_.target_layer_->setDisabled( tool->use_active_layer_state_->get() );
    this->connect( this->private_->ui_.use_active_layer_, SIGNAL( toggled( bool ) ),
      this->private_->ui_.target_layer_, SLOT( setDisabled( bool ) ) );
  }

  this->connect( this->private_->ui_.runFilterButton, 
    SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  
  return true;

} // end build_widget

void BinaryDilateErodeFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  BinaryDilateErodeFilter* tool =
    dynamic_cast< BinaryDilateErodeFilter* > ( base_tool_.get() );

//  ActionBinaryDilateErode::Dispatch( tool->target_layer_state_->export_to_string(), 
//    tool->dilate_state_->get(), tool->erode_state_->get(), tool->replace_state_->get() ); 
}

} // end namespace Seg3D
