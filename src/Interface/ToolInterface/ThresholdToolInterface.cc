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

#include "ThresholdToolInterface.h"
#include "ui_ThresholdToolInterface.h"

namespace Seg3D {
  
SCI_REGISTER_TOOLINTERFACE(ThresholdToolInterface)

class ThresholdToolInterfacePrivate {
  public:
    Ui::ThresholdToolInterface ui_;
};

ThresholdToolInterface::ThresholdToolInterface() :
  private_(new ThresholdToolInterfacePrivate)
{
  SCI_LOG_DEBUG("Building Threshold Tool Interface");
}

ThresholdToolInterface::~ThresholdToolInterface()
{
  
}

bool
ThresholdToolInterface::build_widget(QFrame* frame)
{
  private_->ui_.setupUi(frame);
  
  upperThresholdAdjuster = new SliderSpinCombo();
  private_->ui_.upperHLayout_bottom->addWidget(upperThresholdAdjuster);
  
  lowerThresholdAdjuster = new SliderSpinCombo();
  private_->ui_.lowerHLayout_bottom->addWidget(lowerThresholdAdjuster);
  
  SCI_LOG_DEBUG("Finished Building a Threshold Tool"); 
  return (true);
}

//  --- Function for making signal slots connections ---  //
void ThresholdToolInterface::makeConnections()
{
  connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
  connect(upperThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseUpperThresholdChanged(double)));
  connect(lowerThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senselowerThresholdChanged(double)));
  connect(private_->ui_.createThresholdLayerButton, SIGNAL(clicked()), this, SLOT(senseCreateThresholdLayer()));
  connect(private_->ui_.clearSeedsButton, SIGNAL(clicked()), this, SLOT(senseClearSeeds()));
}
  
  
//  --- Private slots for custom signals ---  //
void ThresholdToolInterface::senseActiveChanged(int active)
{
  Q_EMIT activeChanged(active);
}

void ThresholdToolInterface::senseCreateThresholdLayer()
{
  Q_EMIT createThresholdLayer();
}

void ThresholdToolInterface::senseClearSeeds()
{
  Q_EMIT clearSeeds();
}

void ThresholdToolInterface::senseUpperThresholdChanged(double upper)
{
  Q_EMIT upperThresholdChanged(upper);
}

void ThresholdToolInterface::senselowerThresholdChanged(double lower)
{
  Q_EMIT lowerThresholdChanged(lower);
}
  

//  --- Public slots for setting widget values ---  //
void ThresholdToolInterface::setActive(int active)
{
  private_->ui_.activeComboBox->setCurrentIndex(active);
}

void ThresholdToolInterface::addToActive(QStringList &items)
{
  private_->ui_.activeComboBox->addItems(items);
}

void ThresholdToolInterface::setLowerThreshold(double lower, double upper)
{
  lowerThresholdAdjuster->setRanges(lower, upper);
}

void ThresholdToolInterface::setLowerThresholdStep(double step)
{
  lowerThresholdAdjuster->setStep(step);
}

void ThresholdToolInterface::setUpperThreshold(double lower, double upper)
{
  upperThresholdAdjuster->setRanges(lower, upper);
}

void ThresholdToolInterface::setUpperThresholdStep(double step)
{
  upperThresholdAdjuster->setStep(step);
}

void ThresholdToolInterface::setHistogram()
{
  //TODO - implement histogram display
}
  
  
  
  
  
}  // end namespace Seg3D
