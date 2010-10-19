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
  this->private_->ui_.horizontalLayout->setAlignment( Qt::AlignHCenter );

  // Step 2 - get a pointer to the tool
  BinaryDilateErodeFilter* tool = dynamic_cast< BinaryDilateErodeFilter* > ( this->tool().get() );
  
  // Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.erode_, 
    tool->erode_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.dilate_, 
    tool->dilate_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.mask_,
    tool->mask_state_ );  
  QtUtils::QtBridge::Connect( this->private_->ui_.mask_invert_,
    tool->mask_invert_state_ ); 

  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, 
    tool->replace_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.dilateButton,
    tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.erodeButton,
    tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.dilateerodeButton,
    tool->valid_target_state_ );

  // Step 4 - Qt connections
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    this->private_->ui_.target_layer_->setDisabled( tool->use_active_layer_state_->get() );
    this->connect( this->private_->ui_.use_active_layer_, SIGNAL( toggled( bool ) ),
      this->private_->ui_.target_layer_, SLOT( setDisabled( bool ) ) );
  }

  this->connect( this->private_->ui_.dilateerodeButton, 
    SIGNAL( clicked() ), this, SLOT( dilateerode_filter() ) );

  this->connect( this->private_->ui_.dilateButton, 
    SIGNAL( clicked() ), this, SLOT( dilate_filter() ) );

  this->connect( this->private_->ui_.erodeButton, 
    SIGNAL( clicked() ), this, SLOT( erode_filter() ) );
  
  return true;

} // end build_widget

void BinaryDilateErodeFilterInterface::dilateerode_filter()
{
  BinaryDilateErodeFilter* typed_tool = dynamic_cast< BinaryDilateErodeFilter* > ( tool().get() );
  typed_tool->execute_dilateerode( Core::Interface::GetWidgetActionContext() );
}

void BinaryDilateErodeFilterInterface::dilate_filter()
{
  BinaryDilateErodeFilter* typed_tool = dynamic_cast< BinaryDilateErodeFilter* > ( tool().get() );
  typed_tool->execute_dilate( Core::Interface::GetWidgetActionContext() );
}

void BinaryDilateErodeFilterInterface::erode_filter()
{
  BinaryDilateErodeFilter* typed_tool = dynamic_cast< BinaryDilateErodeFilter* > ( tool().get() );
  typed_tool->execute_erode( Core::Interface::GetWidgetActionContext() );
}

} // end namespace Seg3D
