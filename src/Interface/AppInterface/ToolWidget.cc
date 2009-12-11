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
  
  QVBoxLayout *verticalLayout;
  QWidget *header;
  
  this->resize(243, 237);
  this->setFixedSize(243, 237);
  this->setStyleSheet(QString::fromUtf8("QPushButton{\n"
                                        " \n"
                                        " margin-right: 7px;\n"
                                        " height: 24px;\n"
                                        " text-align: left;\n"
                                        " padding-left: 4px;\n"
                                        " color: white;\n"
                                        " font: bold;\n"
                                        "\n"
                                        "}\n"
                                        "QPushButton:pressed{\n"
                                        " color: gray;\n"
                                        " background-color: none;\n"
                                        "\n"
                                        "}\n"
                                        "\n"
                                        "\n"
                                        "\n"
                                        "QToolButton{\n"
                                        " \n"
                                        "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                        "\n"
                                        "border: none;\n"
                                        "\n"
                                        "}\n"
                                        "\n"
                                        "QWidget#header{\n"
                                        " \n"
                                        " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 255), stop:1 rgba(136, 0, 0, 255));\n"
                                        "    border-radius: 4px;\n"
                                        " border: 1px solid black;\n"
                                        "}"));
  verticalLayout = new QVBoxLayout(this);
  verticalLayout->setSpacing(2);
  verticalLayout->setContentsMargins(4, 4, 4, 4);
  verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
  
  header = new QWidget(this);
  header->setObjectName(QString::fromUtf8("header"));
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(header->sizePolicy().hasHeightForWidth());
  header->setSizePolicy(sizePolicy);
  header->setMinimumSize(QSize(235, 29));
  header->setMaximumSize(QSize(235, 29));
  close_button_ = new QToolButton(header);
  close_button_->setObjectName(QString::fromUtf8("close_button_"));
  close_button_->setGeometry(QRect(208, 2, 25, 25));
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/new/pics/pics/close.png"), QSize(), QIcon::Normal, QIcon::Off);
  close_button_->setIcon(icon);
  close_button_->setIconSize(QSize(25, 25));
  help_button_ = new QToolButton(header);
  help_button_->setObjectName(QString::fromUtf8("help_button_"));
  help_button_->setGeometry(QRect(182, 2, 25, 25));
  QIcon icon1;
  icon1.addFile(QString::fromUtf8(":/new/pics/pics/show_info.png"), QSize(), QIcon::Normal, QIcon::Off);
  help_button_->setIcon(icon1);
  help_button_->setIconSize(QSize(25, 25));
  activate_button_ = new QPushButton(header);
  activate_button_->setObjectName(QString::fromUtf8("activate_button_"));
  activate_button_->setGeometry(QRect(4, 6, 171, 17));
  activate_button_->setCheckable(false);
  activate_button_->setFlat(true);
  activate_button_->setText("Paintbrush Tool");
  
  verticalLayout->addWidget(header);
  
  main_frame_ = new QFrame(this);
  main_frame_->setObjectName(QString::fromUtf8("frame"));
  main_frame_->setFrameShape(QFrame::StyledPanel);
  main_frame_->setFrameShadow(QFrame::Raised);
  
  verticalLayout->addWidget(main_frame_);
  
  
  

  // Generate a vertical layout for the tool widget
  //QVBoxLayout* vbox = new QVBoxLayout;
//  // Ensure it has some tight spacing
//  vbox->setSpacing(0);
//  vbox->setContentsMargins(0,0,0,0);
//  setLayout(vbox);
//
//  // Generate a layout for a horizontal bar
//  QHBoxLayout* hbox = new QHBoxLayout;
//  hbox->setSpacing(0);
//  hbox->setContentsMargins(0,0,0,0);
//
//  // Build the header widget
//  QWidget* header = new QWidget;
//  
//
//
//  
//  
//  header->setLayout(hbox);
//
//  // Add the help button
//  help_button_ = new QToolButton;
//  help_button_->setIcon(style()->standardIcon(QStyle::SP_TitleBarContextHelpButton));
//  help_button_->setFixedHeight(12);
//
//  // Add the close button
//  close_button_ = new QToolButton;
//  close_button_->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
//  close_button_->setFixedHeight(12);
//  
//  // Move the close and help widget to the right of the header bar
//  hbox->addStretch();
//  hbox->addWidget(help_button_);
//  hbox->addWidget(close_button_);
//  
//  main_frame_ = new QFrame;  
//          
//  vbox->addWidget(header);
//  vbox->addWidget(main_frame_);
//  vbox->addStretch();
  
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

