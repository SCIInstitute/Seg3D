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

//Interface Includes
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/BinaryDilateErodeFilterInterface.h>
#include "ui_BinaryDilateErodeFilterInterface.h"

//Application Includes
#include <Application/Tools/BinaryDilateErodeFilter.h>
//#include <Application/Filters/Actions/ActionBinaryDilateErode.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(BinaryDilateErodeFilterInterface)

class BinaryDilateErodeFilterInterfacePrivate
{
public:
  Ui::BinaryDilateErodeFilterInterface ui_;
  
    Core::QtSliderIntCombo *erode_;
  Core::QtSliderIntCombo *dilate_;
  TargetComboBox *target_;
};

// constructor
BinaryDilateErodeFilterInterface::BinaryDilateErodeFilterInterface() :
  private_( new BinaryDilateErodeFilterInterfacePrivate )
{
}

// destructor
BinaryDilateErodeFilterInterface::~BinaryDilateErodeFilterInterface()
{
}

// build the interface and connect it to the state manager
bool BinaryDilateErodeFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  // add sliderspinnercombo's
  this->private_->erode_ = new Core::QtSliderIntCombo();
  this->private_->ui_.erodeHLayout_bottom->addWidget( this->private_->erode_ );

  this->private_->dilate_ = new Core::QtSliderIntCombo();
  this->private_->ui_.dialateHLayout_bottom->addWidget( this->private_->dilate_ );
  
  this->private_->target_ = new TargetComboBox( this );
  this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  BinaryDilateErodeFilter* tool = dynamic_cast< BinaryDilateErodeFilter* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
  
  // set the defaults for erode
  int erode_min = 0; 
  int erode_max = 0;
  int erode_step = 0;
  tool->erode_state_->get_step( erode_step );
  tool->erode_state_->get_range( erode_min, erode_max );
  this->private_->erode_->setStep( erode_step );
  this->private_->erode_->setRange( erode_min, erode_max );
  this->private_->erode_->setCurrentValue( tool->erode_state_->get() );
  
  // set the defaults for dialate
  int dilate_min = 0; 
  int dilate_max = 0;
  int dilate_step = 0;
  tool->dilate_state_->get_step( dilate_step );
  tool->dilate_state_->get_range( dilate_min, dilate_max );
  this->private_->dilate_->setStep( dilate_step );
  this->private_->dilate_->setRange( dilate_min, dilate_max );
  this->private_->dilate_->setCurrentValue( tool->dilate_state_->get() );

  // set the default for the replace state
  this->private_->ui_.replaceCheckBox->setChecked( tool->replace_state_->get() );


  //Step 4 - connect the gui to the tool through the QtBridge
  Core::QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  this->connect( this->private_->target_, SIGNAL( valid( bool ) ), 
    this, SLOT( enable_run_filter( bool ) ) );
  Core::QtBridge::Connect( this->private_->erode_, tool->erode_state_ );
  Core::QtBridge::Connect( this->private_->dilate_, tool->dilate_state_ );
  Core::QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  this->connect( this->private_->ui_.runFilterButton, 
    SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  
  this->private_->target_->sync_layers();

  //Send a message to the log that we have finised with building the Binary Dialate Erode Filter Interface
  CORE_LOG_DEBUG("Finished building a Binary Dilate Erode Filter Interface");
  return ( true );

} // end build_widget

void BinaryDilateErodeFilterInterface::enable_run_filter( bool valid )
{
  if( valid )
    this->private_->ui_.runFilterButton->setEnabled( true );
  else
    this->private_->ui_.runFilterButton->setEnabled( false );
}

void BinaryDilateErodeFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  BinaryDilateErodeFilter* tool =
    dynamic_cast< BinaryDilateErodeFilter* > ( base_tool_.get() );

//  ActionBinaryDilateErode::Dispatch( tool->target_layer_state_->export_to_string(), 
//    tool->dilate_state_->get(), tool->erode_state_->get(), tool->replace_state_->get() ); 
}

} // end namespace Seg3D
