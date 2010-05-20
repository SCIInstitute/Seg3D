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
#include <Interface/ToolInterface/ArithmeticFilterInterface.h>
#include "ui_ArithmeticFilterInterface.h"

//Application Includes
#include <Application/Tools/ArithmeticFilter.h>
//#include <Application/Filters/Actions/ActionArithmetic.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(ArithmeticFilterInterface)

class ArithmeticFilterInterfacePrivate
{
public:
  Ui::ArithmeticFilterInterface ui_;
  TargetComboBox *volume_a_;
  TargetComboBox *volume_b_;
  TargetComboBox *volume_c_;
};

// constructor
ArithmeticFilterInterface::ArithmeticFilterInterface() :
  private_( new ArithmeticFilterInterfacePrivate )
{
}

// destructor
ArithmeticFilterInterface::~ArithmeticFilterInterface()
{
}

// build the interface and connect it to the state manager
bool ArithmeticFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  
  this->private_->volume_a_ = new TargetComboBox( this );
  this->private_->volume_a_->setMinimumHeight( 26 );
  this->private_->ui_.volumeALayout->addWidget( this->private_->volume_a_ );
  
  this->private_->volume_b_ = new TargetComboBox( this );
  this->private_->volume_b_->setMinimumHeight( 26 );
  this->private_->ui_.volumeBLayout->addWidget( this->private_->volume_b_ );

  this->private_->volume_c_ = new TargetComboBox( this );
  this->private_->volume_c_->setMinimumHeight( 26 );
  this->private_->ui_.volumeCLayout->addWidget( this->private_->volume_c_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ArithmeticFilter* tool = dynamic_cast< ArithmeticFilter* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  Core::QtBridge::Connect( this->private_->volume_a_, tool->volume_a_state_ );
  Core::QtBridge::Connect( this->private_->volume_b_, tool->volume_b_state_ );
  Core::QtBridge::Connect( this->private_->volume_c_, tool->volume_c_state_ );
  this->connect( this->private_->volume_a_, SIGNAL( valid( bool ) ), 
    this, SLOT( enable_run_filter( bool ) ) );
  
  Core::QtBridge::Connect( this->private_->ui_.exampleExpComboBox, tool->example_expressions_state_ );
  Core::QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  this->connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), 
    this, SLOT( execute_filter() ) );
  
  this->private_->volume_a_->sync_layers();
  this->private_->volume_b_->sync_layers();
  this->private_->volume_c_->sync_layers();

  //Send a message to the log that we have finised with building the Arithmetic Filter
  CORE_LOG_DEBUG("Finished building an Arithmetic Filter Interface");

  return ( true );
} // end build_widget

void ArithmeticFilterInterface::enable_run_filter( bool valid )
{
  if( valid )
    this->private_->ui_.runFilterButton->setEnabled( true );
  else
    this->private_->ui_.runFilterButton->setEnabled( false );
}

void ArithmeticFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  ArithmeticFilter* tool =
    dynamic_cast< ArithmeticFilter* > ( base_tool_.get() );

//  ActionArithmetic::Dispatch( tool->volume_a_state_->export_to_string(), 
//    tool->volume_b_state_->export_to_string(), tool->volume_c_state_->export_to_string(),
//    tool->example_expressions_state_->get(), tool->replace_state_->get() ); 
}

} //end seg3d
