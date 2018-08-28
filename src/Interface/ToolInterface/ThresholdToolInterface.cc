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

//Interface Includes
#include <Interface/ToolInterface/ThresholdToolInterface.h>
#include "ui_ThresholdToolInterface.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/ThresholdTool.h>
#include <Application/Layer/LayerManager.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, ThresholdToolInterface )

namespace Seg3D
{

class ThresholdToolInterfacePrivate
{
public:
  Ui::ThresholdToolInterface ui_;
};

// constructor
ThresholdToolInterface::ThresholdToolInterface() :
  private_( new ThresholdToolInterfacePrivate )
{
}

// destructor
ThresholdToolInterface::~ThresholdToolInterface()
{
  this->disconnect_all();
}

// build the interface and connect it to the state manager
bool ThresholdToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.verticalLayout_5->setAlignment( Qt::AlignTop );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ThresholdTool* tool = dynamic_cast< ThresholdTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  connect( this->private_->ui_.target_layer_, SIGNAL( currentIndexChanged( QString ) ), 
    this, SLOT( refresh_histogram( QString ) ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.upper_threshold_, tool->upper_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.lower_threshold_, tool->lower_threshold_state_ );
  
  // Connect the thresholds so that they keep in sync
  this->private_->ui_.lower_threshold_->connect_min( this->private_->ui_.upper_threshold_ );
  this->private_->ui_.upper_threshold_->connect_max( this->private_->ui_.lower_threshold_ );
  
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.show_preview_checkbox_, 
    tool->show_preview_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.preview_opacity_slider_, 
    tool->preview_opacity_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.preview_opacity_slider_, tool->show_preview_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.clear_seeds_button_, boost::bind(
    &SeedPointsTool::clear, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.run_button_, boost::bind(
    &ThresholdTool::execute, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Enable( this->private_->ui_.run_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_layer_, 
    tool->use_active_layer_state_, true ); 
  
  this->private_->ui_.upper_threshold_->set_description( "Upper" );
  this->private_->ui_.lower_threshold_->set_description( "Lower" );
  this->private_->ui_.preview_opacity_slider_->set_description( "Opacity" );

  
  QtUtils::QtBridge::Enable( this->private_->ui_.histogram_, tool->valid_target_state_ );
  this->private_->ui_.histogram_->setToolTip("Left mouse sets the lower threshold, right mouse sets the upper");

  boost::function< bool () > condition = boost::lambda::bind( &Core::StateLabeledOption::get, 
    tool->target_layer_state_.get() ) != Tool::NONE_OPTION_C;
  QtUtils::QtBridge::Enable( this->private_->ui_.upper_threshold_, 
    tool->target_layer_state_, condition );
  QtUtils::QtBridge::Enable( this->private_->ui_.lower_threshold_,
    tool->target_layer_state_, condition );
  
  // Finally we set the thresholds to the histogram
  this->private_->ui_.histogram_->set_thresholds( this->private_->ui_.upper_threshold_, 
    this->private_->ui_.lower_threshold_ );
  
  //Send a message to the log that we have finished with building the Threshold Tool Interface
  CORE_LOG_DEBUG( "Finished building a Threshold Tool Interface" );

  return ( true );
} 

void ThresholdToolInterface::refresh_histogram( QString layer_name )
{
  if( layer_name == "" || 
    layer_name == Tool::NONE_OPTION_C.c_str() )
  {
    this->private_->ui_.histogram_->hide_threshold_bars();
    return;
  }

  DataLayerHandle data_layer = boost::dynamic_pointer_cast< DataLayer >(
    LayerManager::Instance()->find_layer_by_name( layer_name.toStdString() ) );
  if ( !data_layer )
  {
    return;
  }
  
  this->private_->ui_.histogram_->set_histogram( data_layer->get_data_volume()->
    get_data_block()->get_histogram() );
  
  this->private_->ui_.histogram_->show_threshold_bars();
}

} // end namespace Seg3D
