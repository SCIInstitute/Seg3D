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

// Qt includes
#include <QComboBox>

// Core includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/Log.h>

//Qt Gui Includes
#include <Interface/ToolInterface/GrowCutToolInterface.h>
#include "ui_GrowCutToolInterface.h"

//Application Includes
#include <Application/Tools/GrowCutTool.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, GrowCutToolInterface )

namespace Seg3D
{

class GrowCutToolInterfacePrivate
{
public:
  Ui::GrowCutToolInterface ui_;
};

// constructor
GrowCutToolInterface::GrowCutToolInterface() :
  private_( new GrowCutToolInterfacePrivate )
{}

// destructor
GrowCutToolInterface::~GrowCutToolInterface()
{}

// build the interface and connect it to the state manager
bool GrowCutToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  this->private_->ui_.horizontalLayout->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.horizontalLayout_3->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  GrowCutTool* tool = dynamic_cast< GrowCutTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_data_layer_, tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.foreground_mask_, tool->foreground_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.background_mask_, tool->background_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.output_mask_, tool->output_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.brush_radius_, tool->brush_radius_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.eraseCheckBox_, tool->erase_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_data_layer_, tool->use_active_layer_state_, true );

  this->private_->ui_.brush_radius_->set_description( "Radius" );

  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  QtUtils::QtBridge::Show( this->private_->ui_.message_warning_, tool->registration_warning_state_, false );

  QtUtils::QtBridge::Enable( this->private_->ui_.run_growcut_, tool->valid_target_state_ );

  QtUtils::QtBridge::Enable( this->private_->ui_.run_smoothing_, tool->valid_target_state_ );

  // enable "initialize masks" only when a valid data layer is selected
  boost::function< bool() > condition = boost::lambda::bind( &Core::StateLabeledOption::get,
                                                             tool->target_layer_state_.get() ) != Tool::NONE_OPTION_C;

  QtUtils::QtBridge::Enable( this->private_->ui_.initialize_masks_, tool->target_layer_state_, condition );

  QtUtils::QtBridge::Connect( this->private_->ui_.run_growcut_, boost::bind(
                                &GrowCutTool::run_growcut, tool, Core::Interface::GetWidgetActionContext(), false ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.initialize_masks_, boost::bind(
                                &GrowCutTool::initialize_layers, tool, Core::Interface::GetWidgetActionContext() ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.run_smoothing_, boost::bind(
                                &GrowCutTool::run_smoothing, tool, Core::Interface::GetWidgetActionContext() ) );

#if defined ( __APPLE__ )
  this->private_->ui_.verticalLayout_8->setSpacing( 8 );
#endif

  return true;
}
} // end namespace Seg3D
