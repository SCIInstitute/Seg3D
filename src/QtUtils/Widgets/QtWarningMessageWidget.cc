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

// QtUtils includes
#include <QtUtils/Widgets/QtWarningMessageWidget.h>

// UI includes
#include "ui_QtWarningMessageWidget.h"


namespace QtUtils
{
  
class QtWarningMessageWidgetPrivate 
{
public:
  Ui::QtWarningMessageWidget ui_;
  bool is_hidden_;
  QRect rect_;
  QString message_;
};
  
  
QtWarningMessageWidget::QtWarningMessageWidget( QWidget *parent ) :
  QWidget( parent ),
  private_( new QtWarningMessageWidgetPrivate )
{
  this->private_->ui_.setupUi( this );
  this->private_->is_hidden_ = false;
}
  
QtWarningMessageWidget::~QtWarningMessageWidget()
{
}
  
void QtWarningMessageWidget::set_warning_message( const std::string& warning )
{
  this->private_->message_ = QString::fromStdString( warning );
  this->private_->rect_ = this->rect();
  this->setGeometry( this->private_->rect_.x(), this->private_->rect_.y(), this->private_->rect_.width(), 0 );
  this->private_->is_hidden_ = true;
}
  
void QtWarningMessageWidget::setVisible( bool visible )
{
  if( visible )
  {
    this->show();
  }
  else 
  {
    this->hide();
  }
}
  
  
void QtWarningMessageWidget::hide()
{
//  if( this->private_->is_hidden_ ) return;
  
  QPropertyAnimation *animation = new QPropertyAnimation( this, "geometry" );
  animation->setDuration( 500 );
  animation->setEndValue( QRect( this->private_->rect_.x(), this->private_->rect_.y(),
    this->private_->rect_.width(), 0 ) );
  connect( animation, SIGNAL( finished() ), this, SLOT( set_hidden() ) );
  animation->start();
}
  
void QtWarningMessageWidget::show()
{
//  if( !this->private_->is_hidden_ ) return;
  
  QPropertyAnimation *animation = new QPropertyAnimation( this, "geometry" );
  animation->setDuration( 500 );
  animation->setEndValue( this->private_->rect_ );
  connect( animation, SIGNAL( finished() ), this, SLOT( set_shown() ) );
  animation->start();
}
  
void QtWarningMessageWidget::set_hidden()
{
  this->private_->ui_.message_->setText( QString::fromStdString( "" ) );
//  this->private_->is_hidden_ = true;
}
                             
void QtWarningMessageWidget::set_shown()
{
  this->private_->ui_.message_->setText( this->private_->message_ );
//  this->private_->is_hidden_ = false;
}



} // end namespace QtUtils
