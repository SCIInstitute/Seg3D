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
#include <QtUtils/Widgets/QtHistogramWidget.h>

//Interface Includes
#include <Interface/ToolInterface/CropToolInterface.h>
#include "ui_CropToolInterface.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/CropTool.h>
#include <Application/Layer/LayerManager.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, CropToolInterface )

namespace Seg3D
{

class CropToolInterfacePrivate
{
public:
  Ui::CropToolInterface ui_;
};

// constructor
CropToolInterface::CropToolInterface() :
  private_( new CropToolInterfacePrivate )
{
}

// destructor
CropToolInterface::~CropToolInterface()
{
}

// build the interface and connect it to the state manager
bool CropToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  CropTool* tool = dynamic_cast< CropTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_group_, tool->target_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.layer_list_, tool->target_layers_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_group_, tool->use_active_group_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.crop_origin_x_, tool->cropbox_origin_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_origin_y_, tool->cropbox_origin_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_origin_z_, tool->cropbox_origin_state_[ 2 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.crop_width_, tool->cropbox_size_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_height_, tool->cropbox_size_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_depth_, tool->cropbox_size_state_[ 2 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.crop_origin_index_x_, tool->cropbox_origin_index_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_origin_index_y_, tool->cropbox_origin_index_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_origin_index_z_, tool->cropbox_origin_index_state_[ 2 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.crop_index_width_, tool->cropbox_size_index_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_index_height_, tool->cropbox_size_index_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.crop_index_depth_, tool->cropbox_size_index_state_[ 2 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.crop_index_checkbox_, tool->crop_in_index_space_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.crop_world_widget_, tool->crop_in_index_space_state_, true );
  QtUtils::QtBridge::Show( this->private_->ui_.crop_index_widget_, tool->crop_in_index_space_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.replace_checkbox_, tool->replace_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.crop_button_, boost::bind(
    &CropTool::execute, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.reset_button_, 
    boost::bind( &CropTool::reset, tool ) );

  QtUtils::QtBridge::Enable( this->private_->ui_.crop_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_group_, 
    tool->use_active_group_state_, true ); 
  
  this->private_->ui_.crop_origin_x_->set_description( "X" );
  this->private_->ui_.crop_origin_y_->set_description( "Y" );
  this->private_->ui_.crop_origin_z_->set_description( "Z" );
  this->private_->ui_.crop_origin_index_x_->set_description( "X" );
  this->private_->ui_.crop_origin_index_y_->set_description( "Y" );
  this->private_->ui_.crop_origin_index_z_->set_description( "Z" );
  
  this->private_->ui_.crop_width_->set_description( "Width" );
  this->private_->ui_.crop_height_->set_description( "Height" );
  this->private_->ui_.crop_depth_->set_description( "Depth" );
  this->private_->ui_.crop_index_width_->set_description( "Width" );
  this->private_->ui_.crop_index_height_->set_description( "Height" );
  this->private_->ui_.crop_index_depth_->set_description( "Depth" );

  CORE_LOG_DEBUG( "Finished building a Resample Tool Interface" );

  return ( true );
} 

} // end namespace Seg3D
