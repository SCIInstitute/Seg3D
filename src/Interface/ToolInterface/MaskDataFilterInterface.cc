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


#include <Interface/ToolInterface/MaskDataFilterInterface.h>
#include "ui_MaskDataFilterInterface.h"

namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(MaskDataFilterInterface)
  
  
  class MaskDataFilterInterfacePrivate {
  public:
    Ui::MaskDataFilterInterface ui_;
  };
  
  
  MaskDataFilterInterface::MaskDataFilterInterface() :
  private_(new MaskDataFilterInterfacePrivate)
  {  
    
  }
  
  MaskDataFilterInterface::~MaskDataFilterInterface()
  {
  }
  
  bool
  MaskDataFilterInterface::build_widget(QFrame* frame)
  {
    
    private_->ui_.setupUi(frame);
    
    makeConnections();
    
    SCI_LOG_DEBUG("Finished building a Mask Data Filter Interface");
    return (true);
  }
  
  void MaskDataFilterInterface::makeConnections()
  {
    connect(private_->ui_.targetComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
    connect(private_->ui_.maskComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseMaskChanged(int)));

  }
  
  
  //  --- Private slots for custom signals ---  //
  void MaskDataFilterInterface::senseActiveChanged(int active)
  {
    private_->ui_.maskComboBox->setCurrentIndex(0);
    Q_EMIT activeChanged( active );
  }
  
  void MaskDataFilterInterface::senseMaskChanged(int mask)
  {
    Q_EMIT maskChanged(mask);
  }

  //  --- Public slots for setting widget values ---  //
  void MaskDataFilterInterface::setActive(int active)
  {
    private_->ui_.targetComboBox->setCurrentIndex(active);
  }
  
  void MaskDataFilterInterface::addToActive(QStringList &items)
  {
    private_->ui_.targetComboBox->addItems(items);
  }
  
  void MaskDataFilterInterface::setMask(int mask)
  {
    private_->ui_.maskComboBox->setCurrentIndex(mask);
  }
  
  void MaskDataFilterInterface::addToMask(QStringList &items)
  {
    private_->ui_.maskComboBox->addItems(items);
  }

  
  
  
} // namespace Seg3D

