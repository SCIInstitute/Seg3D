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

// Interface includes
#include <Interface/AppInterface/DropSpaceWidget.h>
#include <Interface/AppInterface/StyleSheet.h>

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

//  this->timer_ = new QTimer( this );
//  this->timer_->setSingleShot( true );
//  connect( this->timer_, SIGNAL( timeout() ), this, SLOT( change_size() ) );

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
    
    //#if defined( _WIN32 )
    //  if( !this->timer_->isActive() ) 
    //  {
    //    this->timer_->start(10);
    //  }
    //#else
    //  this->setFixedHeight( this->max_height_ );
    //  this->updateGeometry();
    //  this->setVisible( true );
    //#endif
  }
  
  void DropSpaceWidget::hide()
  {
    this->open_ = false;

    this->change_size();
    
    //#if defined( _WIN32 )
    //  if( !this->timer_->isActive() ) 
    //  {
    //    this->timer_->start(10);
    //  }
    //#else
    //  this->setFixedHeight( 0 );
    //  this->updateGeometry();
    //  this->setVisible( false );  
    //#endif
  }
  
//  void DropSpaceWidget::change_size()
//  {
//    if( this->open_ ) 
//    {
//      if( this->isHidden() ) 
//      {
//        this->setVisible( true );
//      }
//      if( this->height() < this->max_height_ ) {
//        this->setFixedHeight( ( this->height() + this->grow_speed_ ) );
//        this->updateGeometry();
//        this->timer_->start(10);
//      }
//    }
//    else 
//    {
//      if( this->height() > 0 ) 
//      {
//        this->setFixedHeight( ( this->height() - this->shrink_speed_ ) );
//        this->updateGeometry();
//        this->timer_->start(10);
//      }
//      else {
//        this->setVisible( false );
//      }
//    }
//  }


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
    animation->setStartValue( this->height() );
    animation->setEndValue( this->max_height_ );
    animation->start();

  }
  else 
  {
    if( this->height() > 0 ) 
    {
      QPropertyAnimation *animation = new QPropertyAnimation( this, "minimumHeight" );
      animation->setDuration( 200 );
      animation->setStartValue( this->height() );
      animation->setEndValue( 0 );
      animation->start();
    }
    else {
      this->setVisible( false );
    }
  }
}

  
} // end namespace Seg3D