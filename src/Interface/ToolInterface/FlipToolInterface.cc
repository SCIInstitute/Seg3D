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
#include <Application/Tools/FlipTool.h>
#include <Application/PreferencesManager/PreferencesManager.h>

//Interface Includes
#include <Interface/ToolInterface/FlipToolInterface.h>
#include "ui_FlipToolInterface.h"


SCI_REGISTER_TOOLINTERFACE( Seg3D, FlipToolInterface )

namespace Seg3D
{

class FlipToolInterfacePrivate
{
public:
  Ui::FlipToolInterface ui_;
};

FlipToolInterface::FlipToolInterface() :
  private_( new FlipToolInterfacePrivate )
{
}

FlipToolInterface::~FlipToolInterface()
{
  this->disconnect_all();
}

// build the interface and connect it to the state manager
bool FlipToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  FlipTool* tool = dynamic_cast< FlipTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_group_, tool->target_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.layer_list_, tool->target_layers_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_group_, tool->use_active_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replace_checkbox_, tool->replace_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.flip_x_button_, boost::bind(
    &FlipTool::dispatch_flip, tool, Core::Interface::GetWidgetActionContext(), 0 ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.flip_y_button_, boost::bind(
    &FlipTool::dispatch_flip, tool, Core::Interface::GetWidgetActionContext(), 1 ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.flip_z_button_, boost::bind(
    &FlipTool::dispatch_flip, tool, Core::Interface::GetWidgetActionContext(), 2 ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.rotate_axial_ccw_button_, boost::bind(
    &FlipTool::dispatch_rotate, tool, Core::Interface::GetWidgetActionContext(), 2, true ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.rotate_axial_cw_button_, boost::bind(
    &FlipTool::dispatch_rotate, tool, Core::Interface::GetWidgetActionContext(), 2, false ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.rotate_coronal_ccw_button_, boost::bind(
    &FlipTool::dispatch_rotate, tool, Core::Interface::GetWidgetActionContext(), 1, true ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.rotate_coronal_cw_button_, boost::bind(
    &FlipTool::dispatch_rotate, tool, Core::Interface::GetWidgetActionContext(), 1, false ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.rotate_sagittal_ccw_button_, boost::bind(
    &FlipTool::dispatch_rotate, tool, Core::Interface::GetWidgetActionContext(), 0, true ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.rotate_sagittal_cw_button_, boost::bind(
    &FlipTool::dispatch_rotate, tool, Core::Interface::GetWidgetActionContext(), 0, false ) );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_group_, 
    tool->use_active_group_state_, true ); 
  QtUtils::QtBridge::Enable( this->private_->ui_.flip_x_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.flip_y_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.flip_z_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.rotate_axial_ccw_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.rotate_axial_cw_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.rotate_coronal_ccw_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.rotate_coronal_cw_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.rotate_sagittal_ccw_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.rotate_sagittal_cw_button_, tool->valid_target_state_ );
  
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    this->change_x_axis_label( PreferencesManager::Instance()->x_axis_label_state_->get() );
    this->change_y_axis_label( PreferencesManager::Instance()->y_axis_label_state_->get() );
    this->change_z_axis_label( PreferencesManager::Instance()->z_axis_label_state_->get() );

    this->add_connection( PreferencesManager::Instance()->x_axis_label_state_->value_changed_signal_.
      connect( boost::bind( &FlipToolInterface::ChangeXAxisLabel, qpointer_type( this ), _1 ) ) );
    this->add_connection( PreferencesManager::Instance()->y_axis_label_state_->value_changed_signal_.
      connect( boost::bind( &FlipToolInterface::ChangeYAxisLabel, qpointer_type( this ), _1 ) ) );
    this->add_connection( PreferencesManager::Instance()->z_axis_label_state_->value_changed_signal_.
      connect( boost::bind( &FlipToolInterface::ChangeZAxisLabel, qpointer_type( this ), _1 ) ) );
  }
  
#if defined ( __APPLE__ )  
  this->private_->ui_.verticalLayout->setSpacing( 8 );
  this->private_->ui_.verticalLayout_2->setSpacing( 8 );
#endif

  //Send a message to the log that we have finished with building the Flip Tool Interface
  CORE_LOG_DEBUG( "Finished building a Flip Tool Interface" );

  return true;
} // end build_widget

void FlipToolInterface::ChangeXAxisLabel( qpointer_type qpointer, std::string label )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &FlipToolInterface::change_x_axis_label, qpointer.data(), label ) ) );
}

void FlipToolInterface::ChangeYAxisLabel( qpointer_type qpointer, std::string label )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &FlipToolInterface::change_y_axis_label, qpointer.data(), label ) ) );
}

void FlipToolInterface::ChangeZAxisLabel( qpointer_type qpointer, std::string label )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &FlipToolInterface::change_z_axis_label, qpointer.data(), label ) ) );
}

void FlipToolInterface::change_x_axis_label( std::string label )
{
  this->private_->ui_.rotate_sagittal_cw_button_->setText( 
    QString::fromStdString( label )+ QString::fromUtf8( " (Clockwise)" ) );
  this->private_->ui_.rotate_sagittal_ccw_button_->setText( 
    QString::fromStdString( label )+ QString::fromUtf8( " (Counterclockwise)" ) );
}

void FlipToolInterface::change_y_axis_label( std::string label )
{
  this->private_->ui_.rotate_coronal_cw_button_->setText( 
    QString::fromStdString( label )+ QString::fromUtf8( " (Clockwise)" ) );
  this->private_->ui_.rotate_coronal_ccw_button_->setText( 
    QString::fromStdString( label )+ QString::fromUtf8( " (Counterclockwise)" ) );
}

void FlipToolInterface::change_z_axis_label( std::string label )
{
  this->private_->ui_.rotate_axial_cw_button_->setText( 
    QString::fromStdString( label )+ QString::fromUtf8( " (Clockwise)" ) );
  this->private_->ui_.rotate_axial_ccw_button_->setText( 
    QString::fromStdString( label )+ QString::fromUtf8( " (Counterclockwise)" ) );
}

} // namespace Seg3D
