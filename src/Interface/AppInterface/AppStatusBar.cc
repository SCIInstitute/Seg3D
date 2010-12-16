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

// boost includes
#include <boost/tokenizer.hpp>

// Qt includes
#include <QtCore/QPropertyAnimation>

// Core Includes - for logging
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtPointer.h>

// Interface includes
#include <Interface/AppInterface/AppStatusBar.h>
#include <Interface/AppInterface/StyleSheet.h>

//UI Includes
#include "ui_StatusBar.h"

//  Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>


namespace Seg3D
{

class AppStatusBarPrivate : public Core::RecursiveLockable
{
public:
  Ui::StatusBar ui_;
  boost::posix_time::ptime last_message_time_;
  QString current_message_;
  int message_type_;
  bool error_icon_set_;
  bool animating_;
  QIcon normal_message_icon_;
  QIcon error_message_icon_;
  
};

AppStatusBar::AppStatusBar( QMainWindow* parent ) :
  QObject( parent ), 
  show_world_coord_( false ), 
  private_( new AppStatusBarPrivate )
{
  this->statusbar_ = parent->statusBar();
  this->statusbar_widget_ = new QWidget( this->statusbar_ );
  
  this->private_->ui_.setupUi( this->statusbar_widget_ );
  this->private_->ui_.actives_->hide();

  this->statusbar_->setContentsMargins( 0, 0, 0, 0 );

  this->statusbar_->addWidget( this->statusbar_widget_, 1 );
  
  this->statusbar_->setStyleSheet( StyleSheet::STATUSBAR_C );
  
  connect( this->private_->ui_.swap_visibility_button_, 
    SIGNAL( clicked() ), this, SLOT( swap_bars() ) );
    
  QtUtils::QtBridge::Connect( this->private_->ui_.info_button_, 
    InterfaceManager::Instance()->message_window_visibility_state_ );
  
  connect( this->private_->ui_.info_button_, SIGNAL( toggled( bool ) ), this,
    SLOT( reset_icon( bool ) ) ); 
  
  connect( this->private_->ui_.world_button_, 
    SIGNAL( clicked( bool ) ), this, SLOT( set_coordinates_mode( bool ) ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.layer_combobox_, 
    LayerManager::Instance()->active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.tool_combobox_,
    ToolManager::Instance()->active_tool_state_ );

  this->add_connection( StatusBar::Instance()->data_point_info_updated_signal_.connect( 
    boost::bind( &AppStatusBar::update_data_point_info, this, _1 ) ) );
  
  this->add_connection( StatusBar::Instance()->message_updated_signal_.connect( 
    boost::bind( &AppStatusBar::SetMessage, qpointer_type( this ), _1, _2 ) ) );
  
  this->add_connection( Core::ActionDispatcher::Instance()->post_action_signal_.connect( 
    boost::bind( &AppStatusBar::HandleActionEvent, qpointer_type( this ) ) ) );
  
  this->private_->ui_.status_report_label_->setGeometry( QRect( 0, 
    -this->private_->ui_.status_report_label_->height(), 
    this->private_->ui_.status_report_label_->width(), 
    this->private_->ui_.status_report_label_->height() ) );
  
  this->private_->ui_.status_report_label_->hide();
  
  this->private_->last_message_time_ = boost::posix_time::second_clock::local_time();
  
  this->private_->normal_message_icon_.addFile( QString::fromUtf8( ":/Images/TextOff.png" ), QSize(), QIcon::Normal );
  this->private_->error_message_icon_.addFile( QString::fromUtf8( ":/Images/TextOffError.png" ), QSize(), QIcon::Normal );
  this->private_->error_icon_set_ = false;
  this->private_->animating_ = false;
  
}

AppStatusBar::~AppStatusBar()
{
  this->disconnect_all();
}
  
void AppStatusBar::reset_icon( bool show_errors )
{
  if( show_errors && this->private_->error_icon_set_ )
  {
    this->private_->ui_.info_button_->setIcon( this->private_->normal_message_icon_ );
    this->private_->error_icon_set_ = false;
  }
}


// -- private slots -- //
void AppStatusBar::swap_bars()
{
  if( this->private_->ui_.actives_->isVisible() )
  {
    this->private_->ui_.actives_->hide();
    this->private_->ui_.label_values_->show();
  }
  else
  {
    this->private_->ui_.actives_->show();
    this->private_->ui_.label_values_->hide();
  }
}

// -- public slots -- //
void AppStatusBar::set_coordinates_mode( bool is_world )
{
  if( !is_world )
  {
    this->private_->ui_.world_button_->setToolTip( QString::fromUtf8(
        "Coordinate mode is set to Local, click to toggle to Global" ) );
  }
  else
  {
    this->private_->ui_.world_button_->setToolTip( QString::fromUtf8(
        "Coordinate mode is set to Global, click to toggle to Local" ) );
  }

  this->show_world_coord_ = is_world;
  this->update_data_point_label();
}

void AppStatusBar::set_status_report_label( std::string& status )
{
  QString report = QString::fromStdString( status );
  this->private_->ui_.status_report_label_->setText( QString::fromUtf8( "Status: " ) + report );
}

void AppStatusBar::fix_icon_status()
{
  this->private_->ui_.status_report_label_->setText( 
    QString::fromUtf8( "Status = true " ) );
}

void AppStatusBar::update_data_point_info( DataPointInfoHandle data_point )
{
  if( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( 
      &AppStatusBar::update_data_point_info, this, data_point ) );
    return;
  }

  this->data_point_info_ = *data_point;
  this->update_data_point_label();
}
  
void AppStatusBar::update_data_point_label()
{
  bool zero_based_slice_numbers = false;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    zero_based_slice_numbers = PreferencesManager::Instance()->
      zero_based_slice_numbers_state_->get();
  }
  
  
  // get some local copies of the data
  double world_x = this->data_point_info_.world_coord().x();
  double world_y = this->data_point_info_.world_coord().y();
  double world_z = this->data_point_info_.world_coord().z();
  
