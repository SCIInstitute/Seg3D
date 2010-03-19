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
#include <Interface/ToolInterface/ThresholdToolInterface.h>
#include <Interface/ToolInterface/CustomWidgets/SliderSpinComboDouble.h>
#include "ui_ThresholdToolInterface.h"

//Application Includes
#include <Application/Tools/ThresholdTool.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(ThresholdToolInterface)

class ThresholdToolInterfacePrivate
{
public:
  Ui::ThresholdToolInterface ui_;
  
  SliderSpinComboDouble *upperThresholdAdjuster;
  SliderSpinComboDouble *lowerThresholdAdjuster;
};

// constructor
ThresholdToolInterface::ThresholdToolInterface() :
  private_( new ThresholdToolInterfacePrivate )
{
}

// destructor
ThresholdToolInterface::~ThresholdToolInterface()
{
}

// build the interface and connect it to the state manager
bool ThresholdToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  this->private_->upperThresholdAdjuster = new SliderSpinComboDouble();
  this->private_->ui_.verticalLayout_2->addWidget( this->private_->upperThresholdAdjuster );

  this->private_->lowerThresholdAdjuster = new SliderSpinComboDouble();
  this->private_->ui_.verticalLayout_3->addWidget( this->private_->lowerThresholdAdjuster );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ThresholdTool* tool = dynamic_cast< ThresholdTool* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
  
      //set default falues for the target option list 
      std::vector< std::string > temp_option_list = tool->target_layer_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.targetComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.targetComboBox->setCurrentIndex(tool->target_layer_state_->index());
      
      // set the defaults for the upper threshold
        double upper_threshold_min = 0.0; 
      double upper_threshold_max = 0.0;
      tool->upper_threshold_state_->get_range( upper_threshold_min, upper_threshold_max );
        this->private_->upperThresholdAdjuster->setRanges( upper_threshold_min, upper_threshold_max );
        this->private_->upperThresholdAdjuster->setCurrentValue( tool->upper_threshold_state_->get() );
        
        // set the defaults for the lower threshold
        double lower_threshold_min = 0.0; 
      double lower_threshold_max = 0.0;
      tool->lower_threshold_state_->get_range( lower_threshold_min, lower_threshold_max );
        this->private_->lowerThresholdAdjuster->setRanges( lower_threshold_min, lower_threshold_max );
        this->private_->lowerThresholdAdjuster->setCurrentValue( tool->lower_threshold_state_->get() );


  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( private_->ui_.targetComboBox, tool->target_layer_state_ );
  QtBridge::Connect( this->private_->upperThresholdAdjuster, tool->upper_threshold_state_ );
  QtBridge::Connect( this->private_->lowerThresholdAdjuster, tool->lower_threshold_state_ );

  //Send a message to the log that we have finised with building the Threshold Tool Interface
  SCI_LOG_DEBUG("Finished building a Threshold Tool Interface");

  return ( true );
} // end build_widget

} // end namespace Seg3D
