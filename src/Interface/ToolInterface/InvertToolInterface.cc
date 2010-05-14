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
#include <Interface/ToolInterface/InvertToolInterface.h>
#include "ui_InvertToolInterface.h"

//Application Includes
#include <Application/Tools/InvertTool.h>
//#include <Application/Filters/Actions/ActionInvert.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(InvertToolInterface)

class InvertToolInterfacePrivate
{
public:
  Ui::InvertToolInterface ui_;
  
  TargetComboBox *target_;
};

// constructor
InvertToolInterface::InvertToolInterface() :
  private_( new InvertToolInterfacePrivate )
{
}

// destructor
InvertToolInterface::~InvertToolInterface()
{
}

// build the interface and connect it to the state manager
bool InvertToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  
    this->private_->target_ = new TargetComboBox( this );
    this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  InvertTool* tool = dynamic_cast< InvertTool* > ( base_tool_.get() );
  
  //Step 3 - set the default values from the state variables
        //set the default replace checkbox value
        this->private_->ui_.replaceCheckBox->setChecked(tool->replace_state_);
  
  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  connect( this->private_->target_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  
  connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  
  this->private_->target_->sync_layers();



  //Send a message to the log that we have finised with building the Invert Tool Interface
  CORE_LOG_MESSAGE("Finished building an Invert Tool Interface");

  return ( true );
} // end build_widget
  
  void InvertToolInterface::enable_run_filter( bool valid )
  {
    if( valid )
      this->private_->ui_.runFilterButton->setEnabled( true );
    else
      this->private_->ui_.runFilterButton->setEnabled( false );
  }
  
  void InvertToolInterface::execute_filter()
  {
    ToolHandle base_tool_ = tool();
    InvertTool* tool =
    dynamic_cast< InvertTool* > ( base_tool_.get() );
    
//    ActionInvert::Dispatch( tool->target_layer_state_->export_to_string(), 
//                      tool->replace_state_->get() ); 
  }

} // end namespace Seg3D
