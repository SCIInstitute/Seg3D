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
#include <Interface/ToolInterface/CannyEdgeDetectionFilterInterface.h>
#include "ui_CannyEdgeDetectionFilterInterface.h"

//Application Includes
#include <Application/Tools/CannyEdgeDetectionFilter.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(CannyEdgeDetectionFilterInterface)

class CannyEdgeDetectionFilterInterfacePrivate
{
public:
  Ui::CannyEdgeDetectionFilterInterface ui_;
  
  SliderDoubleCombo *variance_;
  SliderDoubleCombo *max_error_;
  SliderDoubleCombo *threshold_;
};

// constructor
CannyEdgeDetectionFilterInterface::CannyEdgeDetectionFilterInterface() :
  private_( new CannyEdgeDetectionFilterInterfacePrivate )
{
}

// destructor
CannyEdgeDetectionFilterInterface::~CannyEdgeDetectionFilterInterface()
{
}

// build the interface and connect it to the state manager
bool CannyEdgeDetectionFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  private_->ui_.setupUi( frame );

  //Add the SliderSpinCombos
  private_->variance_ = new SliderDoubleCombo();
  private_->ui_.varianceHLayout_bottom->addWidget( private_->variance_ );

  private_->max_error_ = new SliderDoubleCombo();
  private_->ui_.errorHLayout_bottom->addWidget( private_->max_error_ );

  private_->threshold_ = new SliderDoubleCombo();
  private_->ui_.thresholdHLayout_bottom->addWidget( private_->threshold_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  CannyEdgeDetectionFilter* tool = dynamic_cast< CannyEdgeDetectionFilter* > ( base_tool_.get() );

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
        
        // set the defaults for the max error
        double max_error_min = 0.0; 
      double max_error_max = 0.0;
      double max_error_step = 0.0;
      tool->max_error_state_->get_step( max_error_step );
      tool->max_error_state_->get_range( max_error_min, max_error_max );
      private_->max_error_->setStep( max_error_step );
        private_->max_error_->setRange( max_error_min, max_error_max );
        private_->max_error_->setCurrentValue( tool->max_error_state_->get() );

        // set the defaults for the threshold
        double threshold_min = 0.0; 
      double threshold_max = 0.0;
      double threshold_step = 0.0;
      tool->threshold_state_->get_step( threshold_step );
      tool->threshold_state_->get_range( threshold_min, threshold_max );
      private_->threshold_->setStep( threshold_step );
        private_->threshold_->setRange( threshold_min, threshold_max );
        private_->threshold_->setCurrentValue( tool->threshold_state_->get() );
        
        //set the default replace checkbox value
        this->private_->ui_.replaceCheckBox->setChecked(tool->replace_state_);


      
      

  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( private_->ui_.targetComboBox, tool->target_layer_state_ );
  QtBridge::Connect( private_->variance_, tool->variance_state_ );
  QtBridge::Connect( private_->max_error_, tool->max_error_state_ );
  QtBridge::Connect( private_->threshold_, tool->threshold_state_ );
  QtBridge::Connect( private_->ui_.replaceCheckBox, tool->replace_state_ );

  //Send a message to the log that we have finised with building the Detection Filter Interface
  SCI_LOG_DEBUG("Finished building a Canny Edge Detection Filter Interface");

  return ( true );
}

} // namespace Seg3D

