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

#include <QtUtils/Widgets/QtColorButton.h>

namespace QtUtils
{
  
QtColorButton::QtColorButton( QWidget *parent, int index, Core::Color button_color,
  int height, int width ) :
  QToolButton( parent ),
  index_( index )
{
  this->set_color( button_color );
  this->setCheckable( true );

  if( height )
  {
    this->setMinimumHeight( height );
    this->setMaximumHeight( height );
  }
  if( width )
  {
    this->setMinimumWidth( width );
    this->setMaximumWidth( width );
  }
  
  if( !height && !width )
  {
    QSizePolicy sizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    sizePolicy.setHorizontalStretch( 0 );
    sizePolicy.setVerticalStretch( 0 );
    sizePolicy.setHeightForWidth( this->sizePolicy().hasHeightForWidth() );
    this->setSizePolicy( sizePolicy );
  }

  //QIcon button_icon;
  //button_icon.addFile( QString::fromUtf8(":/Palette_Icons/color_transparent.png"),
  //                     QSize(), QIcon::Normal );
  //this->setIcon(button_icon);

  connect( this, SIGNAL( toggled ( bool ) ), this, SLOT( trigger_signal( bool ) ) );
}
  
QtColorButton::~QtColorButton()
{
}

void QtColorButton::set_color( Core::Color button_color )
{
  this->button_color_ = button_color;
  
  QString style_sheet = QString(
	  ":enabled { background-color: rgb(%1, %2, %3); "
	  "border-radius: 3px; "
	  " border: 1px none rgba(106, 106, 112, 255); }"
	  ":disabled { background-color: rgba(%1,%2,%3,127); "
	  "border-radius: 3px; "
	  " border: 1px none rgba(106, 106, 112, 255); }"
	  ).arg(button_color_.r()).arg(button_color_.g()).arg(button_color_.b());
      
  this->setStyleSheet( style_sheet );
  
  std::string style = style_sheet.toStdString();
  
  Q_EMIT this->color_changed( this->button_color_ );
  Q_EMIT this->color_changed( this->index_ );
}

void QtColorButton::trigger_signal( bool torf )
{
  Q_EMIT this->button_clicked( this->button_color_, this->isChecked() );
  Q_EMIT this->index( this->index_ );
  Q_EMIT this->color_changed( this->index_ );
}
  
} // end namespace QtUtils
