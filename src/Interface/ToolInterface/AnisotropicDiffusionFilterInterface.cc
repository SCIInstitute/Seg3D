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

//Qt Gui Includes
#include <Interface/ToolInterface/AnisotropicDiffusionFilterInterface.h>
#include "ui_AnisotropicDiffusionFilterInterface.h"

// Core includes
#include <Core/Utils/Log.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/AnisotropicDiffusionFilter.h>
//#include <Application/Filters/Actions/ActionAnisotropicDiffusion.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, AnisotropicDiffusionFilterInterface )

namespace Seg3D
{

class AnisotropicDiffusionFilterInterfacePrivate
{
public:
  Ui::AnisotropicDiffusionFilterInterface ui_;
  QtUtils::QtSliderIntCombo *iterations_;
  QtUtils::QtSliderIntCombo *step_;
  QtUtils::QtSliderDoubleCombo *conductance_;
};

// constructor
AnisotropicDiffusionFilterInterface::AnisotropicDiffusionFilterInterface() :
  private_( new AnisotropicDiffusionFilterInterfacePrivate )
{
}

// destructor
AnisotropicDiffusionFilterInterface::~AnisotropicDiffusionFilterInterface()
{
}

// build the interface and connect it to the state manager
bool AnisotropicDiffusionFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
    
  this->private_->iterations_ = new QtUtils::QtSliderIntCombo();
  this->private_->ui_.iterationsHLayout_bottom->addWidget( this->private_->iterations_ );

  this->private_->step_ = new QtUtils::QtSliderIntCombo();
  this->private_->ui_.integrationHLayout_bottom->addWidget( this->private_->step_ );

  this->private_->conductance_ = new QtUtils::QtSliderDoubleCombo();
  this->private_->ui_.conductanceHLayout_bottom->addWidget( this->private_->conductance_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  AnisotropicDiffusionFilter* tool =
      dynamic_cast< AnisotropicDiffusionFilter* > ( base_tool_.get() );
  
    //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_mask_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->iterations_, 
    tool->iterations_state_ );
  QtUtils::QtBridge::Connect( this->private_->step_, 
    tool->steps_state_ );
  QtUtils::QtBridge::Connect( this->private_->conductance_, 
    tool->conductance_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, 
    tool->replace_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton,
    tool->valid_target_state_ );
  
  this->private_->ui_.target_mask_->setDisabled( tool->use_active_layer_state_->get() );
  this->connect( this->private_->ui_.use_active_layer_, SIGNAL( toggled( bool ) ),
    this->private_->ui_.target_mask_, SLOT( setDisabled( bool ) ) );

  this->connect( this->private_->ui_.runFilterButton, 
    SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  
  return true;
}

void AnisotropicDiffusionFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  AnisotropicDiffusionFilter* tool =
    dynamic_cast< AnisotropicDiffusionFilter* > ( base_tool_.get() );

//  ActionAnisotropicDiffusion::Dispatch( tool->target_layer_state_->export_to_string(), 
//    tool->iterations_state_->get(), tool->steps_state_->get(),
//    tool->conductance_state_->get(), tool->replace_state_->get() ); 
}


} // end namespace Seg3D

