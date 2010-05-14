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

//Interface Includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/CannyEdgeDetectionFilterInterface.h>
#include "ui_CannyEdgeDetectionFilterInterface.h"

//Application Includes
#include <Application/Tools/CannyEdgeDetectionFilter.h>
//#include <Application/Filters/Actions/ActionCannyEdgeDetection.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(CannyEdgeDetectionFilterInterface)

class CannyEdgeDetectionFilterInterfacePrivate
{
public:
  Ui::CannyEdgeDetectionFilterInterface ui_;
  
  SliderDoubleCombo *variance_;
  SliderDoubleCombo *max_error_;
  SliderDoubleCombo *threshold_;
  TargetComboBox *target_;
};

// constructor
CannyEdgeDetectionFilterInterface::CannyEdgeDetectionFilterInterface() :
  private_( new CannyEdgeDetectionFilterInterfacePrivate )
{
}

// destructor
CannyEdgeDetectionFilterInterface::~CannyEdgeDetectionFilterInterface()
{
}

// build the interface and connect it to the state manager
bool CannyEdgeDetectionFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  //Add the SliderSpinCombos
  this->private_->variance_ = new SliderDoubleCombo();
  this->private_->ui_.varianceHLayout_bottom->addWidget( this->private_->variance_ );

  this->private_->max_error_ = new SliderDoubleCombo();
  this->private_->ui_.errorHLayout_bottom->addWidget( this->private_->max_error_ );

  this->private_->threshold_ = new SliderDoubleCombo();
  this->private_->ui_.thresholdHLayout_bottom->addWidget( this->private_->threshold_ );
  
  this->private_->target_ = new TargetComboBox( this );
  this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );


  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  CannyEdgeDetectionFilter* tool = dynamic_cast< CannyEdgeDetectionFilter* > ( base_tool_.get() );

    //Step 3 - set the values for the tool ui from the state engine
  
  // set the defaults for the variance
  double variance_min = 0.0; 
  double variance_max = 0.0;
  double variance_step = 0.0;
  tool->variance_state_->get_step( variance_step );
  tool->variance_state_->get_range( variance_min, variance_max );
  this->private_->variance_->setStep( variance_step );
  this->private_->variance_->setRange( variance_min, variance_max );
  this->private_->variance_->setCurrentValue( tool->variance_state_->get() );
  
  // set the defaults for the max error
  double max_error_min = 0.0; 
  double max_error_max = 0.0;
  double max_error_step = 0.0;
  tool->max_error_state_->get_step( max_error_step );
  tool->max_error_state_->get_range( max_error_min, max_error_max );
  this->private_->max_error_->setStep( max_error_step );
  this->private_->max_error_->setRange( max_error_min, max_error_max );
  this->private_->max_error_->setCurrentValue( tool->max_error_state_->get() );

  // set the defaults for the threshold
  double threshold_min = 0.0; 
  double threshold_max = 0.0;
  double threshold_step = 0.0;
  tool->threshold_state_->get_step( threshold_step );
  tool->threshold_state_->get_range( threshold_min, threshold_max );
  this->private_->threshold_->setStep( threshold_step );
  this->private_->threshold_->setRange( threshold_min, threshold_max );
  this->private_->threshold_->setCurrentValue( tool->threshold_state_->get() );
  
  //set the default replace checkbox value
  this->private_->ui_.replaceCheckBox->setChecked(tool->replace_state_);

  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  connect( this->private_->target_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  QtBridge::Connect( this->private_->variance_, tool->variance_state_ );
  QtBridge::Connect( this->private_->max_error_, tool->max_error_state_ );
  QtBridge::Connect( this->private_->threshold_, tool->threshold_state_ );
  QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );

  this->private_->target_->sync_layers();

  //Send a message to the log that we have finised with building the Detection Filter Interface
  CORE_LOG_DEBUG("Finished building a Canny Edge Detection Filter Interface");

  return ( true );
}

void CannyEdgeDetectionFilterInterface::enable_run_filter( bool valid )
{
  if( valid )
    this->private_->ui_.runFilterButton->setEnabled( true );
  else
    this->private_->ui_.runFilterButton->setEnabled( false );
}

void CannyEdgeDetectionFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  CannyEdgeDetectionFilter* tool =
    dynamic_cast< CannyEdgeDetectionFilter* > ( base_tool_.get() );

//  ActionCannyEdgeDetection::Dispatch( tool->target_layer_state_->export_to_string(), 
//    tool->variance_state_->get(), tool->max_error_state_->get(),
//    tool->threshold_state_->get(), tool->replace_state_->get() ); 
}

} // namespace Seg3D

