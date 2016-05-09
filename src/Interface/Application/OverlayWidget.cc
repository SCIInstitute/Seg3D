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

//Qt includes
#include <QtGui/QPainter>
#include <QtGui/QPen>

// Interface includes
#include <Interface/Application/OverlayWidget.h>

namespace Seg3D
{
  
class OverlayWidgetPrivate 
{
public:
  QColor color_;
  
};

OverlayWidget::OverlayWidget( QWidget *parent ) 
  : QWidget( parent ),
  private_( new OverlayWidgetPrivate )
{
  this->private_->color_ = QColor( 0, 0, 0, 0 );
  QSizePolicy size_policy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  this->setSizePolicy( size_policy );
}

OverlayWidget::~OverlayWidget()
{
}

void OverlayWidget::paintEvent( QPaintEvent * event )
{
  QPainter painter( this );
  painter.fillRect( this->rect(), this->private_->color_ );
}

void OverlayWidget::set_transparent( bool transparent )
{
  if( transparent )
  {
    this->private_->color_ = QColor( 0, 0, 0, 0 );
  }
  else
  {
    this->private_->color_ = QColor( 100, 100, 100, 150 );
  }
}

} // end namespace Seg3D
