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

// Qt includes
#include <QUrl>
#include <QDesktopServices>

namespace Seg3D  {

// constructor
ToolBoxWidget::ToolBoxWidget(QWidget* parent) :
  QScrollArea(parent)
{

  { // setup the icons
    active_close_icon_.addFile(QString::fromUtf8(":/Images/CloseWhite.png"), 
                               QSize(), QIcon::Normal, QIcon::Off);    
    inactive_close_icon_.addFile(QString::fromUtf8(":/Images/Close.png"), 
                                 QSize(), QIcon::Normal, QIcon::Off);

    active_help_icon_.addFile(QString::fromUtf8(":/Images/HelpWhite.png"), 
                               QSize(), QIcon::Normal, QIcon::Off);    
    inactive_help_icon_.addFile(QString::fromUtf8(":/Images/Help.png"), 
                                 QSize(), QIcon::Normal, QIcon::Off);
  }

  // set up the ToolBoxWidget's scroll box parameters
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setContentsMargins(1, 1, 1, 1);
  setWidgetResizable(true);
  
  // build a base QWidget
  main_ = new QWidget(this);
  setWidget(main_);
  
  // create a QVBoxLayout for our base QWidget
  main_layout_ = new QVBoxLayout( main_ );
  main_layout_->setContentsMargins(1, 1, 1, 1);
  main_layout_->setSpacing(1);
  
  // create a layout for our toolboxe
  tool_layout_ = new QVBoxLayout;
  main_layout_->addLayout(tool_layout_);
  main_layout_->addStretch();
  main_->setLayout(main_layout_);
  main_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

} // end constructor
    
// destructor
ToolBoxWidget::~ToolBoxWidget()
{ } // end destructor


