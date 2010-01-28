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


#include <Interface/ToolInterface/PaintToolInterface.h>
#include <Interface/QtInterface/QtBridge.h>
#include <Application/Tools/PaintTool.h>
#include "ui_PaintToolInterface.h"

namespace Seg3D {

SCI_REGISTER_TOOLINTERFACE(PaintToolInterface)


class PaintToolInterfacePrivate {
  public:
    Ui::PaintToolInterface ui_;
};


PaintToolInterface::PaintToolInterface() :
  private_(new PaintToolInterfacePrivate)
{  
  
}

PaintToolInterface::~PaintToolInterface()
{
}

bool
PaintToolInterface::build_widget(QFrame* frame)
{
  
  private_->ui_.setupUi(frame);
  
  paintBrushAdjuster = new SliderSpinComboInt();
  private_->ui_.verticalLayout->addWidget(paintBrushAdjuster);
  
  upperThresholdAdjuster = new SliderSpinComboDouble();
  private_->ui_.upperHLayout_bottom->addWidget(upperThresholdAdjuster);
  
  lowerThresholdAdjuster = new SliderSpinComboDouble();
  private_->ui_.lowerHLayout_bottom->addWidget(lowerThresholdAdjuster);


  ToolHandle base_tool_ = tool();
  PaintTool* tool = dynamic_cast<PaintTool*>(base_tool_.get());
  QtBridge::connect(private_->ui_.activeComboBox, tool->target_layer_);
  QtBridge::connect(private_->ui_.maskComboBox, tool->mask_layer_);
  QtBridge::connect(paintBrushAdjuster, tool->brush_radius_);
  QtBridge::connect(upperThresholdAdjuster, tool->upper_threshold_);
  QtBridge::connect(lowerThresholdAdjuster, tool->lower_threshold_);
  QtBridge::connect(private_->ui_.eraseCheckBox, tool->erase_);

  makeConnections();
  SCI_LOG_DEBUG("Finished building a Paint Brush Interface");
  
  return (true);
}
  
void PaintToolInterface::makeConnections()
{
  connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(senseActiveChanged(QString)));
  connect(private_->ui_.maskComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(senseMaskChanged(QString)));

}


//  --- Private slots for custom signals ---  //
void PaintToolInterface::senseActiveChanged(QString active)
{
  //private_->ui_.maskComboBox->setCurrentIndex(0);
  Q_EMIT activeChanged( active );
}

void PaintToolInterface::senseMaskChanged(QString mask)
{
  Q_EMIT maskChanged(mask);
}

void PaintToolInterface::senseEraseModeChanged(bool mode)
{
  Q_EMIT eraseModeChanged(mode);
}

void PaintToolInterface::sensePaintBrushSizeChanged(double size)
{
  Q_EMIT paintBrushSizeChanged(size);
}

void PaintToolInterface::senseUpperThresholdChanged(double upper)
{
  Q_EMIT  upperThresholdChanged(upper);
}

void PaintToolInterface::senselowerThresholdChanged(double lower)
{
  Q_EMIT lowerThresholdChanged(lower);
}

//  --- Public slots for setting widget values ---  //
void PaintToolInterface::setActive(int active)
{
  private_->ui_.activeComboBox->setCurrentIndex(active);
}

void PaintToolInterface::addToActive(QStringList &items)
{
  private_->ui_.activeComboBox->addItems(items);
}

void PaintToolInterface::setMask(int mask)
{
  private_->ui_.maskComboBox->setCurrentIndex(mask);
}

void PaintToolInterface::addToMask(QStringList &items)
{
  private_->ui_.maskComboBox->addItems(items);
}

void PaintToolInterface::setPaintBrushSize(int size)
{
  paintBrushAdjuster->setCurrentValue(size);
}

void PaintToolInterface::setLowerThreshold(double lower, double upper)
{
  lowerThresholdAdjuster->setRanges(lower, upper);
}

void PaintToolInterface::setLowerThresholdStep(double step)
{
  lowerThresholdAdjuster->setStep(step);
}

void PaintToolInterface::setUpperThreshold(double lower, double upper)
{
  upperThresholdAdjuster->setRanges(lower, upper);
}

void PaintToolInterface::setUpperThresholdStep(double step)
{
  upperThresholdAdjuster->setStep(step);
}  
  
  

} // namespace Seg3D

