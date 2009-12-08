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

#include <Interface/AppInterface/ToolWidget.h>

namespace Seg3D {

ToolWidget::ToolWidget(QWidget* parent, ToolHandle& tool) :
  QFrame(parent),
  tool_(tool)
{
  setFrameStyle(QFrame::NoFrame);
  // Generate a vertical layout for the tool widget
  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->setSpacing(0);
  vbox->setContentsMargins(0,0,0,0);
  setLayout(vbox);

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setSpacing(0);
  hbox->setContentsMargins(0,0,0,0);

  QWidget* header = new QWidget;
  header->setLayout(hbox);

  help_button_ = new QToolButton;
  help_button_->setIcon(style()->standardIcon(QStyle::SP_TitleBarContextHelpButton));
  help_button_->setFixedHeight(12);

  close_button_ = new QToolButton;
  close_button_->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  close_button_->setFixedHeight(12);
  
  hbox->addStretch();
  hbox->addWidget(help_button_);
  hbox->addWidget(close_button_);
    
  vbox->addWidget(header);
  vbox->addStretch();
  
  connect(close_button_,SIGNAL(released()),this,SLOT(close_tool()));
}

ToolWidget::~ToolWidget()
{
}



} //end namespace Seg3D

