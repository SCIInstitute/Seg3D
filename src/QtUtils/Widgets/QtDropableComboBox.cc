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

// Qt includes
#include <QPainter>
#include <QMimeData>

// QtUtils includes
#include <QtUtils/Widgets/QtDropableComboBox.h>

namespace QtUtils
{
  
QtDropableComboBox::QtDropableComboBox( QWidget *parent ) :
  QComboBox( parent )
{
  this->setAcceptDrops( true );
  this->setSizeAdjustPolicy( QComboBox::AdjustToContents );
}
  
QtDropableComboBox::~QtDropableComboBox()
{
}

void QtDropableComboBox::dropEvent( QDropEvent* event )
{
  int index = this->findText( event->mimeData()->text() );
  
  if( index != -1 )
  { 
    this->setCurrentIndex( index );
    event->accept();
  }

  event->ignore();
  return;
}

void QtDropableComboBox::dragEnterEvent( QDragEnterEvent* event )
{
  if( this->isEnabled() && (  this->findText( event->mimeData()->text() ) != -1 ) )
  { 
    this->setFocus();
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void QtDropableComboBox::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->clearFocus();
}

void QtDropableComboBox::add_color_icon( Core::Color button_color, int index )
{
  QPixmap pixmap( 20, 20 );
  QPainter painter( &pixmap );
  painter.setPen( Qt::NoPen );
  painter.fillRect( QRect( 0, 0, 20, 20 ), 
    QColor( button_color.r(), button_color.g(), button_color.b() ) );
  this->setItemIcon( index, QIcon( pixmap ) );
}
  
} // end namespace QtUtils
