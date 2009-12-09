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

ToolWidget::ToolWidget() :
  close_button_(0),
  help_button_(0),
  main_frame_(0)
{
}

ToolWidget::~ToolWidget()
{
}


bool
ToolWidget::create_widget(QWidget* parent, ToolHandle& tool)
{
  // Setup the parent widget: this one will be used for memory management of
  // this widget class
  setParent(parent);

    // Add the handle of the underlying tool to the widget
  set_tool(tool);

  // Generate a vertical layout for the tool widget
  QVBoxLayout* vbox = new QVBoxLayout;
  // Ensure it has some thight spacing
  vbox->setSpacing(0);
  vbox->setContentsMargins(0,0,0,0);
  setLayout(vbox);

  // Generate a layout for a horizontal bar
  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setSpacing(0);
  hbox->setContentsMargins(0,0,0,0);

  // Build the header widget
  QWidget* header = new QWidget;
  header->setLayout(hbox);

  // Add the help button
  help_button_ = new QToolButton;
  help_button_->setIcon(style()->standardIcon(QStyle::SP_TitleBarContextHelpButton));
  help_button_->setFixedHeight(12);

  // Add the close button
  close_button_ = new QToolButton;
  close_button_->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  close_button_->setFixedHeight(12);
  
  // Move the close and help widget to the right of the header bar
  hbox->addStretch();
  hbox->addWidget(help_button_);
  hbox->addWidget(close_button_);
  
  main_frame_ = new QFrame;  
          
  vbox->addWidget(header);
  vbox->addWidget(main_frame_);
  vbox->addStretch();
  
  // Ensure that when the close button is pressed, the tool will be closed
  // The button is connected to an internal slot that relays the signal to the
  // main application. 
  connect(close_button_,SIGNAL(released()),this,SLOT(close_tool()));
  
  // Create the custom part of the widget
  return ( build_widget(main_frame_));
}

bool
ToolWidget::build_widget(QFrame* frame)
{
  return (true);
}

} //end namespace Seg3D

