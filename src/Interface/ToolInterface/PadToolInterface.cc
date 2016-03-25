/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

// Core includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/Log.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Widgets/QtHistogramWidget.h>

//Interface Includes
#include <Interface/ToolInterface/PadToolInterface.h>
#include "ui_PadToolInterface.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/PadTool.h>
#include <Application/Layer/LayerManager.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, PadToolInterface )

namespace Seg3D
{

class PadToolInterfacePrivate
{
public:
  Ui::PadToolInterface ui_;
};

// constructor
PadToolInterface::PadToolInterface() :
  private_( new PadToolInterfacePrivate )
{
}

// destructor
PadToolInterface::~PadToolInterface()
{
}

// build the interface and connect it to the state manager
bool PadToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_6->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.horizontalLayout_7->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  PadTool* tool = dynamic_cast< PadTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_group_, tool->target_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.layer_list_, tool->target_layers_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_group_, tool->use_active_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.padding_combobox_, tool->padding_value_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.pad_x_, tool->pad_level_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.pad_y_, tool->pad_level_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.pad_z_, tool->pad_level_state_[ 2 ] );

  this->private_->ui_.pad_x_->set_description( "Pad Level X" );
  this->private_->ui_.pad_y_->set_description( "Pad Level Y" );
  this->private_->ui_.pad_z_->set_description( "Pad Level Z" );

  QtUtils::QtBridge::Connect( this->private_->ui_.aspect_checkbox_, tool->constraint_aspect_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replace_checkbox_, tool->replace_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.run_button_, boost::bind(
    &PadTool::execute, tool, Core::Interface::GetWidgetActionContext() ) );

  std::vector< Core::StateBaseHandle > execution_related_states( 1 );
  execution_related_states[ 0 ] = tool->valid_target_state_;

  QtUtils::QtBridge::Enable( this->private_->ui_.run_button_, execution_related_states,
                             boost::lambda::bind( &Core::StateBool::get, tool->valid_target_state_.get() ) );

  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_group_, tool->use_active_group_state_, true );

  CORE_LOG_DEBUG( "Finished building a Pad Tool Interface" );

  return ( true );
} 

} // end namespace Seg3D
