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

//QtInterface Includes
#include <QtInterface/Utils/QtBridge.h>
#include <QtInterface/Widgets/QtHistogramWidget.h>

//Interface Includes
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/ThresholdToolInterface.h>
#include "ui_ThresholdToolInterface.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/ThresholdTool.h>
//#include <Application/Filters/Actions/ActionThreshold.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(ThresholdToolInterface)

class ThresholdToolInterfacePrivate
{
public:
  Ui::ThresholdToolInterface ui_;
  
  Core::QtSliderDoubleCombo *upper_threshold_;
  Core::QtSliderDoubleCombo *lower_threshold_;
  TargetComboBox *target_;
  Core::QtHistogramWidget *histogram_;
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
  this->private_->upper_threshold_ = new Core::QtSliderDoubleCombo();
  this->private_->ui_.verticalLayout_2->addWidget( this->private_->upper_threshold_ );

  this->private_->lower_threshold_ = new Core::QtSliderDoubleCombo();
  this->private_->ui_.verticalLayout_3->addWidget( this->private_->lower_threshold_ );
  
  // add the TargetComboBox
  this->private_->target_ = new TargetComboBox( this );
  this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );
  
  this->private_->histogram_ = new Core::QtHistogramWidget( this );
  this->private_->ui_.histogramHLayout->addWidget( this->private_->histogram_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ThresholdTool* tool = dynamic_cast< ThresholdTool* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
      
  // set the defaults for the upper threshold
  double upper_threshold_min = 0.0; 
  double upper_threshold_max = 0.0;
  double upper_threshold_step = 0.0;
  tool->upper_threshold_state_->get_step( upper_threshold_step );
  tool->upper_threshold_state_->get_range( upper_threshold_min, upper_threshold_max );
  this->private_->upper_threshold_->setStep( upper_threshold_step );
  this->private_->upper_threshold_->setRange( upper_threshold_min, upper_threshold_max );
  this->private_->upper_threshold_->setCurrentValue( tool->upper_threshold_state_->get() );
  
  // set the defaults for the lower threshold
  double lower_threshold_min = 0.0; 
  double lower_threshold_max = 0.0;
  double lower_threshold_step = 0.0;
  tool->lower_threshold_state_->get_step( lower_threshold_step );
  tool->lower_threshold_state_->get_range( lower_threshold_min, lower_threshold_max );
  this->private_->lower_threshold_->setStep( lower_threshold_step );
  this->private_->lower_threshold_->setRange( lower_threshold_min, lower_threshold_max );
  this->private_->lower_threshold_->setCurrentValue( tool->lower_threshold_state_->get() );


  //Step 4 - connect the gui to the tool through the QtBridge
  Core::QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  connect( this->private_->target_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  connect( this->private_->target_, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( refresh_histogram( QString ) ) );
  Core::QtBridge::Connect( this->private_->upper_threshold_, tool->upper_threshold_state_ );
  Core::QtBridge::Connect( this->private_->lower_threshold_, tool->lower_threshold_state_ );
  
  //connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  this->private_->target_->sync_layers();

  //Send a message to the log that we have finised with building the Threshold Tool Interface
  CORE_LOG_DEBUG("Finished building a Threshold Tool Interface");

  return ( true );
} // end build_widget
  
void ThresholdToolInterface::enable_run_filter( bool valid )
{
  ///if( valid )
    //this->private_->ui_.runFilterButton->setEnabled( true );
  //else
    //this->private_->ui_.runFilterButton->setEnabled( false );
}

void ThresholdToolInterface::refresh_histogram( QString layer_name )
{
  if( layer_name == "" )
  {
    return;
  }

  Core::Histogram temp_histogram = dynamic_cast< DataLayer* >( LayerManager::Instance()->
    get_layer_by_name( layer_name.toStdString() ).get() )->
    get_data_volume()->data_block()->get_histogram();

  // Now, display histogram!
  this->private_->histogram_->set_histogram( temp_histogram );  
}

void ThresholdToolInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  ThresholdTool* tool =
  dynamic_cast< ThresholdTool* > ( base_tool_.get() );
  
//  ActionThreshold::Dispatch( tool->target_layer_state_->export_to_string(), 
//    tool->upper_threshold_state_->get(), tool->lower_threshold_state_->get() ); 
}

} // end namespace Seg3D
