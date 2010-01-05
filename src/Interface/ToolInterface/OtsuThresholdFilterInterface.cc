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



#include "OtsuThresholdFilterInterface.h"
#include "ui_OtsuThresholdFilterInterface.h"


namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(OtsuThresholdFilterInterface)
  
  class OtsuThresholdFilterInterfacePrivate {
  public:
    Ui::OtsuThresholdFilterInterface ui_;
  };
  
  OtsuThresholdFilterInterface::OtsuThresholdFilterInterface() :
  private_(new OtsuThresholdFilterInterfacePrivate)
  {
    
  }
  
  OtsuThresholdFilterInterface::~OtsuThresholdFilterInterface()
  {
  }
  
  
  bool
  OtsuThresholdFilterInterface::build_widget(QFrame* frame)
  {
    private_->ui_.setupUi(frame);
    SCI_LOG_DEBUG("Finished Building an Median Interface Filter");
    
    orderAdjuster = new SliderSpinCombo();
    private_->ui_.orderHLayout_bottom->addWidget(orderAdjuster);
    
    return (true);
    
  }
  
  //  --- Function for making signal slots connections ---  //
  void 
  OtsuThresholdFilterInterface::makeConnections()
  {
    connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
    connect(orderAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseOrderChanged(double)));
  }
  
  
  //  --- Private slots for custom signals ---  //
  void OtsuThresholdFilterInterface::senseActiveChanged(int active)
  {
    Q_EMIT activeChanged(active);
  }
  
  void OtsuThresholdFilterInterface::senseOrderChanged(double size)
  {
    Q_EMIT orderChanged(size);
  }
  
  //  --- Public slots for setting widget values ---  //
  void OtsuThresholdFilterInterface::setActive(int active)
  {
    private_->ui_.activeComboBox->setCurrentIndex(active);
  }
  
  void OtsuThresholdFilterInterface::addToActive(QStringList &items)
  {
    private_->ui_.activeComboBox->addItems(items);
  }
  
  void OtsuThresholdFilterInterface::setOrder(int orders)
  {
    orderAdjuster->setCurrentValue(orders);
  }
  
  
  
} // namespace Seg3D