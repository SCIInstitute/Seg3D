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

// Core includes
#include <Core/Interface/Interface.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Qt Gui Includes
#include <Interface/ToolInterface/FillHolesFilterInterface.h>
#include "ui_FillHolesFilterInterface.h"

//Application Includes
#include <Application/Tools/FillHolesFilter.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, FillHolesFilterInterface )

namespace Seg3D
{

class FillHolesFilterInterfacePrivate
{
public:
  Ui::FillHolesFilterInterface ui_;
};

// constructor
FillHolesFilterInterface::FillHolesFilterInterface() :
  private_( new FillHolesFilterInterfacePrivate )
{
}

// destructor
FillHolesFilterInterface::~FillHolesFilterInterface()
{
}

// build the interface and connect it to the state manager
bool FillHolesFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_3->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  FillHolesFilter* tool = dynamic_cast< FillHolesFilter* > ( this->tool().get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.clearSeedsButton, boost::bind( 
    &FillHolesFilter::clear, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );  
  
  QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton, tool->valid_target_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_layer_, 
    tool->use_active_layer_state_, true );
  QtUtils::QtBridge::Connect( this->private_->ui_.runFilterButton, boost::bind(
    &Tool::execute, tool, Core::Interface::GetWidgetActionContext() ) );

  return true;
}

} // end namespace Seg3D