  double index_x = this->data_point_info_.index_coord().x();
  double index_y = this->data_point_info_.index_coord().y();
  double index_z = this->data_point_info_.index_coord().z();
  
  if ( !zero_based_slice_numbers )
  {
    // Correct to one based indexing
    index_x += 1.0;
    index_y += 1.0;
    index_z += 1.0;
  }
  
  // In the case that all the coordinates are 0 then show nice 0's.
  if( ( world_x == 0 ) && ( world_y == 0 ) && ( world_z == 0 ) && 
    ( this->data_point_info_.value() == 0 ) )
  { 
    this->private_->ui_.x_->setText( QString::fromUtf8("0.000") );
    this->private_->ui_.y_->setText( QString::fromUtf8("0.000") );
    this->private_->ui_.z_->setText( QString::fromUtf8("0.000") );
    this->private_->ui_.value_->setText( QString::fromUtf8("0.000") );
  }
  // In the case that the coordinates are outside of .0001-1000.00,
  // format them with scientific notation.
  else if( ( world_x > 1000 ) || ( world_x > 0.0 && world_x < 0.0001 ) ||
    ( world_y > 1000 ) || ( world_y > 0.0 && world_y < 0.0001 ) ||
    ( world_z > 1000 ) || ( world_z > 0.0 && world_z < 0.0001 ) )
  {
    if( this->show_world_coord_ )
    {
      this->private_->ui_.x_->setText( QString( "%1" ).arg( world_x, 0, 'e', 3 ) );
      this->private_->ui_.y_->setText( QString( "%1" ).arg( world_y, 0, 'e', 3 ) );
      this->private_->ui_.z_->setText( QString( "%1" ).arg( world_z, 0, 'e', 3 ) );
    }
    else
    {
      this->private_->ui_.x_->setText( QString( "%1" ).arg( index_x, 0, 'f', 0 ) );
      this->private_->ui_.y_->setText( QString( "%1" ).arg( index_y, 0, 'f', 0 ) );
      this->private_->ui_.z_->setText( QString( "%1" ).arg( index_z, 0, 'f', 0 ) );
    }
    this->private_->ui_.value_->setText( QString( "%1" ).arg( 
      this->data_point_info_.value(), 0, 'e', 3 ) );
  }
  // Otherwise format them normally.
  else
  { 
    if( this->show_world_coord_ )
    {
      this->private_->ui_.x_->setText( QString( "%1" ).arg( world_x, 0, 'f', 3 ) );
      this->private_->ui_.y_->setText( QString( "%1" ).arg( world_y, 0, 'f', 3 ) );
      this->private_->ui_.z_->setText( QString( "%1" ).arg( world_z, 0, 'f', 3 ) );
    }
    else
    {
      this->private_->ui_.x_->setText( QString( "%1" ).arg( index_x, 0, 'f', 0 ) );
      this->private_->ui_.y_->setText( QString( "%1" ).arg( index_y, 0, 'f', 0 ) );
      this->private_->ui_.z_->setText( QString( "%1" ).arg( index_z, 0, 'f', 0 ) );
    }
    this->private_->ui_.value_->setText( QString( "%1" ).arg( 
      this->data_point_info_.value(), 0, 'f', 3 ) );
  }
}

void AppStatusBar::set_message( int msg_type, std::string message )
{
  this->private_->message_type_ = msg_type;
  this->private_->current_message_ = QString::fromStdString( message );
  
  if( this->private_->message_type_ == Core::LogMessageType::ERROR_E )
  {
    this->private_->ui_.info_button_->setIcon( this->private_->error_message_icon_ );
    this->private_->error_icon_set_ = true;
  }
  
  if( this->private_->ui_.status_report_label_->text() == "" )
  {
    this->slide_in();
  }
  else
  {
    this->slide_out_then_in();
  }
  
  // Last thing we do is reset the last message time.
  this->private_->last_message_time_ = boost::posix_time::second_clock::local_time();
}

