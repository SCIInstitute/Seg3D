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
#include <Interface/ToolInterface/ConfidenceConnectedFilterInterface.h>
#include "ui_ConfidenceConnectedFilterInterface.h"

//Application Includes
#include <Application/Tools/ConfidenceConnectedFilter.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(ConfidenceConnectedFilterInterface)

class ConfidenceConnectedFilterInterfacePrivate
{
public:
  Ui::ConfidenceConnectedFilterInterface ui_;
    SliderIntCombo *iterations_;
  SliderIntCombo *multiplier_;
};

// constructor
ConfidenceConnectedFilterInterface::ConfidenceConnectedFilterInterface() :
  private_( new ConfidenceConnectedFilterInterfacePrivate )
{
}

// destructor
ConfidenceConnectedFilterInterface::~ConfidenceConnectedFilterInterface()
{
}

// build the interface and connect it to the state manager
bool ConfidenceConnectedFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  private_->ui_.setupUi( frame );

  //Add the SliderSpinCombos
  private_->iterations_ = new SliderIntCombo();
  private_->ui_.iterationsHLayout_bottom->addWidget( private_->iterations_ );

  private_->multiplier_ = new SliderIntCombo();
  private_->ui_.multiplierHLayout_bottom->addWidget( private_->multiplier_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ConfidenceConnectedFilter* tool =
      dynamic_cast< ConfidenceConnectedFilter* > ( base_tool_.get() );
      
  //Step 3 - set the values for the tool ui from the state engine
  
      //set default falues for the target option list 
      std::vector< std::string > temp_option_list = tool->target_layer_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.targetComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.targetComboBox->setCurrentIndex(tool->target_layer_state_->index());
        
        // set the defaults for the iterations
      int iterations_min = 0; 
      int iterations_max = 0;
      int iterations_step = 0;
      tool->iterations_state_->get_step( iterations_step );
      tool->iterations_state_->get_range( iterations_min, iterations_max );
      private_->iterations_->setStep( iterations_step );
        private_->iterations_->setRange( iterations_min, iterations_max );
        private_->iterations_->setCurrentValue( tool->iterations_state_->get() );
        
        // set the defaults for the multiplier
      int multiplier_min = 0; 
      int multiplier_max = 0;
      int multiplier_step = 0;
      tool->threshold_multiplier_state_->get_step( multiplier_step );
      tool->threshold_multiplier_state_->get_range( multiplier_min, multiplier_max );
      private_->multiplier_->setStep( multiplier_step );
        private_->multiplier_->setRange( multiplier_min, multiplier_max );
        private_->multiplier_->setCurrentValue( tool->threshold_multiplier_state_->get() );
   
   

  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( private_->ui_.targetComboBox, tool->target_layer_state_ );
  QtBridge::Connect( private_->iterations_, tool->iterations_state_ );
  QtBridge::Connect( private_->multiplier_, tool->threshold_multiplier_state_ );

  //Send a message to the log that we have finised with building the Confidence Connected Filter Interface
  SCI_LOG_DEBUG("Finished building a Confidence Connected Filter Interface");
  return ( true );

} // end build_widget


} // end namespace Seg3D
