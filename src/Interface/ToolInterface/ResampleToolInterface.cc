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

// boost includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Core includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/Log.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Widgets/QtHistogramWidget.h>
#include <QButtonGroup>

//Interface Includes
#include <Interface/ToolInterface/ResampleToolInterface.h>
#include "ui_ResampleToolInterface.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/ResampleTool.h>
#include <Application/Layer/LayerManager.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, ResampleToolInterface )

namespace Seg3D
{

class ResampleToolInterfacePrivate
{
public:
  Ui::ResampleToolInterface ui_;
};

// constructor
ResampleToolInterface::ResampleToolInterface() :
  private_( new ResampleToolInterfacePrivate )
{
}

// destructor
ResampleToolInterface::~ResampleToolInterface()
{
}

// build the interface and connect it to the state manager
bool ResampleToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_6->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.horizontalLayout_7->setAlignment( Qt::AlignHCenter );

  QButtonGroup* button_group = new QButtonGroup( this );
  button_group->addButton( this->private_->ui_.rb_another_group_ );
  button_group->addButton( this->private_->ui_.rb_manual_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ResampleTool* tool = dynamic_cast< ResampleTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_group_, tool->target_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.layer_list_, tool->target_layers_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_group_, tool->use_active_group_state_ );

  QtUtils::QtBridge::Connect( button_group, tool->size_scheme_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.dst_group_combobox_, tool->dst_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.padding_combobox_, tool->padding_value_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.output_x_, tool->output_dimensions_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.output_y_, tool->output_dimensions_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.output_z_, tool->output_dimensions_state_[ 2 ] );
  
  this->private_->ui_.output_x_->set_description( "X" );
  this->private_->ui_.output_y_->set_description( "Y" );
  this->private_->ui_.output_z_->set_description( "Z" );
  this->private_->ui_.scale_->set_description( "Scale" );
  this->private_->ui_.spline_order_->set_description( "Spline Order" );

  QtUtils::QtBridge::Connect( this->private_->ui_.aspect_checkbox_, tool->constraint_aspect_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.scale_, tool->scale_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.spline_order_, tool->spline_order_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.kernel_combobox_, tool->kernel_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.sigma_spinbox_, tool->gauss_sigma_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.cutoff_spinbox_, tool->gauss_cutoff_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.replace_checkbox_, tool->replace_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.run_button_, boost::bind(
    &ResampleTool::execute, tool, Core::Interface::GetWidgetActionContext() ) );

  std::vector< Core::StateBaseHandle > execution_related_states( 2 );
  execution_related_states[ 0 ] = tool->valid_target_state_;
  execution_related_states[ 1 ] = tool->valid_size_state_;
  QtUtils::QtBridge::Enable( this->private_->ui_.run_button_, execution_related_states,
    boost::lambda::bind( &Core::StateBool::get, tool->valid_target_state_.get() ) &&
    boost::lambda::bind( &Core::StateBool::get, tool->valid_size_state_.get() ) );
  
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_group_, tool->use_active_group_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.scale_, tool->constraint_aspect_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.output_x_, tool->constraint_aspect_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.output_y_, tool->constraint_aspect_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.output_z_, tool->constraint_aspect_state_, true );

  QtUtils::QtBridge::Show( this->private_->ui_.gaussian_param_widget_, tool->has_gaussian_params_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.widget_new_size_, tool->manual_size_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.widget_another_group_, tool->manual_size_state_, true );
  QtUtils::QtBridge::Show( this->private_->ui_.bspline_widget_, tool->has_bspline_params_state_ );

  CORE_LOG_DEBUG( "Finished building a Resample Tool Interface" );

  return ( true );
} 

} // end namespace Seg3D
