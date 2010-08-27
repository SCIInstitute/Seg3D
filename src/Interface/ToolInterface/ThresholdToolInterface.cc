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

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Widgets/QtHistogramWidget.h>

//Interface Includes
#include <Interface/ToolInterface/ThresholdToolInterface.h>
#include "ui_ThresholdToolInterface.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/ThresholdTool.h>
#include <Application/LayerManager/LayerManager.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, ThresholdToolInterface )

namespace Seg3D
{

class ThresholdToolInterfacePrivate
{
public:
  Ui::ThresholdToolInterface ui_;
  
  QtUtils::QtSliderDoubleCombo *upper_threshold_;
  QtUtils::QtSliderDoubleCombo *lower_threshold_;
  QtUtils::QtHistogramWidget *histogram_;
};

// constructor
ThresholdToolInterface::ThresholdToolInterface() :
  private_( new ThresholdToolInterfacePrivate )
{
}

// destructor
ThresholdToolInterface::~ThresholdToolInterface()
{
}

// build the interface and connect it to the state manager
bool ThresholdToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  // Add the SliderSpinner Combos
  this->private_->upper_threshold_ = new QtUtils::QtSliderDoubleCombo();
  this->private_->ui_.verticalLayout_2->addWidget( this->private_->upper_threshold_ );

  this->private_->lower_threshold_ = new QtUtils::QtSliderDoubleCombo();
  this->private_->ui_.verticalLayout_3->addWidget( this->private_->lower_threshold_ );
    
  this->private_->histogram_ = new QtUtils::QtHistogramWidget( this );
  this->private_->ui_.histogramHLayout->addWidget( this->private_->histogram_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ThresholdTool* tool = dynamic_cast< ThresholdTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  connect( this->private_->ui_.target_layer_, SIGNAL( currentIndexChanged( QString ) ), 
    this, SLOT( refresh_histogram( QString ) ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->upper_threshold_, tool->upper_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->lower_threshold_, tool->lower_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.clear_seeds_button_, boost::bind(
    &SeedPointsTool::clear, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.run_button_, boost::bind(
    &ThresholdTool::execute, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Enable( this->private_->ui_.run_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_layer_, 
    tool->use_active_layer_state_, true ); 

  //Send a message to the log that we have finished with building the Threshold Tool Interface
  CORE_LOG_DEBUG("Finished building a Threshold Tool Interface");

  return ( true );
} 

void ThresholdToolInterface::refresh_histogram( QString layer_name )
{
  if( layer_name == "" || 
    layer_name == Tool::NONE_OPTION_C.c_str() )
  {
    return;
  }

  DataLayerHandle data_layer = boost::dynamic_pointer_cast< DataLayer >(
    LayerManager::Instance()->get_layer_by_name( layer_name.toStdString() ) );
  if ( !data_layer )
  {
    return;
  }
  
  this->private_->histogram_->set_histogram( data_layer->get_data_volume()->
    get_data_block()->get_histogram() );  
}

} // end namespace Seg3D
