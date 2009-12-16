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
#include <Utils/Core/Log.h>

#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>



namespace Seg3D  {

    
ToolBoxWidget::ToolBoxWidget(QWidget* parent)
{
  main_layout_ = new QVBoxLayout( this );
  main_layout_->setContentsMargins(4, 4, 4, 0);
  main_layout_->setSpacing(2);
  
//  main_layout = new QGridLayout( this );
//  main_layout->setContentsMargins(4, 4, 4, 0);
//  main_layout->setSpacing(2);
  
  
}
  
void ToolBoxWidget::add_tool(QWidget * tool, const QString &label)
{
  if ( !tool )
    return;
  
  QSharedPointer<Page> page_handle_ (new Page);
  
  
  page_handle_->tool_ = tool;
    
  //  --- Begin QT Widget Design --- //
  
  page_handle_->page_ = new QWidget;

  page_handle_->page_->setStyleSheet(QString::fromUtf8("QPushButton{\n"
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
                                                 "QWidget#page_{ background-color: black; }\n"
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
                                                 "}\n"
                                                 "QFrame#tool_frame_{ border-radius: 4px; border: 1px solid gray; }     "));
  page_handle_->vLayout_ = new QVBoxLayout(page_handle_->page_);
  page_handle_->vLayout_->setSpacing(1);
  page_handle_->vLayout_->setContentsMargins(0, 0, 0, 0);
  page_handle_->vLayout_->setObjectName(QString::fromUtf8("vLayout_"));
  page_handle_->vLayout_->setContentsMargins(0, 0, 0, 0);
  page_handle_->background_ = new QWidget(page_handle_->page_);
  page_handle_->background_->setObjectName(QString::fromUtf8("background_"));
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(page_handle_->background_->sizePolicy().hasHeightForWidth());
  page_handle_->background_->setSizePolicy(sizePolicy);
  page_handle_->background_->setMinimumSize(QSize(215, 29));
  page_handle_->background_->setMaximumSize(QSize(215, 29));
  page_handle_->hLayout_2 = new QHBoxLayout(page_handle_->background_);
  page_handle_->hLayout_2->setSpacing(0);
  page_handle_->hLayout_2->setContentsMargins(0, 0, 0, 0);
  page_handle_->hLayout_2->setObjectName(QString::fromUtf8("hLayout_2"));
  page_handle_->header_ = new QWidget(page_handle_->background_);
  page_handle_->header_->setObjectName(QString::fromUtf8("header_"));
  sizePolicy.setHeightForWidth(page_handle_->header_->sizePolicy().hasHeightForWidth());
  page_handle_->header_->setSizePolicy(sizePolicy);
  page_handle_->header_->setMinimumSize(QSize(215, 29));
  page_handle_->header_->setMaximumSize(QSize(215, 29));
  page_handle_->hLayout_ = new QHBoxLayout(page_handle_->header_);
  page_handle_->hLayout_->setSpacing(1);
  page_handle_->hLayout_->setContentsMargins(0, 0, 0, 0);
  page_handle_->hLayout_->setObjectName(QString::fromUtf8("hLayout_"));
  page_handle_->activate_button_ = new QPushButton(page_handle_->header_);
  page_handle_->activate_button_->setText(label);
  page_handle_->activate_button_->setObjectName(QString::fromUtf8("activate_button_"));
  page_handle_->activate_button_->setCheckable(false);
  page_handle_->activate_button_->setFlat(true);
  page_handle_->hLayout_->addWidget(page_handle_->activate_button_);
  page_handle_->help_button_ = new QToolButton(page_handle_->header_);
  page_handle_->help_button_->setObjectName(QString::fromUtf8("help_button_"));
  
  ///  ---  This is where we add the icon's for the help button --- //
  //QIcon icon;
  //icon.addFile(QString::fromUtf8(":/new/prefix1/show_info.png"), QSize(), QIcon::Normal, QIcon::Off);
  //page_handle_->help_button_->setIcon(icon);
  //page_handle_->help_button_->setIconSize(QSize(25, 25));
  page_handle_->hLayout_->addWidget(page_handle_->help_button_);
  page_handle_->close_button_ = new QToolButton(page_handle_->header_);
  page_handle_->close_button_->setObjectName(QString::fromUtf8("close_button_"));
  
   ///  ---  This is where we add the icon's for the close button --- //
  //QIcon icon1;
  //icon1.addFile(QString::fromUtf8(":/new/prefix1/close.png"), QSize(), QIcon::Normal, QIcon::Off);
  //page_handle_->close_button_->setIcon(icon1);
  //page_handle_->close_button_->setIconSize(QSize(25, 25));
  page_handle_->hLayout_->addWidget(page_handle_->close_button_);
  page_handle_->hLayout_->setStretch(0, 1);
  page_handle_->hLayout_2->addWidget(page_handle_->header_);
  page_handle_->vLayout_->addWidget(page_handle_->background_);
  
  page_handle_->tool_frame_ = new QFrame(page_handle_->page_);
  page_handle_->tool_frame_->setObjectName(QString::fromUtf8("tool_frame_"));
  page_handle_->tool_frame_->setFrameShape(QFrame::StyledPanel);
  page_handle_->tool_frame_->setFrameShadow(QFrame::Raised);
  
  page_handle_->vLayout_2 = new QVBoxLayout(page_handle_->tool_frame_);
  page_handle_->vLayout_2->setSpacing(0);
  page_handle_->vLayout_2->setContentsMargins(2, 2, 2, 2);
  page_handle_->vLayout_2->setObjectName(QString::fromUtf8("vLayout_2"));
  
  page_handle_->vLayout_2->addWidget(page_handle_->tool_, 0, Qt::AlignTop);
  
  page_handle_->vLayout_->addWidget(page_handle_->tool_frame_);
  page_handle_->vLayout_->setStretch(1, 1);
  //  --- End QT Widget Design --- //
  
  // Begin Connections 
  connect(page_handle_->activate_button_, SIGNAL( clicked() ), this, SLOT(buttonClicked()));
  connect(tool, SIGNAL(destroyed(QObject*)), this, SLOT(itemDestroyed(QObject*)));
  
  tool_list_.insert(0, page_handle_);
  
  // Add tool to the layout
  main_layout_->addWidget(page_handle_->page_, 0, Qt::AlignTop|Qt::AlignCenter);

  // Set active tool
  set_active_tool(page_handle_->tool_);
  
  // Report which tool was added
  SCI_LOG_MESSAGE(label.toStdString() + " - has been successfully added");
  
}
  
  
void ToolBoxWidget::set_active_index( int index )
{
  active_index_ = index;
  set_active_tool(tool_list_.at(index)->tool_);
}
  
  
void ToolBoxWidget::set_active_tool( QWidget *tool )
{
  int index = 0;
  while (!(tool_list_.at(index)->tool_ == tool)) {
    index++;
  }
  
  QSharedPointer<Page> page_handle_ (new Page);
  page_handle_ = tool_list_.at(index);
  
  std::string s = boost::lexical_cast<std::string>(index);
  SCI_LOG_MESSAGE("The active tool is: "+ s);
  
  active_index_ = index;
  active_tool_ = tool_list_.at(index)->tool_;
  active_page_ = tool_list_.at(index);
  
  active_page_->page_->setFixedSize(215, 235);
  
  //TODO - need to get the correct size of the tool to set the window size properly
  active_page_->tool_frame_->setFixedSize(215, (235-30));
  
  std::string h = boost::lexical_cast<std::string>(active_page_->tool_->height());
  SCI_LOG_MESSAGE("The the height of the active tool is: "+ h);
  
  
  active_page_->background_->setStyleSheet(QString::fromUtf8("QWidget#background_ { background-color: rgb(128, 0, 0); }"));
  active_page_->activate_button_->setStyleSheet(QString::fromUtf8("QPushButton{\n"
                                                                      " \n"
                                                                      " margin-right: 7px;\n"
                                                                      " height: 24px;\n"
                                                                      " text-align: left;\n"
                                                                      " padding-left: 4px;\n"
                                                                      " color: white;\n"
                                                                      " font: bold;\n"
                                                                      "\n"
                                                                      "}\n"));
  
  for(int i=0; i < tool_list_.size(); i++)
  {
    if(i != active_index_)
    {
      tool_list_.at(i)->background_->setStyleSheet(QString::fromUtf8("QWidget#background_ { background-color: rgb(220, 220, 220); }"));
      tool_list_.at(i)->activate_button_->setStyleSheet(QString::fromUtf8("QPushButton{\n"
                                                                          " \n"
                                                                          " margin-right: 7px;\n"
                                                                          " height: 24px;\n"
                                                                          " text-align: left;\n"
                                                                          " padding-left: 4px;\n"
                                                                          " color: rgb(180, 180, 180);\n"
                                                                          " font: bold;\n"
                                                                          "\n"
                                                                          "}\n"));
      tool_list_.at(i)->page_->setFixedSize(215, 29);
      tool_list_.at(i)->tool_frame_->setFixedSize(215, 0);
      SCI_LOG_MESSAGE(tool_list_.at(i)->activate_button_->text().toStdString() + " - tool has been set to inactive.");
    }
  }
  SCI_LOG_MESSAGE(active_page_->activate_button_->text().toStdString() + " - tool has been set active");
  Q_EMIT currentChanged (index_of(tool));
}
  
QSharedPointer<ToolBoxWidget::Page> ToolBoxWidget::page( QWidget *tool )
{
  if ( !tool )
    return QSharedPointer<Page>::QSharedPointer();
  
  for ( PageList::ConstIterator i = tool_list_.constBegin(); i != tool_list_.constEnd(); ++i )
    if ( (*i)->tool_ == tool )
      return (*i);
  return QSharedPointer<Page>::QSharedPointer();
}


  
int ToolBoxWidget::index_of( QWidget *tool ) 
{
  QSharedPointer <Page> page_ptr_ = page( tool );
  return page_ptr_ ? tool_list_.indexOf( page_ptr_ ) : -1;
}
  
  
void ToolBoxWidget::remove_tool( QWidget *tool )
{

  int index = index_of(tool);
  if (index >= 0)
  {
    disconnect(tool, SIGNAL(destroyed(QObject *)), this, SLOT(itemDestroyed(QObject*)));
    tool->setParent(this);
  }
  tool_removed(index);

}
  
 
void ToolBoxWidget::remove_tool(int index)
{
  if (index >= 0)
  {
    //TODO - remove the tool 
    //disconnect(tool_list_.at(index), SIGNAL(destroyed(QObject*)), this, SLOT(itemDestroyed(QObject*)));
  }
  tool_removed(index);
}
  
  
void ToolBoxWidget::tool_removed(int index)
{
  Q_UNUSED(index)
}
  
  
void ToolBoxWidget::buttonClicked()
{
  QPushButton *activate_button = ::qobject_cast<QPushButton*>(sender());
  QWidget* item =0;
    for ( PageList::ConstIterator i = tool_list_.constBegin(); i != tool_list_.constEnd(); ++i )
    {
     if ((*i)->activate_button_ == activate_button ) 
     {
       item = (*i)->tool_;
       break;
     } 
    }
  set_active_tool( item );
  SCI_LOG_MESSAGE("Button has been clicked.");
  
}

void ToolBoxWidget::itemDestroyed(QObject *object)
{
  //destroy!!
}
  

ToolBoxWidget::~ToolBoxWidget()
{
  
}
  
  



} //end Seg3D namespace