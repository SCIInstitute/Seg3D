/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

// Core includes
#include <Core/Viewer/Mouse.h>
#include <Core/State/StateEngine.h>

// QtUtils includes
#include <QtUtils/Widgets/QtCustomDockWidget.h>

namespace QtUtils
{
  
class QtCustomDockWidgetPrivate
{
public:
  QWidget* overlay_;

};
  
QtCustomDockWidget::QtCustomDockWidget( QWidget *parent ) :
  QDockWidget( parent ),
  private_( new QtCustomDockWidgetPrivate )
{
  this->dock_base_ = new QWidget( this );
  this->dock_base_->setObjectName( QString::fromUtf8( "dock_base_" ) );

  this->private_->overlay_ = new QWidget( this );
  this->private_->overlay_->setObjectName( QString::fromUtf8( "overlay_" ) );
  this->private_->overlay_->setStyleSheet( QString::fromUtf8( "QWidget#overlay_{"
    " background-color: rgba(211, 211, 211, 212);"
    "}" ) );
  this->private_->overlay_->hide();

#ifdef BUILD_STANDALONE_LIBRARY
  this->setFeatures(features() & ~DockWidgetFloatable);
#endif
}
  
QtCustomDockWidget::~QtCustomDockWidget()
{
}

void QtCustomDockWidget::showEvent( QShowEvent* event )
{
  QDockWidget::showEvent( event );
  Q_EMIT opened();
  this->raise();
}

void QtCustomDockWidget::closeEvent( QCloseEvent* event )
{
  Q_EMIT closed();
  event->accept();
}

void QtCustomDockWidget::resizeEvent( QResizeEvent *event )
{
  if( this->isFloating() && this->private_->overlay_->isVisible() )
  {
    this->private_->overlay_->resize( event->size() );
  }
  
}

void QtCustomDockWidget::set_enabled( bool enabled )
{
  if( !this->isFloating() ) return;
  
  if( enabled )
  {
    this->private_->overlay_->hide();
  }
  else
  {
    this->private_->overlay_->show();
    this->private_->overlay_->raise();
    this->private_->overlay_->setStyleSheet( QString::fromUtf8( "QWidget#overlay_{"
      " background-color: rgba( 211, 211, 211, 212 );"
      "}" ) );
    this->private_->overlay_->resize( this->size() );
    this->repaint();
  }
}


} // end namespace QtUtils
