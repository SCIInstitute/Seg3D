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

// boost includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

//Qt Gui Includes
#include <Interface/ToolInterface/ClipboardToolInterface.h>
#include "ui_ClipboardToolInterface.h"

//Application Includes
#include <Application/Tools/ClipboardTool.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, ClipboardToolInterface )

namespace Seg3D
{

class ClipboardToolInterfacePrivate
{
public:
  Ui::ClipboardToolInterface ui_; 
};

// constructor
ClipboardToolInterface::ClipboardToolInterface() :
  private_( new ClipboardToolInterfacePrivate )
{
}

// destructor
ClipboardToolInterface::~ClipboardToolInterface()
{
}

// build the interface and connect it to the state manager
bool ClipboardToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );

  
  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ClipboardTool* tool = dynamic_cast< ClipboardTool* > ( base_tool_.get() );
  
  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_mask_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.slice_type_, 
    tool->slice_type_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.copy_slice_number_,
    tool->copy_slice_number_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.paste_slice_number_min_, 
    tool->paste_min_slice_number_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.paste_slice_number_max_,
    tool->paste_max_slice_number_state_ );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_mask_,
    tool->use_active_layer_state_, true );
    QtUtils::QtBridge::Enable( this->private_->ui_.copy_button_, 
    tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.paste_button_,
    tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.copy_slice_number_,
    tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.paste_slice_number_min_,
    tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.paste_slice_number_max_,
    tool->valid_target_state_ );
  
  QtUtils::QtBridge::Connect( this->private_->ui_.copy_button_, boost::bind( 
    &ClipboardTool::copy, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.paste_button_, boost::bind( 
    &ClipboardTool::paste, tool, Core::Interface::GetWidgetActionContext() ) );

  return true;
} 

} // end namespace Seg3D