  void ToolBoxWidget::add_tool(QWidget * tool, const QString &label, boost::function<void ()> close_function, boost::function<void ()> activate_function, QUrl help_url)
  {
    // return right away if somehow we got here without a tool
    if ( !tool ) return;
    
    PageHandle page_handle(new Page);
    page_handle->tool_ = tool;
    page_handle->url_ = help_url;
      
    //  --- Begin QT Widget Design --- //
    
    page_handle->page_ = new QWidget();
    page_handle->page_->setObjectName(QString::fromUtf8("page_"));
    page_handle->page_->resize(250, 36);
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(page_handle->page_->sizePolicy().hasHeightForWidth());
    page_handle->page_->setSizePolicy(sizePolicy);
    page_handle->page_->setStyleSheet(QString::fromUtf8(
                    "QPushButton#activate_button_{\n"
                    " height: 24px;\n"
                    " text-align: left;\n"
                    " color: white;\n"
                    "}\n"
                    "\n"
                    "QPushButton#activate_button_:pressed{\n"
                    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
                    " border: 1px solid  qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
                    " color:  rgb(68, 68, 68);\n"
                    "}\n"
                    "\n"
                    "QFrame#page_frame_{\n"
                    " border-radius: 4px;\n"
                    " border: 1px solid gray;\n"
                    "}\n"
                    "\n"
                    "QToolButton#close_button_{\n"
                    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
                    " border-radius: 4px;\n"
                    "}\n"
                    "\n"
                    "QToolButton#close_button_:pressed{\n"
                    " background-color: rgb(68, 68, 68);\n"
                    " border-radius: 4px;\n"
                    "}\n"
                    "\n"
                    "QToolButton#help_button_{\n"
                    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0)"
                                    ", stop:1 rgba(136, 0, 0, 0));\n"
                    " border-radius: 4px;\n"
                    "}\n"
                    "\n"
                    "QToolButton#help_button_:pressed{\n"
                    " background-color: rgb(68, 68, 68);\n"
                    " border-radius: 4px;\n"
                    "}\n"
                    "\n"
                    "QWidget#page_header_{\n"
                    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(25, 25, 25, 0), stop:0.753769 rgba(0, 0, 0, 100), stop:1 rgba(0, 0, 0, 84));\n"
                    "    border-radius: 4px;\n"
                    " border: 1px solid black;\n"
                    "}\n"
                    "\n"
                    "QWidget#page_background_{\n"
                    " background-color: rgb(255, 128, 0);\n"
                    "    border-radius: 6px;\n"
                    "}\n"
                    "\n"
                    "QWidget#page_{\n"
                    " background-color: gray;\n"
                    " border-radius:6px;\n"
                    "}\n"
                    "\n"
                    "QWidget#tool_{\n"
                    " background-color: rgb(240, 240, 240);\n"
                    " border-radius: 4px;\n"
                    " border: 1px solid rgb(90, 90, 90);\n"
                    "    background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(221, 221, 221, 255), stop:0.155779 rgba(228, 228, 228, 255), stop:1 rgba(202, 204, 204, 255));\n"
                    "}\n"
                    "\n"
                    ""));
    page_handle->verticalLayout_ = new QVBoxLayout(page_handle->page_);
    page_handle->verticalLayout_->setSpacing(0);
    page_handle->verticalLayout_->setContentsMargins(1, 1, 1, 1);
    page_handle->verticalLayout_->setObjectName(QString::fromUtf8("verticalLayout_"));
    page_handle->page_background_ = new QWidget(page_handle->page_);
    page_handle->page_background_->setObjectName(QString::fromUtf8("page_background_"));
    sizePolicy.setHeightForWidth(page_handle->page_background_->sizePolicy().hasHeightForWidth());
    page_handle->page_background_->setSizePolicy(sizePolicy);
    page_handle->page_background_->setMinimumSize(QSize(201, 21));
    page_handle->page_background_->setMaximumSize(QSize(16777215, 21));
    page_handle->horizontalLayout_1_ = new QHBoxLayout(page_handle->page_background_);
    page_handle->horizontalLayout_1_->setSpacing(0);
    page_handle->horizontalLayout_1_->setContentsMargins(0, 0, 0, 0);
    page_handle->horizontalLayout_1_->setObjectName(QString::fromUtf8("horizontalLayout_1_"));
    page_handle->page_header_ = new QWidget(page_handle->page_background_);
    page_handle->page_header_->setObjectName(QString::fromUtf8("page_header_"));
    QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(page_handle->page_header_->sizePolicy().hasHeightForWidth());
    page_handle->page_header_->setSizePolicy(sizePolicy1);
    page_handle->page_header_->setMinimumSize(QSize(215, 21));
    page_handle->page_header_->setMaximumSize(QSize(16777215, 21));
    page_handle->horizontalLayout = new QHBoxLayout(page_handle->page_header_);
    page_handle->horizontalLayout->setSpacing(0);
    page_handle->horizontalLayout->setContentsMargins(0, 0, 0, 0);
    page_handle->horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    page_handle->activate_button_ = new QPushButton(page_handle->page_header_);
    page_handle->activate_button_->setObjectName(QString::fromUtf8("activate_button_"));
    sizePolicy.setHeightForWidth(page_handle->activate_button_->sizePolicy().hasHeightForWidth());
    page_handle->activate_button_->setSizePolicy(sizePolicy);
    page_handle->activate_button_->setMinimumSize(QSize(152, 21));
    page_handle->activate_button_->setStyleSheet(QString::fromUtf8("font: bold;"));
    page_handle->activate_button_->setCheckable(false);
    page_handle->activate_button_->setFlat(true);

    page_handle->activate_button_->setText(label);

    page_handle->horizontalLayout->addWidget(page_handle->activate_button_);

    page_handle->help_button_ = new QToolButton(page_handle->page_header_);
    page_handle->help_button_->setObjectName(QString::fromUtf8("help_button_"));
    page_handle->help_button_->setMinimumSize(QSize(19, 19));
    page_handle->help_button_->setMaximumSize(QSize(19, 19));
    
    ///  ---  This is where we add the icon's for the help button --- //
    page_handle->help_button_->setIcon(active_help_icon_);
    page_handle->help_button_->setIconSize(QSize(16, 16));
    
    page_handle->horizontalLayout->addWidget(page_handle->help_button_);
    
    page_handle->close_button_ = new QToolButton(page_handle->page_header_);
    page_handle->close_button_->setObjectName(QString::fromUtf8("close_button_"));
    page_handle->close_button_->setMinimumSize(QSize(19, 19));
    page_handle->close_button_->setMaximumSize(QSize(19, 19));
    
    ///  ---  This is where we add the icon's for the close button --- //
    page_handle->close_button_->setIcon(active_close_icon_);
    page_handle->close_button_->setIconSize(QSize(18, 18));
    
    page_handle->horizontalLayout->addWidget(page_handle->close_button_);

    page_handle->horizontalLayout_1_->addWidget(page_handle->page_header_);

    page_handle->verticalLayout_->addWidget(page_handle->page_background_);

    page_handle->tool_frame_ = new QFrame(page_handle->page_);
    page_handle->tool_frame_->setObjectName(QString::fromUtf8("tool_frame_"));
    QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(page_handle->tool_frame_->sizePolicy().hasHeightForWidth());
    page_handle->tool_frame_->setSizePolicy(sizePolicy2);
    page_handle->tool_frame_->setMinimumSize(QSize(0, 0));
    page_handle->tool_frame_->setMaximumSize(QSize(16777215, 16777215));
    page_handle->tool_frame_->setFrameShape(QFrame::StyledPanel);
    page_handle->tool_frame_->setFrameShadow(QFrame::Raised);
    page_handle->tool_frame_->setLineWidth(0);
    page_handle->tool_layout_ = new QHBoxLayout(page_handle->tool_frame_);
    page_handle->tool_layout_->setSpacing(2);
    page_handle->tool_layout_->setContentsMargins(0, 0, 0, 0);
    page_handle->tool_layout_->setObjectName(QString::fromUtf8("tool_layout_"));

    page_handle->tool_layout_->addWidget(page_handle->tool_);

    page_handle->verticalLayout_->addWidget(page_handle->tool_frame_);
    //  --- End QT Widget Design --- //
    
    // Begin Connections 
    connect(page_handle->help_button_, SIGNAL( clicked() ), this, SLOT(help_button_clicked()));
    QtBridge::connect(page_handle->activate_button_, activate_function);
    QtBridge::connect(page_handle->close_button_, close_function);
     
    tool_list_.append(page_handle);
    
    // Add tool to the layout
    tool_layout_->addWidget(page_handle->page_);

    // Set active tool
    set_active_tool(page_handle->tool_);
    
    // Report which tool was added
    SCI_LOG_MESSAGE(label.toStdString() + " - has been successfully added");
  } // end add_tool
    

    
  void ToolBoxWidget::set_active_tool( QWidget *tool )
  {
    if(tool == get_active_tool())
      return;
    // Find the index that corresponds to the tool
    QList<PageHandle>::iterator it = tool_list_.begin();
    QList<PageHandle>::iterator it_end = tool_list_.end();
    
    while (it != it_end) 
    { 
      if ((*it)->tool_ != tool)
      { 
        if (!((*it)->tool_frame_->isHidden()))
        {
          (*it)->page_background_->setStyleSheet(QString::fromUtf8("QWidget#page_background_ { background-color: rgb(220, 220, 220); }"));
          (*it)->activate_button_->setStyleSheet(QString::fromUtf8(
                                        "QPushButton{\n"
                                        " \n"
                                        " margin-right: 7px;\n"
                                        " height: 24px;\n"
                                        " text-align: left;\n"
                                        " padding-left: 4px;\n"
                                        " color: rgb(25, 25, 25);\n"                           
                                        "\n"
                                        "}\n"));

          (*it)->close_button_->setIcon(inactive_close_icon_);
          (*it)->help_button_->setIcon(inactive_help_icon_);
          (*it)->tool_frame_->hide();
        }
      }
      ++it; 
    }

    int index = 0;
    it = tool_list_.begin();
    it_end = tool_list_.end();
    
    while( it != it_end ) 
    { 
      if( (*it)->tool_ == tool )
      {
        active_index_ = index;
        active_tool_ = (*it)->tool_;
        active_page_ = *it;

        if( (*it)->tool_frame_->isHidden() )
        {
          //set the size of the active page as well as the color of its header
          (*it)->page_background_->setStyleSheet(QString::fromUtf8(
                      "QWidget#page_background_ { background-color: rgb(255, 128, 0); }"));
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
          
          ///  ---  This is where we change the icon's for the help button --- //
          (*it)->close_button_->setIcon(active_close_icon_);
          (*it)->help_button_->setIcon(active_help_icon_);
          (*it)->tool_frame_->show();
          Q_EMIT currentChanged ((*it)->activate_button_->text());
        }
      }
      ++it; index++; 
    } 
  } // end set_active_tool
    
    
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
    return index;
  }
    
    
    
  void ToolBoxWidget::set_active_index( int index )
  {
    // Find the index that corresponds to the tool
    QList<PageHandle>::iterator it = tool_list_.begin();
    QList<PageHandle>::iterator it_end = tool_list_.end();

    if (index >= 0) 
    {
      int counter = 0;
      
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
  } // end set_active_index

    
  // Find the index that corresponds to the tool
  void ToolBoxWidget::remove_tool( int index )
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
       
      }
      ++it; counter++;
    }
    
    // Set the previous tool to active if the one to be deleted is active.
    if (active_index_ == index) { set_active_index(index-1); }
  } // end remove_tool
    
   
    
  void ToolBoxWidget:: help_button_clicked()
  {
    SCI_LOG_MESSAGE( "Help button has been clicked." );
    QToolButton *help_button = ::qobject_cast<QToolButton*>(sender());

    for( PageList::iterator it = tool_list_.begin(); it != tool_list_.end(); ++it )
    {
      if( (*it)->help_button_ == help_button )
      {
        QDesktopServices::openUrl( (*it)->url_ );
      }
    } 
  } // end help_button_clicked

} //end Seg3D namespace