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


#include "InvertToolInterface.h"
#include "ui_InvertToolInterface.h"
#include <Interface/QtInterface/QtBridge.h>
#include <Application/Tools/InvertTool.h>

namespace Seg3D {
  
SCI_REGISTER_TOOLINTERFACE(InvertToolInterface)

class InvertToolInterfacePrivate {
  public:
    Ui::InvertToolInterface ui_;
};
  
InvertToolInterface::InvertToolInterface() :
private_(new InvertToolInterfacePrivate)
{
  
}

InvertToolInterface::~InvertToolInterface()
{
}
  
bool
InvertToolInterface::build_widget(QFrame* frame)
{
  private_->ui_.setupUi(frame);
  SCI_LOG_DEBUG("Finished building an Invert Tool");
  ToolHandle base_tool_ = tool();
  InvertTool* tool = dynamic_cast<InvertTool*>(base_tool_.get());
  QtBridge::connect(private_->ui_.replaceCheckBox,tool->replace_);
  //QtBridge::connect(private_->ui_.activeComboBox, tool->target_layer_);

  private_->ui_.activeComboBox->addItem(QString::fromUtf8("first item"));
  private_->ui_.activeComboBox->addItem(QString::fromUtf8("second item"));
  private_->ui_.activeComboBox->addItem(QString::fromUtf8("third item"));
  makeConnections();
  return (true);
  
}


//  --- Function for making signal slots connections ---  //
void InvertToolInterface::makeConnections()
{
  connect(private_->ui_.invertButton, SIGNAL(clicked()), this, SLOT(senseInverted()));
  connect(private_->ui_.activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
}
 

//  --- Private slots for custom signals ---  //
void InvertToolInterface::senseActiveChanged(int active)
{
  Q_EMIT activeChanged(active);
}

void InvertToolInterface::senseInverted()
{
  if(private_->ui_.replaceCheckBox->isChecked())
  {
    Q_EMIT invert(true);
  }
  else
  {
    Q_EMIT invert(false);
  }
}

//  --- Public slots for setting widget values ---  //
void InvertToolInterface::setActive(int active)
{
  private_->ui_.activeComboBox->setCurrentIndex(active);
}

void InvertToolInterface::addToActive(QStringList &items)
{
  private_->ui_.activeComboBox->addItems(items);
}



} // namespace Seg3D