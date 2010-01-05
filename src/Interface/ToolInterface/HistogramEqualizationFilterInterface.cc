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



#include "HistogramEqualizationFilterInterface.h"
#include "ui_HistogramEqualizationFilterInterface.h"


namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(HistogramEqualizationFilterInterface)
  
  class HistogramEqualizationFilterInterfacePrivate {
  public:
    Ui::HistogramEqualizationFilterInterface ui_;
  };
  
  HistogramEqualizationFilterInterface::HistogramEqualizationFilterInterface() :
  private_(new HistogramEqualizationFilterInterfacePrivate)
  {
    
  }
  
  HistogramEqualizationFilterInterface::~HistogramEqualizationFilterInterface()
  {
  }
  
  
  bool
  HistogramEqualizationFilterInterface::build_widget(QFrame* frame)
  {
    private_->ui_.setupUi(frame);
        
    upperThresholdAdjuster = new SliderSpinCombo();
    private_->ui_.upperHLayout_bottom->addWidget(upperThresholdAdjuster);
    
    lowerThresholdAdjuster = new SliderSpinCombo();
    private_->ui_.lowerHLayout_bottom->addWidget(lowerThresholdAdjuster);
    
    alphaAdjuster = new SliderSpinCombo();
    private_->ui_.alphaHLayout_bottom->addWidget(alphaAdjuster);
    
    SCI_LOG_DEBUG("Finished building a Histogram Equalization Filter Interface");
    return (true);
    
  }
  
  //  --- Function for making signal slots connections ---  //
  void 
  HistogramEqualizationFilterInterface::makeConnections()
  {
    connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
    
  }
  
  
  //  --- Private slots for custom signals ---  //
  void HistogramEqualizationFilterInterface::senseActiveChanged(int active)
  {
    Q_EMIT activeChanged(active);
  }
  
  
  //  --- Public slots for setting widget values ---  //
  void HistogramEqualizationFilterInterface::setActive(int active)
  {
    private_->ui_.activeComboBox->setCurrentIndex(active);
  }
  
  void HistogramEqualizationFilterInterface::addToActive(QStringList &items)
  {
    private_->ui_.activeComboBox->addItems(items);
  }
  
  
} // namespace Seg3D