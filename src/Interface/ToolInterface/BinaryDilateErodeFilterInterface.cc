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

// Boost includes
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

// Core includes
#include <Core/Interface/Interface.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Qt Gui Includes
#include <Interface/ToolInterface/BinaryDilateErodeFilterInterface.h>
#include "ui_BinaryDilateErodeFilterInterface.h"

//Application Includes
#include <Application/Tools/BinaryDilateErodeFilter.h>

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
  
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );

  QtUtils::QtBridge::Connect( this->private_->ui_.only2d_, tool->only2d_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.slice_type_, tool->slice_type_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_viewer_, tool->use_active_viewer_state_ );

  QtUtils::QtBridge::Enable( this->private_->ui_.use_active_viewer_, tool->only2d_state_ );

  std::vector< Core::StateBaseHandle > states;
  states.push_back( tool->only2d_state_ );
  states.push_back( tool->use_active_viewer_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.slice_type_, states, 
    boost::lambda::bind( &Core::StateBool::get, tool->only2d_state_.get() ) &&
    !boost::lambda::bind( &Core::StateBool::get, tool->use_active_viewer_state_.get() ) );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_layer_, tool->use_active_layer_state_, true );

  QtUtils::QtBridge::Connect( this->private_->ui_.dilateerodeButton, boost::bind( 
    &BinaryDilateErodeFilter::execute_dilateerode, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.dilateButton, boost::bind( 
    &BinaryDilateErodeFilter::execute_dilate, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.erodeButton, boost::bind( 
    &BinaryDilateErodeFilter::execute_erode, tool, Core::Interface::GetWidgetActionContext() ) );
  
  this->private_->ui_.erode_->set_description( "Erode" );
  this->private_->ui_.dilate_->set_description( "Dilate" );
  
  return true;

} // end build_widget

} // end namespace Seg3D
