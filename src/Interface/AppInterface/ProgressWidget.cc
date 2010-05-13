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

// Qt includes
#include <QTimer>

// Interface includes
#include <Interface/AppInterface/ProgressWidget.h>

//UI Includes
#include "ui_ProgressWidget.h"

namespace Seg3D
{

class ProgressWidgetPrivate {
public:
  Ui::ProgressWidget ui_;
  
  QPixmap running_pixmap_[18];
  int running_count_;
  QTimer* running_timer_;
};

ProgressWidget::ProgressWidget( Core::ActionProgressHandle action_progress, QWidget *parent ) :
  QDialog( parent ),
  action_progress_( action_progress ),
  private_( new ProgressWidgetPrivate )
{
  // Step (1): Prevent any import from other widgets while progress bar is shown
  setModal( true );

  // Step (2): Add the Ui children onto the QWidget
  this->private_->ui_.setupUi( this );
  
  // Step (3): Setup pixmaps for animation
  for (size_t j = 0; j < 18; j++)
  {
    std::string pixmap_name = std::string(":/Images/Circle") + Core::ExportToString( 20 * j ) + 
      std::string( ".png" );
    this->private_->running_pixmap_[ j ].load( QString::fromStdString( pixmap_name ) ); 
  }
  
  this->private_->running_count_ = 0;
  
  // Step (4): Insert message
  this->private_->ui_.message_->setText( QString::fromStdString( action_progress_->get_message() ) );

  // Step (5): Setup cancel button
  if( !( this->action_progress_->is_interruptable() ) )
  {
    this->private_->ui_.cancel_button_->hide();
    this->private_->ui_.running_->setPixmap( this->private_->running_pixmap_[ 0 ] );
  }

  // Step (6): Setup progress bar or waiting animation
  if( !( this->action_progress_->has_progress_updates() ) )
  {
    this->private_->ui_.progress_bar_->hide();
    this->private_->ui_.line_->hide();

    this->private_->running_timer_ = new QTimer( this );
    this->private_->running_timer_->setInterval( 40 );
    
    connect( this->private_->running_timer_, SIGNAL( timeout() ), 
      this, SLOT( update_running() ) );
  
    this->private_->running_timer_->start();
  }
  else
  {
    this->private_->ui_.running_->hide();
    this->private_->ui_.progress_bar_->setMinimum( 0 );
    this->private_->ui_.progress_bar_->setMaximum( 100 );
  }

  // Step (7): Connect cancel button
  connect( this->private_->ui_.cancel_button_, SIGNAL( clicked() ), this, SLOT( interrupt() ) );
}

ProgressWidget::~ProgressWidget()
{
}

void ProgressWidget::update_running()
{
  // Loop through the animation
  this->private_->running_count_++;
  if( this->private_->running_count_ > 17 ) this->private_->running_count_ = 0;
  this->private_->ui_.running_->setPixmap( 
    this->private_->running_pixmap_[ this->private_->running_count_ ] );
}

void ProgressWidget::update_progress( )
{
  // Update the progress bar
  this->private_->ui_.progress_bar_->setValue( static_cast<int>( 
    this->action_progress_->get_progress()*100.0 ) );
}

void ProgressWidget::interrupt()
{
  this->action_progress_->set_interrupt( true );
}


} // end namespace Seg3D
