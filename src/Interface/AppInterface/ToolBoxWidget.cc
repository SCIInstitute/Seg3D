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

// Interface includes
#include <Interface/QtInterface/QtBridge.h>

// Application includes
#include <Interface/AppInterface/StyleSheet.h>

// Qt includes
#include <QUrl>
#include <QDesktopServices>
#include "ui_ToolBoxPageWidget.h"

namespace Seg3D
{

class ToolBoxPageWidget
{
public:
  QWidget* page_;
  QWidget* tool_;
  Ui::ToolBoxPageWidget ui_;
};

typedef std::vector<ToolBoxPageWidget> ToolBoxPageWidgetVector;

class ToolBoxWidgetPrivate
{
public:
  ToolBoxPageWidgetVector page_list_;
};


ToolBoxWidget::ToolBoxWidget( QWidget* parent ) :
  QScrollArea( parent )
{

  { // Prepare the icons!!
    this->active_close_icon_.addFile( QString::fromUtf8( ":/Images/CloseWhite.png" ), QSize(),
        QIcon::Normal, QIcon::Off );
    this->inactive_close_icon_.addFile( QString::fromUtf8( ":/Images/Close.png" ), QSize(),
        QIcon::Normal, QIcon::Off );

    this->active_help_icon_.addFile( QString::fromUtf8( ":/Images/HelpWhite.png" ), QSize(),
        QIcon::Normal, QIcon::Off );
    this->inactive_help_icon_.addFile( QString::fromUtf8( ":/Images/Help.png" ), QSize(),
        QIcon::Normal, QIcon::Off );
  }

  this->private_ = ToolBoxWidgetPrivateHandle( new ToolBoxWidgetPrivate );

  this->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  this->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  this->setContentsMargins( 1, 1, 1, 1 );
  this->setWidgetResizable( true );

  this->main_ = new QWidget( this );
  setWidget( this->main_ );

  this->main_layout_ = new QVBoxLayout( this->main_ );
  this->main_layout_->setContentsMargins( 1, 1, 1, 1 );
  this->main_layout_->setSpacing( 1 );

  this->tool_layout_ = new QVBoxLayout;
  this->main_layout_->addLayout( this->tool_layout_ );
  this->main_layout_->addStretch();

  this->main_->setLayout( this->main_layout_ );
  this->main_->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );

}

ToolBoxWidget::~ToolBoxWidget()
{

}

void ToolBoxWidget::add_tool( QWidget * tool, const QString &label,
    boost::function< void() > close_function, boost::function< void() > activate_function,
    const std::string& help_url )
{
  if ( !tool ) return;

  ToolBoxPageWidget new_page;

  new_page.page_ = new QWidget();
  new_page.tool_ = tool;
  new_page.ui_.setupUi( new_page.page_ );

  new_page.ui_.url_->setText( QString::fromStdString( help_url ) );
  new_page.ui_.url_->hide();

  new_page.ui_.activate_button_->setText( label );

  new_page.ui_.help_button_->setIcon( active_help_icon_ );
  new_page.ui_.help_button_->setIconSize( QSize( 18, 18 ) );

  new_page.ui_.close_button_->setIcon( active_close_icon_ );
  new_page.ui_.close_button_->setIconSize( QSize( 18, 18 ) );

  new_page.ui_.tool_frame_layout_->addWidget( new_page.tool_ );

  // add the new_page_ to the tool_layout
  this->tool_layout_->addWidget( new_page.page_ );

  //make all the proper connections
  connect( new_page.ui_.help_button_, SIGNAL( clicked() ), this, 
    SLOT( help_button_clicked() ) );
  QtBridge::Connect( new_page.ui_.activate_button_, activate_function );
  QtBridge::Connect( new_page.ui_.close_button_, close_function );

  set_active_tool( tool );

  this->private_->page_list_.push_back( new_page );
}

