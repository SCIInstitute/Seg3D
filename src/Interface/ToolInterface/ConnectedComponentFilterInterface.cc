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
#include <Interface/ToolInterface/ConnectedComponentFilterInterface.h>
#include "ui_ConnectedComponentFilterInterface.h"

//Application Includes
#include <Application/Tools/ConnectedComponentFilter.h>
//#include <Application/Filters/Actions/ActionConnectedComponent.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(ConnectedComponentFilterInterface)

class ConnectedComponentFilterInterfacePrivate
{
public:
  Ui::ConnectedComponentFilterInterface ui_;
  TargetComboBox *target_;
};

// constructor
ConnectedComponentFilterInterface::ConnectedComponentFilterInterface() :
  private_( new ConnectedComponentFilterInterfacePrivate )
{
}

// destructor
ConnectedComponentFilterInterface::~ConnectedComponentFilterInterface()
{
}

// build the interface and connect it to the state manager
bool ConnectedComponentFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  
    this->private_->target_ = new TargetComboBox( this );
    this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ConnectedComponentFilter* tool = dynamic_cast< ConnectedComponentFilter* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
  
  // N/A

    //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  connect( this->private_->target_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  
  connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  
  this->private_->target_->sync_layers();

  //Send a message to the log that we have finised with building the Connected Component Filter Interface
  CORE_LOG_DEBUG("Finished building a Connected Component Filter Interface");
  return ( true );

} // end build_widget
  
void ConnectedComponentFilterInterface::enable_run_filter( bool valid )
{
  if( valid )
    this->private_->ui_.runFilterButton->setEnabled( true );
  else
    this->private_->ui_.runFilterButton->setEnabled( false );
}

void ConnectedComponentFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  ConnectedComponentFilter* tool =
  dynamic_cast< ConnectedComponentFilter* > ( base_tool_.get() );
  
//  ActionConnectedComponent::Dispatch( tool->target_layer_state_->export_to_string() ); 
}
  

} // namespace Seg3D
