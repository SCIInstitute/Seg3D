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


#include <Interface/ToolInterface/PolyLineToolInterface.h>
#include "ui_PolyLineToolInterface.h"

namespace Seg3D {

SCI_REGISTER_TOOLINTERFACE(PolyLineToolInterface)


class PolyLineToolInterfacePrivate {
  public:
    Ui::PolyLineToolInterface ui_;
};


PolyLineToolInterface::PolyLineToolInterface() :
  private_(new PolyLineToolInterfacePrivate)
{  
  SCI_LOG_DEBUG("Building PolyLine Tool Interface");
}

PolyLineToolInterface::~PolyLineToolInterface()
{
}

bool
PolyLineToolInterface::build_widget(QFrame* frame)
{
  
  private_->ui_.setupUi(frame);
  makeConnections();
  
  SCI_LOG_DEBUG("Finished Building a PolyLine Tool");
  return (true);
}
  
//  --- Function for making signal slots connections ---  //
void PolyLineToolInterface::makeConnections()
{
  connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
  connect(private_->ui_.resetPLButton, SIGNAL(clicked()), this, SLOT(senseResetPolyLineTool()));
  connect(private_->ui_.insideFillButton, SIGNAL(clicked()), this, SLOT(senseFillPolyLine()));
  connect(private_->ui_.insideEraseButton, SIGNAL(clicked()), this, SLOT(senseErasePolyLine()));
}


//  --- Private slots for custom signals ---  //
void PolyLineToolInterface::senseActiveChanged(int active)
{
  Q_EMIT activeChanged(active);
}

void PolyLineToolInterface::senseResetPolyLineTool()
{
  Q_EMIT resetPolyLineTool();
}

void PolyLineToolInterface::senseFillPolyLine()
{
  Q_EMIT fillPolyLine();
}

void PolyLineToolInterface::senseErasePolyLine()
{
  Q_EMIT erasePolyLine();
}

//  --- Public slots for setting widget values ---  //
void PolyLineToolInterface::setActive(int active)
{
  private_->ui_.activeComboBox->setCurrentIndex(active);
}

void PolyLineToolInterface::addToActive(QStringList &items)
{
  private_->ui_.activeComboBox->addItems(items);
}
  
  
  
  

} // namespace Seg3D

