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

//Boost Includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Core includes
#include <Core/Interface/Interface.h>

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
  
  std::vector< std::pair< std::string, std::string > > predefined_;
};

// constructor
ArithmeticFilterInterface::ArithmeticFilterInterface() :
  private_( new ArithmeticFilterInterfacePrivate )
{
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean AND Filter", " RESULT = A && B;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean AND Filter (3 way)", " RESULT = A && B && C;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean AND Filter (4 way)", " RESULT = A && B && C && D;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean OR Filter", " RESULT = A || B;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean OR Filter (3 way)", " RESULT = A || B || C;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean OR Filter (4 way)", " RESULT = A || B || C || D;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean NOT Filter", " RESULT = ! A;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean XOR Filter", " RESULT = xor( A, B );") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Boolean REMOVE Filter", " RESULT = A - B;") );
  this->private_->predefined_.push_back( std::pair< std::string, std::string >(
    "Mask Data", " RESULT = A * B;") );
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
  this->private_->ui_.horizontalLayout_7->setAlignment( Qt::AlignHCenter );
  
  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ArithmeticFilter* tool = dynamic_cast< ArithmeticFilter* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.input_a_, tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.input_b_, tool->input_b_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.input_c_, tool->input_c_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.input_d_, tool->input_d_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.expressions_, tool->expressions_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.output_type_, tool->output_type_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replace_, tool->replace_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.preserve_checkbox_, 
    tool->preserve_data_format_state_ );

  // Only enable replace checkbox if input type == output type (data/mask)
  QtUtils::QtBridge::Enable( this->private_->ui_.replace_, 
    tool->input_matches_output_state_ );

  // Only enable preserve data type checkbox if output is data and input type == output type
  std::vector< Core::StateBaseHandle > enable_states( 2 );
  enable_states[ 0 ] = tool->input_matches_output_state_;
  enable_states[ 1 ] = tool->output_type_state_;
  boost::function< bool () > condition = boost::lambda::bind( &Core::StateBool::get, 
    tool->input_matches_output_state_.get() ) && boost::lambda::bind( &Core::StateBool::get,
    tool->output_is_data_state_.get() );
  
  QtUtils::QtBridge::Enable( this->private_->ui_.preserve_checkbox_, enable_states, condition );
  
  QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton,tool->valid_target_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );

  connect( this->private_->ui_.predefined_expressions_, SIGNAL( currentIndexChanged( 
    int ) ), SLOT( set_predefined_text( int ) ) );
  
  // Step 4 - Qt connections
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() ); 
    this->private_->ui_.input_a_->setDisabled( tool->use_active_layer_state_->get() );
  
    this->connect( this->private_->ui_.use_active_layer_, SIGNAL( toggled( bool ) ),
      this->private_->ui_.input_a_, SLOT( setDisabled( bool ) ) );

    this->connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), 
      this, SLOT( run_filter() ) );
  }

  for ( size_t j = 0; j < this->private_->predefined_.size(); j++ )
  {
    this->private_->ui_.predefined_expressions_->addItem( QString::fromStdString( 
      this->private_->predefined_[ j ].first ) );
  }

  return true;
  
} // end build_widget

void ArithmeticFilterInterface::set_predefined_text( int index )
{
  this->private_->ui_.expressions_->setPlainText( 
    QString::fromStdString( this->private_->predefined_[ index ].second ) );
}

void ArithmeticFilterInterface::run_filter()
{
  tool()->execute( Core::Interface::GetWidgetActionContext() );
}

} //end seg3d
