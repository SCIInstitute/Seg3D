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

#include <Interface/AppInterface/ToolBoxWidget.h>
#include <Utils/Core/Log.h>

#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <Interface/QtInterface/QtBridge.h>

namespace Seg3D  {
    
ToolBoxWidget::ToolBoxWidget(QWidget* parent) :
  QScrollArea(parent)
{
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setContentsMargins(1, 1, 1, 1);
  setWidgetResizable(true);

  main_ = new QWidget(this);
  setWidget(main_);

  main_layout_ = new QVBoxLayout( main_ );
  main_layout_->setContentsMargins(1, 1, 1, 1);
  main_layout_->setSpacing(1);
  
  tool_layout_ = new QVBoxLayout;
  main_layout_->addLayout(tool_layout_);
  main_layout_->addStretch();
  
  main_->setLayout(main_layout_);
}

ToolBoxWidget::~ToolBoxWidget()
{
}
  

void 
  ToolBoxWidget::add_tool(QWidget * tool, const QString &label, boost::function<void ()> close_function)
{
  if ( !tool ) return;
  
  PageHandle page_handle(new Page);
  page_handle->tool_ = tool;
    
  //  --- Begin QT Widget Design --- //
  
  page_handle->page_ = new QWidget;
  page_handle->page_->setStyleSheet(QString::fromUtf8("QPushButton#activate_button_{\n"
                                                 "  \n"
                                                 "  margin-right: 7px;\n"
                                                 "  height: 18px;\n"
                                                 "  text-align: left;\n"
                                                 "  padding-left: 4px;\n"
                                                 "  color: white;\n"
                                                 "  font: 11pt;\n"
                                                 "  font: bold;\n"
                                                 "}\n"
                                                 "\n"
                                                 "QPushButton#activate_button_:pressed{\n"
                                                 "  color: gray; \n"
                                                 "  margin-right: 7px; \n"
                                                 "  padding-left: 4px; \n"      
                                                 "  height: 18px; border-radius: 4px; \n"
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
                                                 "QToolButton:pressed{\n"
                                                 "  background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
                                                 "  background-color: black; border-radius: 4px;\n"
                                                 "  border: none;\n"
                                                 "\n"
                                                 "}\n"
                                                 "\n"      
                                                 "QWidget#header_{\n"
                                                 "  \n"
                                                 "background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(25, 25, 25, 0), stop:0.753769 rgba(0, 0, 0, 100), stop:1 rgba(0, 0, 0, 84));\n"
                                                 //"  background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 190), stop:1 rgba(136, 0, 0, 0 ));\n"
                                                 "    border-radius: 4px;\n"
                                                 "  border: 1px solid black;\n"
                                                 "}\n"
                                                 "\n"
                                                 "QWidget#background_{\n"
                                                 "  background-color: rgb(190, 0, 0);\n"
                                                 "    border-radius: 6px;\n"
                                                 "}\n"
                                                 "QFrame#tool_frame_{ border-radius: 4px; border: 1px solid gray; }     "));
  page_handle->vLayout_ = new QVBoxLayout(page_handle->page_);
  page_handle->vLayout_->setSpacing(1);
  page_handle->vLayout_->setContentsMargins(0, 0, 0, 0);
  page_handle->vLayout_->setObjectName(QString::fromUtf8("vLayout_"));
  page_handle->vLayout_->setContentsMargins(0, 0, 0, 0);
  page_handle->background_ = new QWidget(page_handle->page_);
  page_handle->background_->setObjectName(QString::fromUtf8("background_"));
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(page_handle->background_->sizePolicy().hasHeightForWidth());
  page_handle->background_->setSizePolicy(sizePolicy);
  page_handle->background_->setMinimumSize(QSize(215, 21));
  page_handle->background_->setMaximumSize(QSize(215, 21));
  page_handle->hLayout_2 = new QHBoxLayout(page_handle->background_);
  page_handle->hLayout_2->setSpacing(0);
  page_handle->hLayout_2->setContentsMargins(0, 0, 0, 0);
  page_handle->hLayout_2->setObjectName(QString::fromUtf8("hLayout_2"));
  page_handle->header_ = new QWidget(page_handle->background_);
  page_handle->header_->setObjectName(QString::fromUtf8("header_"));
  sizePolicy.setHeightForWidth(page_handle->header_->sizePolicy().hasHeightForWidth());
  page_handle->header_->setSizePolicy(sizePolicy);
  page_handle->header_->setMinimumSize(QSize(215, 21));
  page_handle->header_->setMaximumSize(QSize(215, 21));
  page_handle->hLayout_ = new QHBoxLayout(page_handle->header_);
  page_handle->hLayout_->setSpacing(1);
  page_handle->hLayout_->setContentsMargins(0, 0, 0, 0);
  page_handle->hLayout_->setObjectName(QString::fromUtf8("hLayout_"));
  page_handle->activate_button_ = new QPushButton(page_handle->header_);
  page_handle->activate_button_->setText(label);
  page_handle->activate_button_->setObjectName(QString::fromUtf8("activate_button_"));
  page_handle->activate_button_->setCheckable(false);
  page_handle->activate_button_->setFlat(true);
  page_handle->hLayout_->addWidget(page_handle->activate_button_);
  page_handle->help_button_ = new QToolButton(page_handle->header_);
  page_handle->help_button_->setObjectName(QString::fromUtf8("help_button_"));
  
  ///  ---  This is where we add the icon's for the help button --- //
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/Images/Help.png"), QSize(), QIcon::Normal, QIcon::Off);
  page_handle->help_button_->setIcon(icon);
  page_handle->help_button_->setIconSize(QSize(16, 16));
  
  page_handle->hLayout_->addWidget(page_handle->help_button_);
  
  page_handle->close_button_ = new QToolButton(page_handle->header_);
  QtBridge::connect(page_handle->close_button_, close_function);
  
  page_handle->close_button_->setObjectName(QString::fromUtf8("close_button_"));
  
   ///  ---  This is where we add the icon's for the close button --- //
  QIcon icon1;
  icon1.addFile(QString::fromUtf8(":/Images/Close.png"), QSize(), QIcon::Normal, QIcon::Off);
  page_handle->close_button_->setIcon(icon1);
  page_handle->close_button_->setIconSize(QSize(18, 18));
  
  page_handle->hLayout_->addWidget(page_handle->close_button_);
  page_handle->hLayout_->setStretch(0, 1);
  page_handle->hLayout_2->addWidget(page_handle->header_);
  page_handle->vLayout_->addWidget(page_handle->background_);
  
  page_handle->tool_frame_ = new QFrame(page_handle->page_);
  page_handle->tool_frame_->setObjectName(QString::fromUtf8("tool_frame_"));
  page_handle->tool_frame_->setFrameShape(QFrame::StyledPanel);
  page_handle->tool_frame_->setFrameShadow(QFrame::Raised);
  
  page_handle->vLayout_2 = new QVBoxLayout(page_handle->tool_frame_);
  page_handle->vLayout_2->setSpacing(0);
  page_handle->vLayout_2->setContentsMargins(0, 0, 0, 0);
  page_handle->vLayout_2->setObjectName(QString::fromUtf8("vLayout_2"));
  
  page_handle->vLayout_2->addWidget(page_handle->tool_, 0, Qt::AlignTop);
  
  page_handle->vLayout_->addWidget(page_handle->tool_frame_);
  page_handle->vLayout_->setStretch(1, 1);
  //  --- End QT Widget Design --- //
  
  // Begin Connections 
  connect(page_handle->activate_button_, SIGNAL( clicked() ), this, SLOT(activate_button_clicked()));
  connect(page_handle->help_button_, SIGNAL( clicked() ), this, SLOT(help_button_clicked()));
  
  connect(tool, SIGNAL(destroyed(QObject*)), this, SLOT(itemDestroyed(QObject*)));
  
  tool_list_.append(page_handle);
  
  // Add tool to the layout
  tool_layout_->addWidget(page_handle->page_, 0, Qt::AlignTop|Qt::AlignCenter);

  // Set active tool
  set_active_tool(page_handle->tool_);
  
  // Report which tool was added
  SCI_LOG_MESSAGE(label.toStdString() + " - has been successfully added");
}
  

  
void ToolBoxWidget::set_active_tool( QWidget *tool )
{
  // Find the index that corresponds to the tool
  QList<PageHandle>::iterator it = tool_list_.begin();
  QList<PageHandle>::iterator it_end = tool_list_.end();
  
  while (it != it_end) 
  { 
    if ((*it)->tool_ != tool)
    { 
      if (!((*it)->tool_frame_->isHidden()))
      {
        (*it)->background_->setStyleSheet(QString::fromUtf8("QWidget#background_ { background-color: rgb(220, 220, 220); }"));
        (*it)->activate_button_->setStyleSheet(QString::fromUtf8(
                                      "QPushButton{\n"
                                      " \n"
                                      " margin-right: 7px;\n"
                                      " height: 24px;\n"
                                      " text-align: left;\n"
                                      " padding-left: 4px;\n"
                                      " color: rgb(25, 25, 25);\n"                           
                                      " font: bold;\n"
                                      "\n"
                                      "}\n"));
        (*it)->tool_frame_->hide();
        //main_->adjustSize();
      }
    }
    ++it; 
  }

  size_t index = 0;
  it = tool_list_.begin();
  it_end = tool_list_.end();
  
  while (it != it_end) 
  { 
    if ((*it)->tool_ == tool)
    {
      active_index_ = index;
      active_tool_ = (*it)->tool_;
      active_page_ = *it;

      if ((*it)->tool_frame_->isHidden())
      {
        //set the size of the active page as well as the color of its header
        (*it)->background_->setStyleSheet(QString::fromUtf8(
                    "QWidget#background_ { background-color: rgb(190, 0, 0); }"));
        (*it)->activate_button_->setStyleSheet(QString::fromUtf8(
                                      "QPushButton{\n"
                                      " \n"
                                      " margin-right: 7px;\n"
                                      " height: 24px;\n"
                                      " text-align: left;\n"
                                      " padding-left: 4px;\n"
                                      " color: white;\n"
                                      " font: bold;\n"
                                      "\n"
                                      "}\n"));      
        (*it)->tool_frame_->show();
        //main_->adjustSize();
      }
    }
    ++it; index++; 
  }
  main_->adjustSize();
  Q_EMIT currentChanged (active_index_);
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
  int index = page_ptr_ ? tool_list_.indexOf( page_ptr_ ) : -1;
  
  //std::string h = boost::lexical_cast<std::string>(index);
  //SCI_LOG_MESSAGE("The the index of the tool is: "+ h);
  
  return index;
  
}
  
  
  
void ToolBoxWidget::set_active_index( int index )
{
  //std::string h = boost::lexical_cast<std::string>(index);
//  SCI_LOG_MESSAGE("The new index is: " + h);
  
  // Find the index that corresponds to the tool
  QList<PageHandle>::iterator it = tool_list_.begin();
  QList<PageHandle>::iterator it_end = tool_list_.end();
  
  
  int counter = 0;

  if (index >= 0) 
  {
    
    while (it != it_end) 
    { 
      if (index == counter) 
      {
        set_active_tool( (*it)->tool_);
      }
      ++it; counter++;
    }
    
    active_index_ = index;
  }
  else if(index < 0)
  {

    
    if(it != tool_list_.end())
    {
      set_active_tool( (*it)->tool_);
      //SCI_LOG_MESSAGE("the index is -1 so i am setting it to 0");
    }
    active_index_ = 0;
  }
  
}

  

void ToolBoxWidget::remove_tool(int index)
{

   
  // Find the index that corresponds to the tool
  QList<PageHandle>::iterator it = tool_list_.begin();
  QList<PageHandle>::iterator it_end = tool_list_.end();

  int counter = 0;

  // Loop through the tool list and remove the ones you dont need.
  while (it != it_end) 
  {
    if (counter == index) 
    {
      tool_layout_->removeWidget((*it)->page_);
         
      (*it)->page_->deleteLater();
      tool_list_.removeAt(index);
      //main_->adjustSize();
    }
    ++it; counter++;
  }
  
  // Set the previous tool to active if the one to be deleted is active.
  if (active_index_ == index) { set_active_index(index-1); }
  
  main_->adjustSize();
  tool_removed(index);
  
}
  
  
void ToolBoxWidget::tool_removed(int index)
{
  Q_UNUSED(index)
}
  
// slot for activate button  
void ToolBoxWidget::activate_button_clicked()
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
  SCI_LOG_MESSAGE("Activate button has been clicked.");
  
}


  
  void ToolBoxWidget:: help_button_clicked()
  {
    SCI_LOG_MESSAGE("Help button has been clicked.");
    QToolButton *help_button = ::qobject_cast<QToolButton*>(sender());
    int counter = 0;
    
    QList<PageHandle>::iterator it = tool_list_.begin();
    QList<PageHandle>::iterator it_end = tool_list_.end();
    
    while (it != it_end) 
    { 
      if ((*it)->help_button_ == help_button)
      {
        //TODO add help dialog box
      }
      ++it; counter++;
    }
  }
                                                                      

void ToolBoxWidget::itemDestroyed(QObject *object)
{
  //destroy!!
}
  
} //end Seg3D namespace