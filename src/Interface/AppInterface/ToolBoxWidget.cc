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
#include "ui_ToolBoxPageWidget.h"

namespace Seg3D  {

typedef std::vector<QWidget*> tool_page_list_type;

class ToolBoxWidgetPrivate {  
  public:
    Ui::ToolBoxPageWidget ui_;
  tool_page_list_type page_list_;
};
    
ToolBoxWidget::ToolBoxWidget(QWidget* parent) :
  QScrollArea(parent)
{

  {
    active_close_icon_.addFile(QString::fromUtf8(":/Images/CloseWhite.png"), 
                               QSize(), QIcon::Normal, QIcon::Off);    
    inactive_close_icon_.addFile(QString::fromUtf8(":/Images/Close.png"), 
                                 QSize(), QIcon::Normal, QIcon::Off);

    active_help_icon_.addFile(QString::fromUtf8(":/Images/HelpWhite.png"), 
                               QSize(), QIcon::Normal, QIcon::Off);    
    inactive_help_icon_.addFile(QString::fromUtf8(":/Images/Help.png"), 
                                 QSize(), QIcon::Normal, QIcon::Off);

  }

  private_ = ToolBoxWidgetPrivateHandle (new ToolBoxWidgetPrivate);

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
  main_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

}
    

ToolBoxWidget::~ToolBoxWidget()
{
}


void 
ToolBoxWidget::add_tool(QWidget * tool, 
                        const QString &label, 
                        boost::function<void ()> close_function, 
                        boost::function<void ()> activate_function, 
                        const std::string& help_url)
{
  if ( !tool ) return;

  //create a new base QWidget
  QWidget* new_page_ = new QWidget();
  private_->ui_.setupUi(new_page_);
  
  private_->ui_.url_->setText(QString::fromStdString(help_url));
  private_->ui_.url_->hide();
  
  private_->ui_.activate_button_->setText(label);
  
  private_->ui_.help_button_->setIcon(active_help_icon_);
  private_->ui_.help_button_->setIconSize(QSize(16, 16));
  
  private_->ui_.close_button_->setIcon(active_close_icon_);
  private_->ui_.close_button_->setIconSize(QSize(18, 18));
  
  // create a new widget, send new_page_ as its parent, 
  //  assign its value as tool, set its name, and add it to the tool_layout_
  QWidget* tool_ = new QWidget(new_page_);
  tool_ = tool;
  tool_->setObjectName(QString::fromUtf8("tool_"));
  private_->ui_.tool_frame_layout_->addWidget( tool_ );
  
  // add the new_page_ to the tool_layout
  tool_layout_->addWidget(new_page_);
  
  //make all the proper connections
  connect(private_->ui_.help_button_, SIGNAL( clicked() ), this, SLOT(help_button_clicked()));
  QtBridge::connect(private_->ui_.activate_button_, activate_function);
  QtBridge::connect(private_->ui_.close_button_, close_function);

  set_active_tool(new_page_->findChild< QWidget* >("tool_"));
  
  private_->page_list_.push_back(new_page_);
  
}


void ToolBoxWidget::set_active_tool( QWidget *tool )
{
  for (size_t i = 0; i < private_->page_list_.size(); i++)
  {
    // first we deactivate the inactive tools
    if (private_->page_list_[i]->findChild< QWidget* >("tool_") != tool)
    {
      if (!private_->page_list_[i]->findChild< QFrame* >("tool_frame_")->isHidden())
      {
        private_->page_list_[i]->findChild< QWidget* >( "page_background_" )->setStyleSheet( 
               QString::fromUtf8(" QWidget#page_background_ { background-color: rgb(220, 220, 220); }"));
        private_->page_list_[i]->findChild< QPushButton* >( "activate_button_" )->setStyleSheet(
               QString::fromUtf8("QPushButton{\n"
                         "  margin-right: 7px;\n"
                         "  height: 24px;\n"
                         "  text-align: left;\n"
                         "  padding-left: 4px;\n"
                         "  color: rgb(25, 25, 25);\n"                           
                         "  font: normal;\n"
                         "}\n"));
        private_->page_list_[i]->findChild< QToolButton* >( "close_button_" )->setIcon( inactive_close_icon_ );
        private_->page_list_[i]->findChild< QToolButton* >( "help_button_" )->setIcon( inactive_help_icon_ );
        private_->page_list_[i]->findChild< QFrame* >("tool_frame_")->hide();
        
      }
    }
    
    // then, we activate the active one.
    else 
    {
      active_index_ = i;
      active_tool_ = private_->page_list_[i]->findChild< QWidget* >("tool_");
      
      if (private_->page_list_[i]->findChild< QFrame* >("tool_frame_")->isHidden())
      {
        private_->page_list_[i]->findChild< QWidget* >("page_background_")->setStyleSheet(
                       QString::fromUtf8("QWidget#page_background_ { background-color: rgb(255, 128, 0); }"));
        
        private_->page_list_[i]->findChild< QPushButton* >("activate_button_")->setStyleSheet(
                           QString::fromUtf8("QPushButton{\n"
                                   "  margin-right: 7px;\n"
                                   "  height: 24px;\n"
                                   "  text-align: left;\n"
                                   "  padding-left: 4px;\n"
                                   "  color: white;\n"
                                   "  font: bold;\n"
                                   "}\n"));  
        private_->page_list_[i]->findChild< QToolButton* >("close_button_")->setIcon(active_close_icon_);
        private_->page_list_[i]->findChild< QToolButton* >("help_button_")->setIcon(active_help_icon_);
        private_->page_list_[i]->findChild< QFrame* >("tool_frame_")->show();
      }
    }
  }
}
                    

int ToolBoxWidget::index_of( QWidget *tool ) 
{
  for (size_t i = 0; i < private_->page_list_.size(); i++)
  {
    if (private_->page_list_[i]->findChild< QWidget* >("tool_") == tool) 
    {
      return i;
    }
  }
  return -1;
}

QWidget* ToolBoxWidget::get_tool_at(int index)
{
  return private_->page_list_[index];
} // end get_tool_at



void ToolBoxWidget::set_active_index( int index )
{
  if ( ( index < static_cast<int>(private_->page_list_.size())) 
      && (index >= 0) )
  {
    set_active_tool( private_->page_list_[index]->findChild< QWidget* >("tool_") );
  }
  
} // end set_active_index


void ToolBoxWidget::remove_tool(int index)
{
  // Find the index that corresponds to the tool
  if (index >= static_cast<int>(private_->page_list_.size())) 
  {
    return;
  }

  tool_layout_->removeWidget(private_->page_list_[index]);
  private_->page_list_[index]->deleteLater();
  private_->page_list_.erase(private_->page_list_.begin()+index);
  
  // Set the previous tool to active if the one to be deleted is active.
  if (active_index_ == index) { set_active_index(index-1); }
  
}


void ToolBoxWidget:: help_button_clicked()
{
  QToolButton *help_button = ::qobject_cast<QToolButton*>(sender());
  
  for (size_t i = 0; i < private_->page_list_.size(); i++)
  {
    if( private_->page_list_[i]->findChild< QToolButton* >( "help_button_" ) == help_button )
    {
      QDesktopServices::openUrl(QUrl(private_->page_list_[i]->findChild< QLabel* >( "url_" )->text()));
      break;
    }
  }
}
                                                                    
} //end Seg3D namespace
