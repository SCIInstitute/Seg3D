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

//Qt Gui Includes
#include <Interface/ToolInterface/ArithmeticFilterInterface.h>
#include "ui_ArithmeticFilterInterface.h"

//Application Includes
#include <Application/Tools/ArithmeticFilter.h>
//#include <Application/Filters/Actions/ActionArithmetic.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, ArithmeticFilterInterface )

namespace Seg3D
{

class ArithmeticFilterInterfacePrivate
{
public:
  Ui::ArithmeticFilterInterface ui_;
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
  
  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ArithmeticFilter* tool = dynamic_cast< ArithmeticFilter* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_group_, 
    tool->target_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_group_, 
    tool->use_active_group_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.input_a_, tool->input_layers_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.input_b_, tool->input_layers_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.input_c_, tool->input_layers_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.input_d_, tool->input_layers_state_[ 3 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.expressions_, tool->expressions_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.predefined_expressions_,
    tool->predefined_expressions_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.output_type_, tool->output_type_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replace_, tool->replace_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.execute_button_, boost::bind(
    &ArithmeticFilter::execute, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_group_, tool->use_active_group_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.input_a_, tool->use_active_group_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.output_type_, tool->replace_state_, true );

  connect( this->private_->ui_.predefined_expressions_, SIGNAL( itemDoubleClicked( 
    QListWidgetItem* ) ), SLOT( set_expressions_text( QListWidgetItem* ) ) );

  //Send a message to the log that we have finished with building the Arithmetic Filter
  CORE_LOG_DEBUG("Finished building an Arithmetic Filter Interface");

  return true;
} // end build_widget

void ArithmeticFilterInterface::set_expressions_text( QListWidgetItem* item )
{
  this->private_->ui_.expressions_->setPlainText( item->text() + ";" );
}

} //end seg3d
