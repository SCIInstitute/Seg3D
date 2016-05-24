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
#include <QtCore/QPropertyAnimation>

// Interface includes
#include <Interface/Application/DropSpaceWidget.h>
#include <Interface/Application/StyleSheet.h>

namespace Seg3D
{
  
  
DropSpaceWidget::DropSpaceWidget( QWidget *parent, int height, int grow_speed, int shrink_speed ) :
  changing_size_( false ),
  open_( false ),
  max_height_( height ),
  grow_speed_( grow_speed ),
  shrink_speed_( shrink_speed )
{
  this->setParent( parent );
  this->setObjectName( QString::fromUtf8( "insert_space_" ));
  this->setFixedHeight( 0 );  
  QSizePolicy size_policy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  this->setSizePolicy( size_policy );
  this->setAcceptDrops( false );

}

DropSpaceWidget::~DropSpaceWidget()
{
  this->disconnect();
}

void DropSpaceWidget::show()
{
  this->open_ = true;
  this->change_size();
}

void DropSpaceWidget::hide()
{
  this->open_ = false;
  this->change_size();
}


void DropSpaceWidget::change_size()
{
  if( this->open_ ) 
  {
    if( this->isHidden() ) 
    {
      this->setVisible( true );
    }
    
    QPropertyAnimation *animation = new QPropertyAnimation( this, "minimumHeight" );
    animation->setDuration( 200 );
    animation->setEndValue( this->max_height_ );
    animation->setEasingCurve( QEasingCurve::OutQuad );
    animation->start();

  }
  else 
  {
    if( this->height() > 0 ) 
    {
      QPropertyAnimation *animation = new QPropertyAnimation( this, "minimumHeight" );
      animation->setDuration( 200 );
      animation->setEndValue( 0 );
      animation->setEasingCurve( QEasingCurve::InQuad );
      animation->start();
    }
    else {
      this->setVisible( false );
    }
  }
}

void DropSpaceWidget::instant_hide()
{
  this->setVisible( false );
}
  
} // end namespace Seg3D
