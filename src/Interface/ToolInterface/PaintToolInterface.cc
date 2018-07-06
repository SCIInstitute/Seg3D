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

// Qt includes
#include <QComboBox>

// Core includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/Log.h>

//Qt Gui Includes
#include <Interface/ToolInterface/PaintToolInterface.h>
#include "ui_PaintToolInterface.h"

//Application Includes
#include <Application/Tools/PaintTool.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, PaintToolInterface )

namespace Seg3D
{

class PaintToolInterfacePrivate
{
public:
  Ui::PaintToolInterface ui_;
};

// constructor
PaintToolInterface::PaintToolInterface() :
  private_( new PaintToolInterfacePrivate )
{
}

// destructor
PaintToolInterface::~PaintToolInterface()
{
}

// build the interface and connect it to the state manager
bool PaintToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  
  this->private_->ui_.horizontalLayout->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.horizontalLayout_3->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.horizontalLayout_8->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  PaintTool* tool = dynamic_cast< PaintTool* > ( base_tool_.get() );
  
  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_mask_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.mask_constraint1_, 
    tool->mask_constraint1_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.negative_mask_constraint1_,
    tool->negative_mask_constraint1_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.mask_constraint2_, 
    tool->mask_constraint2_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.negative_mask_constraint2_,
    tool->negative_mask_constraint2_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.data_constraint_, 
    tool->data_constraint_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.negative_data_constraint_,
    tool->negative_data_constraint_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.brush_radius_, 
    tool->brush_radius_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.upper_threshold_, 
    tool->upper_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.lower_threshold_, 
    tool->lower_threshold_state_ );

  this->private_->ui_.lower_threshold_->connect_min( this->private_->ui_.upper_threshold_ );
  this->private_->ui_.upper_threshold_->connect_max( this->private_->ui_.lower_threshold_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.eraseCheckBox, 
    tool->erase_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.show_boundary_,
    tool->show_data_cstr_bound_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_mask_,
    tool->use_active_layer_state_, true );
  QtUtils::QtBridge::Connect( this->private_->ui_.floodfill_button_, boost::bind(
    &PaintTool::flood_fill, tool, Core::Interface::GetWidgetActionContext(), false ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.flooderase_button_, boost::bind(
    &PaintTool::flood_fill, tool, Core::Interface::GetWidgetActionContext(), true ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.clear_seeds_button_, boost::bind(
    &SeedPointsTool::clear, tool, Core::Interface::GetWidgetActionContext() ) );
  
  this->private_->ui_.brush_radius_->set_description( "Radius" );
  this->private_->ui_.upper_threshold_->set_description( "Upper" );
  this->private_->ui_.lower_threshold_->set_description( "Lower" );
  

  //this->private_->ui_.message_alert_->set_warning_message( "Select a mask layer to activate this tool." );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  
  boost::function< bool () > condition = boost::lambda::bind( &Core::StateLabeledOption::get, 
    tool->data_constraint_layer_state_.get() ) != Tool::NONE_OPTION_C;
  
  QtUtils::QtBridge::Enable( this->private_->ui_.upper_threshold_, 
    tool->data_constraint_layer_state_, condition );
  
  QtUtils::QtBridge::Enable( this->private_->ui_.lower_threshold_,
    tool->data_constraint_layer_state_, condition );
    
  QtUtils::QtBridge::Show( this->private_->ui_.data_constraint_widget_, 
    tool->data_constraint_layer_state_, condition );
  
  condition = boost::lambda::bind( &Core::StateLabeledOption::get, 
    tool->mask_constraint1_layer_state_.get() ) != Tool::NONE_OPTION_C;
    
  QtUtils::QtBridge::Show( this->private_->ui_.mask_constraint_1_widget_, 
    tool->mask_constraint1_layer_state_, condition );
  
  condition = boost::lambda::bind( &Core::StateLabeledOption::get, 
    tool->mask_constraint2_layer_state_.get() ) != Tool::NONE_OPTION_C;
    
  QtUtils::QtBridge::Show( this->private_->ui_.mask_constraint_2_widget_, 
    tool->mask_constraint2_layer_state_, condition );
  
#if defined ( __APPLE__ )
  this->private_->ui_.verticalLayout_8->setSpacing( 8 );
#endif

  return true;
} 

} // end namespace Seg3D

