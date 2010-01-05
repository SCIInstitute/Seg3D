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



#include "ThresholdSegmentationLSFilterInterface.h"
#include "ui_ThresholdSegmentationLSFilterInterface.h"


namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(ThresholdSegmentationLSFilterInterface)
  
  class ThresholdSegmentationLSFilterInterfacePrivate {
  public:
    Ui::ThresholdSegmentationLSFilterInterface ui_;
  };
  
  ThresholdSegmentationLSFilterInterface::ThresholdSegmentationLSFilterInterface() :
  private_(new ThresholdSegmentationLSFilterInterfacePrivate)
  {
    
  }
  
  ThresholdSegmentationLSFilterInterface::~ThresholdSegmentationLSFilterInterface()
  {
  }
  
  
  bool
  ThresholdSegmentationLSFilterInterface::build_widget(QFrame* frame)
  {
    private_->ui_.setupUi(frame);
        
    iterationsAdjuster = new SliderSpinCombo();
    private_->ui_.iterationsHLayout_bottom->addWidget(iterationsAdjuster);
    
    upperThresholdAdjuster = new SliderSpinCombo();
    private_->ui_.upperHLayout_bottom->addWidget(upperThresholdAdjuster);
    
    lowerThresholdAdjuster = new SliderSpinCombo();
    private_->ui_.lowerHLayout_bottom->addWidget(lowerThresholdAdjuster);
    
    curvatureAdjuster = new SliderSpinCombo();
    private_->ui_.curvatureHLayout_bottom->addWidget(curvatureAdjuster);
    
    edgeAdjuster = new SliderSpinCombo();
    private_->ui_.edgeHLayout_bottom->addWidget(edgeAdjuster);
    
    propagationAdjuster = new SliderSpinCombo();
    private_->ui_.propagationHLayout_bottom->addWidget(propagationAdjuster);
    
    SCI_LOG_DEBUG("Finished building a Segmentation Level Set Filter Interface");
    return (true);
    
  }
  
  //  --- Function for making signal slots connections ---  //
  void 
  ThresholdSegmentationLSFilterInterface::makeConnections()
  {
    connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
    connect(iterationsAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseIterationsChanged(double)));
  }
  
  
  //  --- Private slots for custom signals ---  //
  void ThresholdSegmentationLSFilterInterface::senseActiveChanged(int active)
  {
    Q_EMIT activeChanged(active);
  }
  
  void ThresholdSegmentationLSFilterInterface::senseIterationsChanged(int iteration)
  {
    Q_EMIT iterationsChanged(iteration);
  }
  
  void ThresholdSegmentationLSFilterInterface::senseUpperThresholdChanged(int upper)
  {
    Q_EMIT  upperThresholdChanged(upper);
  }
  
  void ThresholdSegmentationLSFilterInterface::senselowerThresholdChanged(int lower)
  {
    Q_EMIT lowerThresholdChanged(lower);
  }
  
  void ThresholdSegmentationLSFilterInterface::senseCurvatureChanged(int curve)
  {
    Q_EMIT curvatureChanged(curve);
  }
  
  void ThresholdSegmentationLSFilterInterface::senseEdgeChanged(int edge)
  {
    Q_EMIT  edgeChanged(edge);
  }
  
  void ThresholdSegmentationLSFilterInterface::sensePropagationChanged(int propagation)
  {
    Q_EMIT propagationChanged(propagation);
  }
  
  
  
  //  --- Public slots for setting widget values ---  //
  void ThresholdSegmentationLSFilterInterface::setActive(int active)
  {
    private_->ui_.activeComboBox->setCurrentIndex(active);
  }
  
  void ThresholdSegmentationLSFilterInterface::addToActive(QStringList &items)
  {
    private_->ui_.activeComboBox->addItems(items);
  }
  
  void ThresholdSegmentationLSFilterInterface::setIterations(int iterations)
  {
    iterationsAdjuster->setCurrentValue(iterations);
  }
  
  void ThresholdSegmentationLSFilterInterface::setLowerThreshold(double lower, double upper)
  {
    lowerThresholdAdjuster->setRanges(lower, upper);
  }
  
  void ThresholdSegmentationLSFilterInterface::setLowerThresholdStep(double step)
  {
    lowerThresholdAdjuster->setStep(step);
  }
  
  void ThresholdSegmentationLSFilterInterface::setUpperThreshold(double lower, double upper)
  {
    upperThresholdAdjuster->setRanges(lower, upper);
  }
  
  void ThresholdSegmentationLSFilterInterface::setUpperThresholdStep(double step)
  {
    upperThresholdAdjuster->setStep(step);
  } 
  
  void ThresholdSegmentationLSFilterInterface::setCurvature(double lower, double upper)
  {
    curvatureAdjuster->setRanges(lower, upper);
  }
  
  void ThresholdSegmentationLSFilterInterface::setCurvatureStep(double step)
  {
    curvatureAdjuster->setStep(step);
  }
  
  void ThresholdSegmentationLSFilterInterface::setEdge(double lower, double upper)
  {
    edgeAdjuster->setRanges(lower, upper);
  }
  
  void ThresholdSegmentationLSFilterInterface::setEdgeStep(double step)
  {
    edgeAdjuster->setStep(step);
  } 
  
  void ThresholdSegmentationLSFilterInterface::setPropagation(double lower, double upper)
  {
    propagationAdjuster->setRanges(lower, upper);
  }
  
  void ThresholdSegmentationLSFilterInterface::setPropagationStep(double step)
  {
    propagationAdjuster->setStep(step);
  } 
  
  
  
} // namespace Seg3D