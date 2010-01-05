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


#include <Interface/ToolInterface/CannyEdgeDetectionFilterInterface.h>
#include "ui_CannyEdgeDetectionFilterInterface.h"

namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(CannyEdgeDetectionFilterInterface)
  
  
  class CannyEdgeDetectionFilterInterfacePrivate {
  public:
    Ui::CannyEdgeDetectionFilterInterface ui_;
  };
  
  
  CannyEdgeDetectionFilterInterface::CannyEdgeDetectionFilterInterface() :
  private_(new CannyEdgeDetectionFilterInterfacePrivate)
  {  
    
  }
  
  CannyEdgeDetectionFilterInterface::~CannyEdgeDetectionFilterInterface()
  {
  }
  
  bool
  CannyEdgeDetectionFilterInterface::build_widget(QFrame* frame)
  {
    
    private_->ui_.setupUi(frame);
    
    varianceAdjuster = new SliderSpinCombo();
    private_->ui_.varianceHLayout_bottom->addWidget(varianceAdjuster);
    
    errorAdjuster = new SliderSpinCombo();
    private_->ui_.errorHLayout_bottom->addWidget(errorAdjuster);
    
    thresholdAdjuster = new SliderSpinCombo();
    private_->ui_.thresholdHLayout_bottom->addWidget(thresholdAdjuster);
    
    makeConnections();
    
    SCI_LOG_DEBUG("Finished building a Canny Edge Detection Filter Interface");
    return (true);
  }
  
  void 
  CannyEdgeDetectionFilterInterface::makeConnections()
  {
    connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
    connect(errorAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseErrorChanged(double)));
    connect(varianceAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseVarianceChanged(double)));
    connect(thresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseThresholdChanged(double)));
    connect(private_->ui_.invertButton, SIGNAL(clicked()), this, SLOT(senseFilterRun()));
    
    
  }
  
  //  --- Private slots for custom signals ---  //
  void CannyEdgeDetectionFilterInterface::senseActiveChanged(int active)
  {
    Q_EMIT activeChanged( active );
  }
  
  void CannyEdgeDetectionFilterInterface::senseErrorChanged(int errors)
  {
    Q_EMIT errorChanged(errors);
  }
  
  void CannyEdgeDetectionFilterInterface::senseVarianceChanged(double variance)
  {
    Q_EMIT varianceChanged(variance);
  }
  
  void CannyEdgeDetectionFilterInterface::senseThresholdChanged(int threshold)
  {
    Q_EMIT thresholdChanged(threshold);
  }
  
  void CannyEdgeDetectionFilterInterface::senseFilterRun()
  {
    if(private_->ui_.replaceCheckBox->isChecked())
    {
      Q_EMIT filterRun(true);
    }
    else
    {
      Q_EMIT filterRun(false);
    }
  }
  
  
  //  --- Public slots for setting widget values ---  //
  void CannyEdgeDetectionFilterInterface::setActive(int active)
  {
    private_->ui_.activeComboBox->setCurrentIndex(active);
  }
  
  void CannyEdgeDetectionFilterInterface::addToActive(QStringList &items)
  {
    private_->ui_.activeComboBox->addItems(items);
  }
  
  void CannyEdgeDetectionFilterInterface::setVariance(int variance)
  {
    varianceAdjuster->setCurrentValue(variance);
  }
  
  void CannyEdgeDetectionFilterInterface::setVarianceRange(int lower, int upper)
  {
    varianceAdjuster->setRanges(lower, upper);
  }
  
  void CannyEdgeDetectionFilterInterface::setError(double errors)
  {
    errorAdjuster->setCurrentValue(errors);
  }
  
  void CannyEdgeDetectionFilterInterface::setErrorRange(double lower, double upper)
  {
    errorAdjuster->setRanges(lower, upper);
  }
  
  void CannyEdgeDetectionFilterInterface::setThreshold(int threshold)
  {
    thresholdAdjuster->setCurrentValue(threshold);
  }
  
  void CannyEdgeDetectionFilterInterface::setThresholdRange(int lower, int upper)
  {
    thresholdAdjuster->setRanges(lower, upper);
  }
  
  
  
} // namespace Seg3D

