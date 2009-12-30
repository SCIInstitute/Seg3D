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


#include <Interface/ToolInterface/AnisotropicDiffusionFilterInterface.h>
#include "ui_AnisotropicDiffusionFilterInterface.h"

namespace Seg3D {
  
SCI_REGISTER_TOOLINTERFACE(AnisotropicDiffusionFilterInterface)
  
  
class AnisotropicDiffusionFilterInterfacePrivate {
public:
  Ui::AnisotropicDiffusionFilterInterface ui_;
};


AnisotropicDiffusionFilterInterface::AnisotropicDiffusionFilterInterface() :
private_(new AnisotropicDiffusionFilterInterfacePrivate)
{  
  SCI_LOG_DEBUG("Building Anisotropic Diffusion Filter Interface");
}

AnisotropicDiffusionFilterInterface::~AnisotropicDiffusionFilterInterface()
{
}
  
bool
AnisotropicDiffusionFilterInterface::build_widget(QFrame* frame)
{
  
  private_->ui_.setupUi(frame);
  
  iterationsAdjuster = new SliderSpinCombo();
  private_->ui_.iterationsHLayout_bottom->addWidget(iterationsAdjuster);
  
  stepAdjuster = new SliderSpinCombo();
  private_->ui_.integrationHLayout_bottom->addWidget(stepAdjuster);
  
  conductanceAdjuster = new SliderSpinCombo();
  private_->ui_.conductanceHLayout_bottom->addWidget(conductanceAdjuster);
  
  makeConnections();
  SCI_LOG_DEBUG("Finished Building a PolyLine Tool");
  
  return (true);
}

void 
AnisotropicDiffusionFilterInterface::makeConnections()
{
  connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
  connect(iterationsAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseIterationsChanged(double)));
  connect(stepAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseStepChanged(double)));
  connect(conductanceAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseConductanceChanged(double)));
  connect(private_->ui_.invertButton, SIGNAL(clicked()), this, SLOT(senseFilterRun()));
  
                                                                                                
//  connect(private_->ui_.maskComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseMaskChanged(int)));
//  connect(private_->ui_.eraseCheckBox, SIGNAL(toggled(bool)), this, SLOT(senseEraseModeChanged(bool)));
//  
//  
//  connect(paintBrushAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(sensePaintBrushSizeChanged(double)));
//  connect(upperThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseUpperThresholdChanged(double)));
//  connect(lowerThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senselowerThresholdChanged(double)));

}

  //  --- Private slots for custom signals ---  //
  void AnisotropicDiffusionFilterInterface::senseActiveChanged(int active)
  {
    Q_EMIT activeChanged( active );
  }
  
  void AnisotropicDiffusionFilterInterface::senseIterationsChanged(int iterations)
  {
    Q_EMIT iterationsChanged(iterations);
  }
  
  void AnisotropicDiffusionFilterInterface::senseStepChanged(double step)
  {
    Q_EMIT stepChanged(step);
  }
  
  void AnisotropicDiffusionFilterInterface::senseConductanceChanged(int conductance)
  {
    Q_EMIT conductanceChanged(conductance);
  }
  
  void AnisotropicDiffusionFilterInterface::senseFilterRun()
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
  void AnisotropicDiffusionFilterInterface::setActive(int active)
  {
    private_->ui_.activeComboBox->setCurrentIndex(active);
  }
  
  void AnisotropicDiffusionFilterInterface::addToActive(QStringList &items)
  {
    private_->ui_.activeComboBox->addItems(items);
  }
  
  void AnisotropicDiffusionFilterInterface::setIterations(int iterations)
  {
    iterationsAdjuster->setCurrentValue(iterations);
  }
  
  void AnisotropicDiffusionFilterInterface::setIterationRange(int lower, int upper)
  {
    iterationsAdjuster->setRanges(lower, upper);
  }
  
  void AnisotropicDiffusionFilterInterface::setStep(double iterations)
  {
    stepAdjuster->setCurrentValue(iterations);
  }
  
  void AnisotropicDiffusionFilterInterface::setStepRange(double lower, double upper)
  {
    stepAdjuster->setRanges(lower, upper);
  }
  
  void AnisotropicDiffusionFilterInterface::setConductance(int iterations)
  {
    conductanceAdjuster->setCurrentValue(iterations);
  }
  
  void AnisotropicDiffusionFilterInterface::setConductanceRange(int lower, int upper)
  {
    conductanceAdjuster->setRanges(lower, upper);
  }
  
  

} // namespace Seg3D

