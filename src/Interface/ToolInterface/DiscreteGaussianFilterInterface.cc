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
#include <Interface/ToolInterface/DiscreteGaussianFilterInterface.h>
#include "ui_DiscreteGaussianFilterInterface.h"

//Application Includes
#include <Application/Tools/DiscreteGaussianFilter.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(DiscreteGaussianFilterInterface)

class DiscreteGaussianFilterInterfacePrivate
{
public:
  Ui::DiscreteGaussianFilterInterface ui_;
    
    SliderDoubleCombo *variance_;
  SliderDoubleCombo *kernel_width_;
};

// constructor
DiscreteGaussianFilterInterface::DiscreteGaussianFilterInterface() :
  private_( new DiscreteGaussianFilterInterfacePrivate )
{
}

// destructor
DiscreteGaussianFilterInterface::~DiscreteGaussianFilterInterface()
{
}

// build the interface and connect it to the state manager
bool DiscreteGaussianFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  private_->ui_.setupUi( frame );

  //Add the SliderSpinCombos
  private_->variance_ = new SliderDoubleCombo();
  private_->ui_.varianceHLayout_bottom->addWidget( private_->variance_ );

  private_->kernel_width_ = new SliderDoubleCombo();
  private_->ui_.kernelHLayout_bottom->addWidget( private_->kernel_width_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  DiscreteGaussianFilter* tool = dynamic_cast< DiscreteGaussianFilter* > ( base_tool_.get() );

    //Step 3 - set the values for the tool ui from the state engine
  
      //set default falues for the target option list 
      std::vector< std::string > temp_option_list = tool->target_layer_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.targetComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.targetComboBox->setCurrentIndex(tool->target_layer_state_->index());
        
        // set the defaults for the variance
      double variance_min = 0.0; 
      double variance_max = 0.0;
      double variance_step = 0.0;
      tool->variance_state_->get_step( variance_step );
      tool->variance_state_->get_range( variance_min, variance_max );
      private_->variance_->setStep( variance_step );
        private_->variance_->setRange( variance_min, variance_max );
        private_->variance_->setCurrentValue( tool->variance_state_->get() );
        
        // set the defaults for the kernel width
        double kernel_width_min = 0.0; 
      double kernel_width_max = 0.0;
      double kernel_width_step = 0.0;
      tool->maximum_kernel_width_state_->get_step( kernel_width_step );
      tool->maximum_kernel_width_state_->get_range( kernel_width_min, kernel_width_max );
      private_->kernel_width_->setStep( kernel_width_step );
        private_->kernel_width_->setRange( kernel_width_min, kernel_width_max );
        private_->kernel_width_->setCurrentValue( tool->maximum_kernel_width_state_->get() );
        
        //set the default replace checkbox value
        this->private_->ui_.replaceCheckBox->setChecked(tool->replace_state_);
        
    
  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( private_->ui_.targetComboBox, tool->target_layer_state_ );
  QtBridge::Connect( private_->variance_, tool->variance_state_ );
  QtBridge::Connect( private_->kernel_width_, tool->maximum_kernel_width_state_ );
  QtBridge::Connect( private_->ui_.replaceCheckBox, tool->replace_state_ );

  //Send a message to the log that we have finised with building the Discrete Gaussian Filter Interface
  SCI_LOG_DEBUG("Finished building a Discrete Gaussian Filter Interface");
  return ( true );
} // end build_widget

} // end namespace Seg3D