void ToolBoxWidget::set_active_tool( QWidget *tool )
{
  this->main_->setUpdatesEnabled( false );
  
  for ( size_t i = 0; i < this->private_->page_list_.size(); i++ )
  {
    // first we deactivate the inactive tools
    if ( this->private_->page_list_[ i ].tool_ != tool )
    {
      if ( !( this->private_->page_list_[ i ].ui_.tool_frame_->isHidden() ) )
      {
        this->private_->page_list_[ i ].ui_.page_background_->setStyleSheet( 
          StyleSheet::TOOLBOXPAGEWIDGET_PAGE_BACKGROUND_INACTIVE_C );
        this->private_->page_list_[ i ].ui_.activate_button_->setStyleSheet(
            StyleSheet::TOOLBOXPAGEWIDGET_ACTIVATE_BUTTON_INACTIVE_C );
        this->private_->page_list_[ i ].ui_.close_button_->setIcon(
            inactive_close_icon_ );
        this->private_->page_list_[ i ].ui_.help_button_->setIcon(
            inactive_help_icon_ );
        this->private_->page_list_[ i ].ui_.tool_frame_->hide();
      }
    }
  }
  
    // then, we activate the active one.
  for ( size_t i = 0; i < this->private_->page_list_.size(); i++ )
  {
    if ( this->private_->page_list_[ i ].tool_ == tool )
    {
      this->active_index_ = static_cast< int > ( i );
      this->active_tool_ = private_->page_list_[ i ].tool_;

      if ( this->private_->page_list_[ i ].ui_.tool_frame_->isHidden() )
      {
        this->private_->page_list_[ i ].ui_.page_background_->setStyleSheet(
            StyleSheet::TOOLBOXPAGEWIDGET_PAGE_BACKGROUND_ACTIVE_C );
        this->private_->page_list_[ i ].ui_.activate_button_->setStyleSheet(
            StyleSheet::TOOLBOXPAGEWIDGET_ACTIVATE_BUTTON_ACTIVE_C );
        this->private_->page_list_[ i ].ui_.close_button_->setIcon(
            active_close_icon_ );
        this->private_->page_list_[ i ].ui_.help_button_->setIcon(
            active_help_icon_ );
        this->private_->page_list_[ i ].ui_.tool_frame_->show();
      }
      
    }
  }
  
  this->main_->setUpdatesEnabled( true );
}

int ToolBoxWidget::index_of( QWidget *tool )
{
  for ( size_t i = 0; i < this->private_->page_list_.size(); i++ )
  {
    if ( this->private_->page_list_[ i ].tool_ == tool )
    {
      return static_cast< int > ( i );
    }
  }
  return -1;
}

QWidget* ToolBoxWidget::get_tool_at( int index )
{
  return this->private_->page_list_[ index ].page_;
} // end get_tool_at


void ToolBoxWidget::set_active_index( int index )
{
  if ( ( index < static_cast< int > ( this->private_->page_list_.size() ) ) && ( index >= 0 ) ) set_active_tool(
      this->private_->page_list_[ index ].tool_ );

} // end set_active_index


void ToolBoxWidget::remove_tool( int index )
{
  // Find the index that corresponds to the tool
  if ( index >=  static_cast< int > (this->private_->page_list_.size()) )
  {
    return;
  }

  this->tool_layout_->removeWidget( private_->page_list_[ index ].page_ );
  this->private_->page_list_[ index ].page_->deleteLater();
  this->private_->page_list_.erase( private_->page_list_.begin() + index );

  // Set the previous tool to active if the one to be deleted is active.
  if ( this->active_index_ == index )
  {
    if ( index == 0 )
    {
      set_active_index( index );
    }
    else
    {
      set_active_index( index - 1 );
    }
  }

}

void ToolBoxWidget::help_button_clicked()
{
  QToolButton *help_button = ::qobject_cast< QToolButton* >( sender() );

  for ( size_t i = 0; i < this->private_->page_list_.size(); i++ )
  {
    if ( this->private_->page_list_[ i ].ui_.help_button_ == help_button )
    {
      QDesktopServices::openUrl( QUrl( this->private_->page_list_[ i ].ui_.url_->text() ) );
      break;
    }
  }
}

} // end namespace Seg3D
