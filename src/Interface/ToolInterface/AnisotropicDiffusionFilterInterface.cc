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
#include <Interface/ToolInterface/AnisotropicDiffusionFilterInterface.h>
#include "ui_AnisotropicDiffusionFilterInterface.h"

//Application Includes
#include <Application/Filters/Actions/ActionAnisotropicDiffusion.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(AnisotropicDiffusionFilterInterface)

class AnisotropicDiffusionFilterInterfacePrivate
{
public:
  Ui::AnisotropicDiffusionFilterInterface ui_;
  SliderIntCombo *iterations_;
  SliderIntCombo *step_;
  SliderDoubleCombo *conductance_;
  TargetComboBox *target_;
};

// constructor
AnisotropicDiffusionFilterInterface::AnisotropicDiffusionFilterInterface() :
  private_( new AnisotropicDiffusionFilterInterfacePrivate )
{
  //this->tool_ = boost::shared_dynamic_cast< boost::shared_ptr< AnisotropicDiffusionFilter > >( tool() );
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

  this->private_->iterations_ = new SliderIntCombo();
  this->private_->ui_.iterationsHLayout_bottom->addWidget( this->private_->iterations_ );

  this->private_->step_ = new SliderIntCombo();
  this->private_->ui_.integrationHLayout_bottom->addWidget( this->private_->step_ );

  this->private_->conductance_ = new SliderDoubleCombo();
  this->private_->ui_.conductanceHLayout_bottom->addWidget( this->private_->conductance_ );
  
  this->private_->target_ = new TargetComboBox( this );
  this->private_->ui_.activeHLayout->addWidget( private_->target_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  AnisotropicDiffusionFilter* tool =
      dynamic_cast< AnisotropicDiffusionFilter* > ( base_tool_.get() );
  
    //Step 3 - set the values for the tool ui from the state engine
      
      // set the defaults for the iterations from the state variables
        int iterations_min = 0; 
      int iterations_max = 0;
      int iterations_step = 0;
      tool->iterations_state_->get_step( iterations_step );
      tool->iterations_state_->get_range( iterations_min, iterations_max );
      this->private_->iterations_->setStep( iterations_step );
        this->private_->iterations_->setRange( iterations_min, iterations_max );
        this->private_->iterations_->setCurrentValue( tool->iterations_state_->get() );
        
        // set the defaults for the step from the state variables
        int step_min = 0; 
      int step_max = 0;
      int step_step = 0;
      tool->steps_state_->get_step( step_step );
      tool->steps_state_->get_range( step_min, step_max );
      this->private_->step_->setStep( step_step );
        this->private_->step_->setRange( step_min, step_max );
        this->private_->step_->setCurrentValue( tool->steps_state_->get() );
        
        // set the defaults for the conductance from the state variables
        double conductance_min = 0.0; 
      double conductance_max = 0.0;
      double conductance_step = 0.0;
      tool->conductance_state_->get_step( conductance_step );
      tool->conductance_state_->get_range( conductance_min, conductance_max );
      this->private_->conductance_->setStep( conductance_step );
        this->private_->conductance_->setRange( conductance_min, conductance_max );
        this->private_->conductance_->setCurrentValue( tool->conductance_state_->get() );
        
        //set the default replace checkbox value
        this->private_->ui_.replaceCheckBox->setChecked(tool->replace_state_->get());

      

  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  QtBridge::Connect( this->private_->iterations_, tool->iterations_state_ );
  QtBridge::Connect( this->private_->step_, tool->steps_state_ );
  QtBridge::Connect( this->private_->conductance_, tool->conductance_state_ );
  QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );

  //Send a message to the log that we have finised with building the Anisotropic Diffusion Filter Interface
  SCI_LOG_DEBUG("Finished building an Anisotropic Diffusion Filter Interface");

  return ( true );
} // end build_widget

void AnisotropicDiffusionFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  AnisotropicDiffusionFilter* tool =
    dynamic_cast< AnisotropicDiffusionFilter* > ( base_tool_.get() );

  ActionAnisotropicDiffusion::Dispatch( tool->target_layer_state_->export_to_string(), 
    tool->iterations_state_->get(), tool->steps_state_->get(),
    tool->conductance_state_->get(), tool->replace_state_->get() ); 
}


} // end namespace Seg3D

