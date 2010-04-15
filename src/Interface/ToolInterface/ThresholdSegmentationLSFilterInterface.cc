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
#include <Interface/ToolInterface/CustomWidgets/MaskComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/ThresholdSegmentationLSFilterInterface.h>
#include "ui_ThresholdSegmentationLSFilterInterface.h"

//Application Includes
#include <Application/Tools/ThresholdSegmentationLSFilter.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(ThresholdSegmentationLSFilterInterface)

class ThresholdSegmentationLSFilterInterfacePrivate
{
public:
  Ui::ThresholdSegmentationLSFilterInterface ui_;
  
  SliderIntCombo *iterations_;
  SliderDoubleCombo *upper_threshold_;
  SliderDoubleCombo *lower_threshold_;
  SliderDoubleCombo *curvature_;
  SliderDoubleCombo *edge_;
  SliderDoubleCombo *propagation_;
  TargetComboBox *target_;
  MaskComboBox *mask_;
};

// constructor
ThresholdSegmentationLSFilterInterface::ThresholdSegmentationLSFilterInterface() :
  private_( new ThresholdSegmentationLSFilterInterfacePrivate )
{
}

// destructor
ThresholdSegmentationLSFilterInterface::~ThresholdSegmentationLSFilterInterface()
{
}

// build the interface and connect it to the state manager
bool ThresholdSegmentationLSFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

    // add sliderspinnercombo's
    this->private_->iterations_ = new SliderIntCombo();
    this->private_->ui_.iterationsHLayout_bottom->addWidget( this->private_->iterations_ );

    this->private_->upper_threshold_ = new SliderDoubleCombo();
    this->private_->ui_.upperHLayout_bottom->addWidget( this->private_->upper_threshold_ );

    this->private_->lower_threshold_ = new SliderDoubleCombo();
    this->private_->ui_.lowerHLayout_bottom->addWidget( this->private_->lower_threshold_ );

    this->private_->curvature_ = new SliderDoubleCombo();
    this->private_->ui_.curvatureHLayout_bottom->addWidget( this->private_->curvature_ );

    this->private_->edge_ = new SliderDoubleCombo();
    this->private_->ui_.edgeHLayout_bottom->addWidget( this->private_->edge_ );

    this->private_->propagation_ = new SliderDoubleCombo();
    this->private_->ui_.propagationHLayout_bottom->addWidget( this->private_->propagation_ );
    
    this->private_->target_ = new TargetComboBox( this );
    this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );
    
    this->private_->mask_ = new MaskComboBox( this );
    this->private_->ui_.maskHLayout->addWidget( this->private_->mask_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ThresholdSegmentationLSFilter* tool =
      dynamic_cast< ThresholdSegmentationLSFilter* > ( base_tool_.get() );
      
  //Step 3 - set the values for the tool ui from the state engine
  
        // set the defaults for the iterations
      int iterations_min = 0; 
      int iterations_max = 0;
      int iterations_step = 0;
      tool->iterations_state_->get_step( iterations_step );
      tool->iterations_state_->get_range( iterations_min, iterations_max );
      this->private_->iterations_->setStep( iterations_step );
        this->private_->iterations_->setRange( iterations_min, iterations_max );
        this->private_->iterations_->setCurrentValue( tool->iterations_state_->get() );

        
        // set the defaults for the upper threshold
        double upper_threshold_min = 0.0; 
      double upper_threshold_max = 0.0;
      double upper_threshold_step = 0.0;
      tool->upper_threshold_state_->get_step( upper_threshold_step );
      tool->upper_threshold_state_->get_range( upper_threshold_min, upper_threshold_max );
      this->private_->upper_threshold_->setStep( upper_threshold_step );
        this->private_->upper_threshold_->setRange( upper_threshold_min, upper_threshold_max );
        this->private_->upper_threshold_->setCurrentValue( tool->upper_threshold_state_->get() );
        
        // set the defaults for the lower threshold
        double lower_threshold_min = 0.0; 
      double lower_threshold_max = 0.0;
      double lower_threshold_step = 0.0;
      tool->lower_threshold_state_->get_step( lower_threshold_step );
      tool->lower_threshold_state_->get_range( lower_threshold_min, lower_threshold_max );
      this->private_->lower_threshold_->setStep( lower_threshold_step );
        this->private_->lower_threshold_->setRange( lower_threshold_min, lower_threshold_max );
        this->private_->lower_threshold_->setCurrentValue( tool->lower_threshold_state_->get() );
        
        // set the defaults for the curvature
        double curvature_min = 0.0; 
      double curvature_max = 0.0;
      double curvature_step = 0.0;
      tool->curvature_state_->get_step( curvature_step );
      tool->curvature_state_->get_range( curvature_min, curvature_max );
      this->private_->curvature_->setStep( curvature_step );
        this->private_->curvature_->setRange( curvature_min, curvature_max );
        this->private_->curvature_->setCurrentValue( tool->curvature_state_->get() );
        
        // set the defaults for edge
        double edge_min = 0.0; 
      double edge_max = 0.0;
      double edge_step = 0.0;
      tool->edge_state_->get_step( edge_step );
      tool->edge_state_->get_range( edge_min, edge_max );
      this->private_->edge_->setStep( edge_step );
        this->private_->edge_->setRange( edge_min, edge_max );
        this->private_->edge_->setCurrentValue( tool->edge_state_->get() ); 
        
        // set the defaults for the propagation
        double propagation_min = 0.0; 
      double propagation_max = 0.0;
      double propagation_step = 0.0;
      tool->propagation_state_->get_step( propagation_step );
      tool->propagation_state_->get_range( propagation_min, propagation_max );
      this->private_->propagation_->setStep( propagation_step );
        this->private_->propagation_->setRange( propagation_min, propagation_max );
        this->private_->propagation_->setCurrentValue( tool->propagation_state_->get() );
        
        // set the default for the replace state
        this->private_->ui_.replaceCheckBox->setChecked( tool->replace_state_->get() );



  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  QtBridge::Connect( this->private_->mask_, tool->mask_layer_state_ );
  QtBridge::Connect( this->private_->iterations_, tool->iterations_state_ );
  QtBridge::Connect( this->private_->upper_threshold_, tool->upper_threshold_state_ );
  QtBridge::Connect( this->private_->lower_threshold_, tool->lower_threshold_state_ );
  QtBridge::Connect( this->private_->curvature_, tool->curvature_state_ );
  QtBridge::Connect( this->private_->edge_, tool->propagation_state_ );
  QtBridge::Connect( this->private_->propagation_, tool->edge_state_ );
  QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );

  //Send a message to the log that we have finised with building the Segmentation Level Set Filter Interface
  SCI_LOG_DEBUG("Finished building a Segmentation Level Set Filter Interface");
  return ( true );

} // end build_widget

} // end namespace Seg3D
