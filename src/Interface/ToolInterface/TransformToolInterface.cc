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

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Application Includes
#include <Application/Tools/TransformTool.h>

//Interface Includes
#include <Interface/ToolInterface/TransformToolInterface.h>
#include "ui_TransformToolInterface.h"


SCI_REGISTER_TOOLINTERFACE( Seg3D, TransformToolInterface )

namespace Seg3D
{

class TransformToolInterfacePrivate
{
public:
  Ui::TransformToolInterface ui_;
};

TransformToolInterface::TransformToolInterface() :
  private_( new TransformToolInterfacePrivate )
{
}

TransformToolInterface::~TransformToolInterface()
{
}

// build the interface and connect it to the state manager
bool TransformToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_7->setAlignment( Qt::AlignHCenter );
  
  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  TransformTool* tool = dynamic_cast< TransformTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_group_, tool->target_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.layer_list_, tool->target_layers_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_group_, tool->use_active_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replace_checkbox_, tool->replace_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.origin_x_spinbox_, tool->origin_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.origin_y_spinbox_, tool->origin_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.origin_z_spinbox_, tool->origin_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.spacing_x_spinbox_, tool->spacing_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.spacing_y_spinbox_, tool->spacing_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.spacing_z_spinbox_, tool->spacing_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.keep_aspect_ratio_checkbox_, 
    tool->keep_aspect_ratio_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.preview_layer_combobox_,
    tool->preview_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.show_border_checkbox_, 
    tool->show_border_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.show_preview_checkbox_,
    tool->show_preview_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.execute_button_, boost::bind(
    &TransformTool::execute, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.reset_button_, boost::bind(
    &TransformTool::reset, tool ) );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_group_, 
    tool->use_active_group_state_, true ); 
  QtUtils::QtBridge::Enable( this->private_->ui_.execute_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  
  //Send a message to the log that we have finished with building the Flip Tool Interface
  CORE_LOG_DEBUG( "Finished building a Transform Tool Interface" );

  return true;
} // end build_widget


} // namespace Seg3D
