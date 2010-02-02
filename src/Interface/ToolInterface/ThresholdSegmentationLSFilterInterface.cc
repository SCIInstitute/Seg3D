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
#include <Interface/ToolInterface/ThresholdSegmentationLSFilterInterface.h>
#include "ui_ThresholdSegmentationLSFilterInterface.h"

//Application Includes
#include <Application/Tools/ThresholdSegmentationLSFilter.h>


namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(ThresholdSegmentationLSFilterInterface)
  
class ThresholdSegmentationLSFilterInterfacePrivate {
public:
  Ui::ThresholdSegmentationLSFilterInterface ui_;
};
  
  // constructor
  ThresholdSegmentationLSFilterInterface::ThresholdSegmentationLSFilterInterface() :
  private_(new ThresholdSegmentationLSFilterInterfacePrivate)
  { }
  
  // destructor
  ThresholdSegmentationLSFilterInterface::~ThresholdSegmentationLSFilterInterface()
  { }
  
  // build the interface and connect it to the state manager
  bool
  ThresholdSegmentationLSFilterInterface::build_widget(QFrame* frame)
  {
    //Step 1 - build the Qt GUI Widget
    private_->ui_.setupUi(frame);
        
      // add sliderspinnercombo's
      iterationsAdjuster = new SliderSpinComboInt();
      private_->ui_.iterationsHLayout_bottom->addWidget(iterationsAdjuster);
      
      upperThresholdAdjuster = new SliderSpinComboInt();
      private_->ui_.upperHLayout_bottom->addWidget(upperThresholdAdjuster);
      
      lowerThresholdAdjuster = new SliderSpinComboInt();
      private_->ui_.lowerHLayout_bottom->addWidget(lowerThresholdAdjuster);
      
      curvatureAdjuster = new SliderSpinComboInt();
      private_->ui_.curvatureHLayout_bottom->addWidget(curvatureAdjuster);
      
      edgeAdjuster = new SliderSpinComboInt();
      private_->ui_.edgeHLayout_bottom->addWidget(edgeAdjuster);
      
      propagationAdjuster = new SliderSpinComboInt();
      private_->ui_.propagationHLayout_bottom->addWidget(propagationAdjuster);
    
    //Step 2 - get a pointer to the tool
    ToolHandle base_tool_ = tool();
    ThresholdSegmentationLSFilter* tool = dynamic_cast<ThresholdSegmentationLSFilter*>(base_tool_.get());
    
    //Step 3 - connect the gui to the tool through the QtBridge
    QtBridge::connect(private_->ui_.targetComboBox, tool->target_layer_);
    QtBridge::connect(private_->ui_.maskComboBox, tool->mask_layer_);
    QtBridge::connect(iterationsAdjuster, tool->iterations_);
    QtBridge::connect(upperThresholdAdjuster, tool->upper_threshold_);
    QtBridge::connect(lowerThresholdAdjuster, tool->lower_threshold_);
    QtBridge::connect(curvatureAdjuster, tool->curvature_);
    QtBridge::connect(edgeAdjuster, tool->propagation_);
    QtBridge::connect(propagationAdjuster, tool->edge_);
    QtBridge::connect(private_->ui_.replaceCheckBox,tool->replace_);
    
    //Send a message to the log that we have finised with building the Segmentation Level Set Filter Interface   
    SCI_LOG_DEBUG("Finished building a Segmentation Level Set Filter Interface");
    return (true);
    
  } // end build_widget
  
} // end namespace Seg3D