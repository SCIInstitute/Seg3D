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


#include "FlipToolInterface.h"
#include "ui_FlipToolInterface.h"

namespace Seg3D {
  
SCI_REGISTER_TOOLINTERFACE(FlipToolInterface)
  
class FlipToolInterfacePrivate {
  public:
    Ui::FlipToolInterface ui_;
};
  
FlipToolInterface::FlipToolInterface() :
private_(new FlipToolInterfacePrivate)
{
  
}

FlipToolInterface::~FlipToolInterface()
{
}

bool
FlipToolInterface::build_widget(QFrame* frame)
{
  private_->ui_.setupUi(frame);
  SCI_LOG_DEBUG("Finished Building a Flip Tool"); 
  makeConnections();
  return (true);
}
  
//  --- Function for making signal slots connections ---  //
void FlipToolInterface::makeConnections()
{
  connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
  connect(private_->ui_.flipAxialButton, SIGNAL(clicked()), this, SLOT(senseFlipAxial()));
  connect(private_->ui_.flipCoronalButton, SIGNAL(clicked()), this, SLOT(senseFlipCoronal()));
  connect(private_->ui_.flipSagittalButton, SIGNAL(clicked()), this, SLOT(senseFlipSagittal()));
  connect(private_->ui_.rotateACButton, SIGNAL(clicked()), this, SLOT(senseRotateAxialCoronal()));
  connect(private_->ui_.rotateASButton, SIGNAL(clicked()), this, SLOT(senseRotateAxialSagital()));
  connect(private_->ui_.rotateSAButton, SIGNAL(clicked()), this, SLOT(senseRotateSagittalAxial()));
}


//  --- Private slots for custom signals ---  //
void FlipToolInterface::senseActiveChanged(int active)
{
  Q_EMIT activeChanged(active);
}

void FlipToolInterface::senseFlipAxial()
{
  Q_EMIT flipAxial();
}

void FlipToolInterface::senseFlipCoronal()
{
  Q_EMIT flipCoronal();
}

void FlipToolInterface::senseFlipSagittal()
{
  Q_EMIT flipSagittal();
}

void FlipToolInterface::senseRotateAxialCoronal()
{
  Q_EMIT rotateAxialCoronal();
}

void FlipToolInterface::senseRotateAxialSagital()
{
  Q_EMIT rotateAxialSagittal();
}

void FlipToolInterface::senseRotateSagittalAxial()
{
  Q_EMIT rotateSagittalAxial();
}

//  --- Public slots for setting widget values ---  //
void FlipToolInterface::setActive(int active)
{
  private_->ui_.activeComboBox->setCurrentIndex(active);
}

void FlipToolInterface::addToActive(QStringList &items)
{
  private_->ui_.activeComboBox->addItems(items);
}
  

  
  
} // namespace Seg3D
