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

// Core includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/Log.h>

//Interface Includes
#include <QtUtils/Bridge/QtBridge.h>

//Qt Gui Includes
#include <Interface/Application/StyleSheet.h>
#include <Interface/ToolInterface/SpeedlineToolInterface.h>
#include "ui_SpeedlineToolInterface.h"

//Application Includes
#include <Application/Tools/SpeedlineTool.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, SpeedlineToolInterface )

namespace Seg3D
{

class SpeedlineToolInterfacePrivate
{
public:
  Ui::SpeedlineToolInterface ui_;
};

// constructor
SpeedlineToolInterface::SpeedlineToolInterface() :
  private_( new SpeedlineToolInterfacePrivate )
{
}

// destructor 
SpeedlineToolInterface::~SpeedlineToolInterface()
{
}

// build the interface and connect it to the state manager
bool SpeedlineToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->setStyleSheet( StyleSheet::SPEEDLINE_TOOL_C );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  SpeedlineTool* tool = dynamic_cast< SpeedlineTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_mask_, tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.target_data_layer_, tool->target_data_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.roi_mask_layer_, tool->roi_mask_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_image_spacing_, tool->use_image_spacing_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_face_conn_, tool->use_face_conn_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.fill_button_, boost::bind(
    &SpeedlineTool::fill, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.erase_button_, boost::bind(
    &SpeedlineTool::erase, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.clear_button_, boost::bind(
    &SpeedlineTool::reset, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.reset_parameters_button_, boost::bind(
    &SpeedlineTool::reset_parameters, tool, Core::Interface::GetWidgetActionContext() ) );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_mask_, tool->use_active_layer_state_, true );

  QtUtils::QtBridge::Connect( this->private_->ui_.target_data_layer_, tool->target_data_layer_state_ );

  // weights
  this->private_->ui_.grad_mag_weight_->set_description( "Gradient Magnitude" );
  QtUtils::QtBridge::Connect( this->private_->ui_.grad_mag_weight_, tool->grad_mag_weight_state_ );
  this->private_->ui_.zero_cross_weight_->set_description( "Zero Cross" );
  QtUtils::QtBridge::Connect( this->private_->ui_.zero_cross_weight_, tool->zero_cross_weight_state_ );
  this->private_->ui_.grad_dir_weight_->set_description( "Gradient Direction" );
  QtUtils::QtBridge::Connect( this->private_->ui_.grad_dir_weight_, tool->grad_dir_weight_state_ );

  QtUtils::QtBridge::Enable( this->private_->ui_.fill_button_, tool->valid_target_state_);
  QtUtils::QtBridge::Enable( this->private_->ui_.erase_button_, tool->valid_target_state_ );

  QtUtils::QtBridge::Show( this->private_->ui_.message_mask_alert_, tool->valid_target_state_, true );
  QtUtils::QtBridge::Show( this->private_->ui_.message_data_layer_alert_, tool->valid_target_data_layer_state_, true );

  

  //Send a message to the log that we have finished with building the Speedline Tool Interface
  CORE_LOG_MESSAGE("Finished building a Speedline Tool Interface");
  
#if defined ( __APPLE__ )  
  this->private_->ui_.verticalLayout->setSpacing( 8 );
#endif
  

  return true;
} // end build_widget

} // end namespace Seg3D

