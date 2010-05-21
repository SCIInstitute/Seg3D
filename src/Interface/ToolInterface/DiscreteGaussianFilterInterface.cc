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
#include <QtInterface/Bridge/QtBridge.h>

//Interface Includes
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/DiscreteGaussianFilterInterface.h>
#include "ui_DiscreteGaussianFilterInterface.h"

//Application Includes
#include <Application/Tools/DiscreteGaussianFilter.h>
//#include <Application/Filters/Actions/ActionDiscreteGaussian.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(DiscreteGaussianFilterInterface)

class DiscreteGaussianFilterInterfacePrivate
{
public:
  Ui::DiscreteGaussianFilterInterface ui_;
    QtUtils::QtSliderDoubleCombo *variance_;
  QtUtils::QtSliderDoubleCombo *kernel_width_;
  TargetComboBox *target_;
};

// constructor
DiscreteGaussianFilterInterface::DiscreteGaussianFilterInterface() :
  private_( new DiscreteGaussianFilterInterfacePrivate )
{
}

// destructor
DiscreteGaussianFilterInterface::~DiscreteGaussianFilterInterface()
{
}

// build the interface and connect it to the state manager
bool DiscreteGaussianFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  //Add the SliderSpinCombos
  this->private_->variance_ = new QtUtils::QtSliderDoubleCombo();
  this->private_->ui_.varianceHLayout_bottom->addWidget( this->private_->variance_ );

  this->private_->kernel_width_ = new QtUtils::QtSliderDoubleCombo();
  this->private_->ui_.kernelHLayout_bottom->addWidget( this->private_->kernel_width_ );
  
  this->private_->target_ = new TargetComboBox( this );
  this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  DiscreteGaussianFilter* tool = dynamic_cast< DiscreteGaussianFilter* > ( base_tool_.get() );

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
  
  // set the defaults for the kernel width
  double kernel_width_min = 0.0; 
  double kernel_width_max = 0.0;
  double kernel_width_step = 0.0;
  tool->maximum_kernel_width_state_->get_step( kernel_width_step );
  tool->maximum_kernel_width_state_->get_range( kernel_width_min, kernel_width_max );
  this->private_->kernel_width_->setStep( kernel_width_step );
  this->private_->kernel_width_->setRange( kernel_width_min, kernel_width_max );
  this->private_->kernel_width_->setCurrentValue( tool->maximum_kernel_width_state_->get() );
  
  //set the default replace checkbox value
  this->private_->ui_.replaceCheckBox->setChecked( tool->replace_state_->get() );
        
  //Step 4 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  connect( this->private_->target_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  QtUtils::QtBridge::Connect( this->private_->variance_, tool->variance_state_ );
  QtUtils::QtBridge::Connect( this->private_->kernel_width_, tool->maximum_kernel_width_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  this->connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );

  this->private_->target_->sync_layers();

  //Send a message to the log that we have finised with building the Discrete Gaussian Filter Interface
  CORE_LOG_DEBUG("Finished building a Discrete Gaussian Filter Interface");
  return ( true );
} // end build_widget

void DiscreteGaussianFilterInterface::enable_run_filter( bool valid )
{
  if( valid )
    this->private_->ui_.runFilterButton->setEnabled( true );
  else
    this->private_->ui_.runFilterButton->setEnabled( false );
}

void DiscreteGaussianFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  DiscreteGaussianFilter* tool =
    dynamic_cast< DiscreteGaussianFilter* > ( base_tool_.get() );

//  ActionDiscreteGaussian::Dispatch( tool->target_layer_state_->export_to_string(), 
//    tool->variance_state_->get(), tool->maximum_kernel_width_state_->get(),
//    tool->replace_state_->get() ); 
}


} // end namespace Seg3D
