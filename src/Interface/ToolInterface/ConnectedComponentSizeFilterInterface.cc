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

// QtGui includes
#include "ui_ConnectedComponentSizeFilterInterface.h"

// Application includes
#include <Application/Tools/ConnectedComponentSizeFilter.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interaface includes
#include <Interface/ToolInterface/ConnectedComponentSizeFilterInterface.h>


SCI_REGISTER_TOOLINTERFACE( Seg3D, ConnectedComponentSizeFilterInterface )

namespace Seg3D
{

class ConnectedComponentSizeFilterInterfacePrivate
{
public:
  Ui::ConnectedComponentSizeFilterInterface ui_;
};

// constructor
ConnectedComponentSizeFilterInterface::ConnectedComponentSizeFilterInterface() :
  private_( new ConnectedComponentSizeFilterInterfacePrivate )
{
}

// destructor
ConnectedComponentSizeFilterInterface::~ConnectedComponentSizeFilterInterface()
{
}

// build the interface and connect it to the state manager
bool ConnectedComponentSizeFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  
  //Step 2 - get a pointer to the tool
  ConnectedComponentSizeFilter* tool = 
    dynamic_cast< ConnectedComponentSizeFilter* > ( this->tool().get() );

    //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.log_scale_, 
    tool->log_scale_state_ );

  // Step 4 - Qt connections
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() ); 
    this->private_->ui_.target_layer_->setDisabled( tool->use_active_layer_state_->get() );
  
    this->connect( this->private_->ui_.use_active_layer_, SIGNAL( toggled( bool ) ),
      this->private_->ui_.target_layer_, SLOT( setDisabled( bool ) ) );

    this->connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), 
      this, SLOT( run_filter() ) );
  } 

  return true;
}
  
void ConnectedComponentSizeFilterInterface::run_filter()
{
  tool()->execute( Core::Interface::GetWidgetActionContext() );
}

} // namespace Seg3D
