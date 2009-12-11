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

#include "ToolBoxWidget.h"

namespace Seg3D  {

//  ViewAction::ViewAction(QAction* parent, int column1, int column2) : 
//  QObject(parent)
//  {
    
ToolBoxWidget::ToolBoxWidget(QWidget* parent)
{

  
  
  
}
  
int ToolBoxWidget::insertItem ( int index, QWidget * item, const QString &label)
{
  if ( !item )
    return -1;
  
  
  
  Page newPage;
  newPage.tool_ = item;
  
  //  --- Begin QT Widget Design --- //
  newPage.page_ = new QWidget;
  newPage.page_->setStyleSheet(QString::fromUtf8("QPushButton{\n"
                                                 "  \n"
                                                 "  margin-right: 7px;\n"
                                                 "  height: 24px;\n"
                                                 "  text-align: left;\n"
                                                 "  padding-left: 4px;\n"
                                                 "  color: white;\n"
                                                 "  font: bold;\n"
                                                 "\n"
                                                 "}\n"
                                                 "\n"
                                                 "QPushButton:pressed{\n"
                                                 "  color: gray;\n"
                                                 "\n"
                                                 "\n"
                                                 "}\n"
                                                 "\n"
                                                 "\n"
                                                 "\n"
                                                 "QToolButton{\n"
                                                 "  background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
                                                 "  border: none;\n"
                                                 "\n"
                                                 "}\n"
                                                 "\n"
                                                 "QWidget#header_{\n"
                                                 "  \n"
                                                 "  background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 255), stop:1 rgba(136, 0, 0, 0 ));\n"
                                                 "    border-radius: 4px;\n"
                                                 "  border: 1px solid black;\n"
                                                 "}\n"
                                                 "\n"
                                                 "QWidget#background_{\n"
                                                 "  background-color: rgb(128, 0, 0);\n"
                                                 "    border-radius: 6px;\n"
                                                 "}"));
  newPage.vLayout_ = new QVBoxLayout(newPage.page_);
  newPage.vLayout_->setSpacing(2);
  newPage.vLayout_->setContentsMargins(11, 11, 11, 11);
  newPage.vLayout_->setObjectName(QString::fromUtf8("vLayout_"));
  newPage.vLayout_->setContentsMargins(0, 0, 0, 0);
  newPage.background_ = new QWidget(newPage.page_);
  newPage.background_->setObjectName(QString::fromUtf8("background_"));
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(newPage.background_->sizePolicy().hasHeightForWidth());
  newPage.background_->setSizePolicy(sizePolicy);
  newPage.background_->setMinimumSize(QSize(235, 29));
  newPage.background_->setMaximumSize(QSize(235, 29));
  newPage.hLayout_2 = new QHBoxLayout(newPage.background_);
  newPage.hLayout_2->setSpacing(0);
  newPage.hLayout_2->setContentsMargins(0, 0, 0, 0);
  newPage.hLayout_2->setObjectName(QString::fromUtf8("hLayout_2"));
  newPage.header_ = new QWidget(newPage.background_);
  newPage.header_->setObjectName(QString::fromUtf8("header_"));
  sizePolicy.setHeightForWidth(newPage.header_->sizePolicy().hasHeightForWidth());
  newPage.header_->setSizePolicy(sizePolicy);
  newPage.header_->setMinimumSize(QSize(235, 29));
  newPage.header_->setMaximumSize(QSize(235, 29));
  newPage.hLayout_ = new QHBoxLayout(newPage.header_);
  newPage.hLayout_->setSpacing(1);
  newPage.hLayout_->setContentsMargins(0, 0, 0, 0);
  newPage.hLayout_->setObjectName(QString::fromUtf8("hLayout_"));
  newPage.activate_button_ = new QPushButton(newPage.header_);
  newPage.activate_button_->setText(label);
  newPage.activate_button_->setObjectName(QString::fromUtf8("activate_button_"));
  newPage.activate_button_->setCheckable(false);
  newPage.activate_button_->setFlat(true);
  newPage.hLayout_->addWidget(newPage.activate_button_);
  newPage.help_button_ = new QToolButton(newPage.header_);
  newPage.help_button_->setObjectName(QString::fromUtf8("help_button_"));
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/new/prefix1/show_info.png"), QSize(), QIcon::Normal, QIcon::Off);
  newPage.help_button_->setIcon(icon);
  newPage.help_button_->setIconSize(QSize(25, 25));
  newPage.hLayout_->addWidget(newPage.help_button_);
  newPage.close_button_ = new QToolButton(newPage.header_);
  newPage.close_button_->setObjectName(QString::fromUtf8("close_button_"));
  QIcon icon1;
  icon1.addFile(QString::fromUtf8(":/new/prefix1/close.png"), QSize(), QIcon::Normal, QIcon::Off);
  newPage.close_button_->setIcon(icon1);
  newPage.close_button_->setIconSize(QSize(25, 25));
  newPage.hLayout_->addWidget(newPage.close_button_);
  newPage.hLayout_->setStretch(0, 1);
  newPage.hLayout_2->addWidget(newPage.header_);
  newPage.vLayout_->addWidget(newPage.background_);
  newPage.tool_frame_ = new QFrame(newPage.page_);
  newPage.tool_frame_->setObjectName(QString::fromUtf8("tool_frame_"));
  newPage.tool_frame_->setFrameShape(QFrame::StyledPanel);
  newPage.tool_frame_->setFrameShadow(QFrame::Raised);
  newPage.vLayout_->addWidget(newPage.tool_frame_);
  newPage.vLayout_->setStretch(1, 1);
  //  --- End QT Widget Design --- //
  
  
  // Begin Connections 
  connect(newPage.activate_button_, SIGNAL( clicked() ), this, SLOT(buttonClicked()));
  connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(itemDestroyed(QObject*)));
  
  if ( index < 0 || index >= (int)tool_list_->count() ) 
  {
    index = tool_list_->count();
    tool_list_->append(newPage);
    if(index == 0){}
      //setCurrentIndex (index);
  }
  
  
  
  
  
  
  
  
  return 1;
}

  void ToolBoxWidget::updatePages()
  {
    QWidget * tempWidget = currentPage ? currentPage->background_ : 0;
    bool after = false;
    
    for (PageList::ConstIterator i = tool_list_->constBegin(); i != tool_list_->constEnd(); i++)
    {
      if (after) {
        (*i).background_->setStyleSheet("QWidget#background{background-color: gray;}");
        (*i).page_->resize(235, 300);
      }
      else {
        (*i).background_->setStyleSheet("QWidget#background{background-color: rgb(128, 0, 0;}");
        (*i).page_->resize(235, 29);
      }
      after = (*i).background_ == tempWidget;
    }
    
  }
  
  ToolBoxWidget::Page *ToolBoxWidget::page ( int index ) 
  {
    if( index >= 0 && index < (int)tool_list_->size() )
      {
        //return &tool_list_->at(index);
      }
    return 0;
  }
  

ToolBoxWidget::~ToolBoxWidget()
{
  
}



} //end Seg3D namespace