void AppStatusBar::SetMessage( qpointer_type qpointer, int msg_type, std::string message )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &AppStatusBar::SetMessage, qpointer,
      msg_type, message ) );
    return;
  }

  if ( !qpointer.isNull() && !QCoreApplication::closingDown() )
  {
    qpointer->set_message( msg_type, message );
  }
}
  
  
  void AppStatusBar::HandleActionEvent( qpointer_type qpointer )
  {
    Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
      boost::bind( &AppStatusBar::check_time, qpointer.data() ) ) );
  }
  
  void AppStatusBar::check_time()
  {
    boost::posix_time::ptime current_time = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration duration = current_time - this->private_->last_message_time_;
    
    double time_since_last_message = static_cast< double >
    ( duration.total_milliseconds() ) * 0.001;
    
    if( ( time_since_last_message > 10 ) && ( this->private_->ui_.status_report_label_->text() != "" ) )
    {
      this->slide_out();
    }
  }
  
  void AppStatusBar::slide_in()
  {
    
    if( this->private_->ui_.status_report_label_->isVisible() )
    {
      this->private_->ui_.status_report_label_->hide();
    }
    
    this->private_->ui_.status_report_label_->setGeometry( QRect( 0, 
       -this->private_->ui_.status_report_label_->height(), 
       this->private_->ui_.status_report_label_->width(), 
       this->private_->ui_.status_report_label_->height() ) );
    
    // Here we create the animation that will slide the message into the task bar
    QPropertyAnimation *animation = new QPropertyAnimation( this->private_->ui_.status_report_label_, "geometry" );
    
    // Here we change the color of the text if we are reporting an error.
    if( this->private_->message_type_ == Core::LogMessageType::ERROR_E )
    {
      this->private_->ui_.status_report_label_->setStyleSheet( StyleSheet::STATUSBAR_ERROR_C );
      animation->setDuration( 2000 );
      animation->setEasingCurve( QEasingCurve::OutBounce );
    }
    else
    {
      this->private_->ui_.status_report_label_->setStyleSheet( StyleSheet::STATUSBAR_C );
      animation->setDuration( 500 );
      animation->setEasingCurve( QEasingCurve::InOutQuad );
    }
    
    this->private_->ui_.status_report_label_->setText( this->private_->current_message_ );
    
    this->private_->ui_.status_report_label_->show();
    
    animation->setStartValue( QRect( 0, -this->private_->ui_.status_report_label_->height(), 
      this->private_->ui_.status_report_label_->width(), 
      this->private_->ui_.status_report_label_->height() ) );
    animation->setEndValue( QRect( 0, 0, this->private_->ui_.status_report_label_->width(), 
      this->private_->ui_.status_report_label_->height() ) );
    connect( animation, SIGNAL( finished() ), this, SLOT( set_finished_animating() ) );
    animation->start();
    this->private_->animating_ = true;
  }
  
  void AppStatusBar::slide_out_then_in()
  {
    if( this->private_->animating_ ) return;
    
    QPropertyAnimation *animation = new QPropertyAnimation( this->private_->ui_.status_report_label_, "geometry" );
    animation->setDuration( 1000 );
    animation->setStartValue( QRect( 0, 0, this->private_->ui_.status_report_label_->width(), 
      this->private_->ui_.status_report_label_->height() ) );
    animation->setEndValue( QRect( 0, this->private_->ui_.status_report_label_->height(), 
      this->private_->ui_.status_report_label_->width(), 
      this->private_->ui_.status_report_label_->height() ) );
    connect( animation, SIGNAL( finished() ), this, SLOT( slide_in() ) );
    animation->setEasingCurve( QEasingCurve::OutQuad );
    animation->start();
  }
  
  
  
  void AppStatusBar::slide_out()
  {
    if( this->private_->animating_ ) return;
    
    QPropertyAnimation *animation = new QPropertyAnimation( this->private_->ui_.status_report_label_, "geometry" );
    animation->setDuration( 1000 );
    animation->setStartValue( QRect( 0, 0, this->private_->ui_.status_report_label_->width(), 
      this->private_->ui_.status_report_label_->height() ) );
    animation->setEndValue( QRect( 0, this->private_->ui_.status_report_label_->height(), 
      this->private_->ui_.status_report_label_->width(), 
      this->private_->ui_.status_report_label_->height() ) );
    connect( animation, SIGNAL( finished() ), this, SLOT( clear_label() ) );
    animation->setEasingCurve( QEasingCurve::OutQuad );
    animation->start();
    this->private_->animating_ = true;
  }
  
  void AppStatusBar::clear_label()
  {
    this->private_->ui_.status_report_label_->setText( QString::fromUtf8( "" ) );
    this->private_->ui_.status_report_label_->hide();
    this->private_->current_message_ = QString::fromUtf8( "" );
    this->set_finished_animating();
  }
  
  void AppStatusBar::set_finished_animating()
  {
    this->private_->animating_ = false;
  }

} // end namespace Seg3D


