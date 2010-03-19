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

//Qt Gui Includes
#include <Interface/ToolInterface/AnisotropicDiffusionFilterInterface.h>
#include "ui_AnisotropicDiffusionFilterInterface.h"

//Application Includes
#include <Application/Tools/AnisotropicDiffusionFilter.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(AnisotropicDiffusionFilterInterface)

class AnisotropicDiffusionFilterInterfacePrivate
{
public:
  Ui::AnisotropicDiffusionFilterInterface ui_;
  SliderSpinComboInt *iterationsAdjuster;
  SliderSpinComboInt *stepAdjuster;
  SliderSpinComboDouble *conductanceAdjuster;
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

  this->private_->iterationsAdjuster = new SliderSpinComboInt();
  this->private_->ui_.iterationsHLayout_bottom->addWidget( this->private_->iterationsAdjuster );

  this->private_->stepAdjuster = new SliderSpinComboInt();
  this->private_->ui_.integrationHLayout_bottom->addWidget( this->private_->stepAdjuster );

  this->private_->conductanceAdjuster = new SliderSpinComboDouble();
  this->private_->ui_.conductanceHLayout_bottom->addWidget( this->private_->conductanceAdjuster );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  AnisotropicDiffusionFilter* tool =
      dynamic_cast< AnisotropicDiffusionFilter* > ( base_tool_.get() );

    //Step 3 - set the values for the tool ui from the state engine
  
      //set default falues for the target option list 
      std::vector< std::string > temp_option_list = tool->target_layer_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.targetComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.targetComboBox->setCurrentIndex(tool->target_layer_state_->index());
      
      // set the defaults for the iterations from the state variables
        int iterations_min = 0.0; 
      int iterations_max = 0.0;
      tool->iterations_state_->get_range( iterations_min, iterations_max );
        this->private_->iterationsAdjuster->setRanges( iterations_min, iterations_max );
        this->private_->iterationsAdjuster->setCurrentValue( tool->iterations_state_->get() );
        
        // set the defaults for the step from the state variables
        int step_min = 0.0; 
      int step_max = 0.0;
      tool->steps_state_->get_range( step_min, step_max );
        this->private_->stepAdjuster->setRanges( step_min, step_max );
        this->private_->stepAdjuster->setCurrentValue( tool->steps_state_->get() );
        
        // set the defaults for the conductance from the state variables
        double conductance_min = 0.0; 
      double conductance_max = 0.0;
      tool->conductance_state_->get_range( conductance_min, conductance_max );
        this->private_->stepAdjuster->setRanges( step_min, step_max );
        this->private_->stepAdjuster->setCurrentValue( tool->conductance_state_->get() );

      

  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( this->private_->ui_.targetComboBox, tool->target_layer_state_ );
  QtBridge::Connect( this->private_->iterationsAdjuster, tool->iterations_state_ );
  QtBridge::Connect( this->private_->stepAdjuster, tool->steps_state_ );
  QtBridge::Connect( this->private_->conductanceAdjuster, tool->conductance_state_ );
  QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );

  //Send a message to the log that we have finised with building the Anisotropic Diffusion Filter Interface
  SCI_LOG_DEBUG("Finished building an Anisotropic Diffusion Filter Interface");

  return ( true );
} // end build_widget

} // end namespace Seg3D

