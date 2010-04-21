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

#include <Interface/AppInterface/DropSpaceWidget.h>

namespace Seg3D
{
  
  
DropSpaceWidget::DropSpaceWidget( QWidget *parent ) :
  changing_size_( false ),
  open_( false )
{
  this->setParent( parent );
  
  this->parent_ = parent;
  
  this->setObjectName( QString::fromUtf8( "insert_space_" ));
  this->setFixedHeight( 0 );  
  this->height_ = 0;
  this->setStyleSheet( QString::fromUtf8(
    "/*DropSpaceWidget#insert_space_{\n*/"
    " border-radius: 3px;\n"
    " margin-top: 0;\n"
    " margin-bottom: 0;\n"
    " padding-top: 0;\n"
    " padding-bottom: 0; \n"
    " border: 1px solid rgba(50, 50, 50, 40);\n"
    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(0, 0, 0, 50), stop:0.3 rgba(0, 0, 0, 100), stop:0.7 rg"
    " ba(0, 0, 0, 100), stop:1 rgba(0, 0, 0, 50));\n"
    "/*}\n*/" ) );
  
  
  
  this->timer_ = new QTimer( this );
  this->timer_->setSingleShot( true );
  connect( this->timer_, SIGNAL( timeout() ), this, SLOT( change_size() ) );

  this->setAcceptDrops( false );

  
}

DropSpaceWidget::~DropSpaceWidget()
{
  this->disconnect();
}

  void DropSpaceWidget::show()
  {
    this->open_ = true;
    if ( !this->timer_->isActive() ) {
      this->timer_->start(10);
    }
  }
  
  void DropSpaceWidget::hide()
  {
    this->open_ = false;
    if ( !this->timer_->isActive() ) {
      this->timer_->start(10);
    }
  }
  
  void DropSpaceWidget::change_size()
  {
    if ( this->open_ ) 
    {
      if ( this->isHidden() ) 
      {
        this->setVisible( true );
      }
      if ( this->height() < 45 ) {
        this->setFixedHeight( ( this->height() + 5 ) );
        this->updateGeometry();
        this->timer_->start(10);
      }
    }
    else 
    {
      if ( this->height() > 0 ) 
      {
        this->setFixedHeight( ( this->height() - 15 ) );
        this->updateGeometry();
        this->timer_->start(10);
      }
      else {
        this->setVisible( false );
      }
    }

  }

  
  
} // end namespace Seg3D