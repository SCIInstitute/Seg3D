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

#include <QStatusBar>

//  Application includes
#include <Application/Interface/Interface.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>

// Interface includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/AppStatusBar.h>

#include <Utils/Core/Log.h>

// boost includes
#include<boost/tokenizer.hpp>

namespace Seg3D
{

AppStatusBar::AppStatusBar( QMainWindow* parent ) :
  QObject( parent ), show_world_coord_( false )
{

  { // setup the icons
    world_icon_.addFile( QString::fromUtf8( ":/Images/WorldOff.png" ), QSize(), QIcon::Normal,
        QIcon::Off );
    world_icon_.addFile( QString::fromUtf8( ":/Images/World.png" ), QSize(), QIcon::Normal,
        QIcon::On );
    text_icon_.addFile( QString::fromUtf8( ":/Images/TextOff.png" ), QSize(), QIcon::Normal,
        QIcon::Off );
    text_icon_.addFile( QString::fromUtf8( ":/Images/Text.png" ), QSize(), QIcon::Normal,
        QIcon::On );
  }

  QStatusBar* statusbar = parent->statusBar();
  this->history_widget_ = new MessageHistoryWidget(parent);

  build_coordinates_label();
  build_status_report_label();
  build_buttons();

  statusbar->setContentsMargins( 4, 0, 0, 0 );
  statusbar->addWidget( info_button_, 0 );
  statusbar->addWidget( status_report_label_, 5 );
  statusbar->addWidget( coordinates_label_, 1 );
  statusbar->addWidget( world_button_, 0 );
  statusbar->setStyleSheet(
      QString::fromUtf8(
          "QStatusBar::item { \n"
            " border: none; }"
            "QStatusBar QToolButton { \n"
            " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
            " border: 1px solid  qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
            " \n}" ) );

  connect(this->info_button_, SIGNAL(clicked(bool)), this, SLOT(activate_history(bool)));
  connect(this->history_widget_, SIGNAL( destroyed() ), this, SLOT( fix_icon_status() ) );
  connect(this->world_button_, SIGNAL(clicked(bool)), this, SLOT(set_coordinates_mode(bool)));

  this->update_data_point_label();

  this->add_connection( StatusBar::Instance()->data_point_info_updated_signal_.connect( 
    boost::bind( &AppStatusBar::update_data_point_info, this, _1 ) ) );
  this->add_connection( StatusBar::Instance()->message_updated_signal_.connect( 
    boost::bind( &AppStatusBar::set_message, this, _1, _2 ) ) );

}

AppStatusBar::~AppStatusBar()
{
  this->disconnect_all();
  this->history_widget_->close();
}

// -- build status bar widgets -- //

void AppStatusBar::build_buttons()
{
  this->world_button_ = new QToolButton;
  this->world_button_->setCheckable( true );
  this->world_button_->setIcon( this->world_icon_ );
  this->world_button_->setIconSize( QSize( 16, 16 ) );
  this->world_button_->setContentsMargins( 0, 0, 0, 0 );

  this->info_button_ = new QToolButton;
  this->info_button_->setCheckable( true );
  this->info_button_->setIcon( this->text_icon_ );
  this->info_button_->setIconSize( QSize( 16, 16 ) );
  this->info_button_->setContentsMargins( 0, 0, 0, 0 );

}

void AppStatusBar::build_coordinates_label()
{
  this->coordinates_label_ = new QLabel( "x: 0000  y: 0000 z: 0000 value: 0000" );
  this->coordinates_label_->setObjectName( QString::fromUtf8( "coordinates_label_" ) );
  this->coordinates_label_->setContentsMargins( 0, 0, 0, 0 );
  this->coordinates_label_->setLayoutDirection( Qt::LeftToRight );
  this->coordinates_label_->setStyleSheet(
      QString::fromUtf8(
          "QStatusBar QLabel#coordinates_label_{ \n"
            " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
            " border: 1px solid  qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
            "  color: black; \n"
            "  text-align: center; \n}" ) );
}

void AppStatusBar::build_status_report_label()
{
  this->status_report_label_ = new QLabel();
  this->status_report_label_->setObjectName( QString::fromUtf8( "status_report_label_" ) );
  this->status_report_label_->setContentsMargins( 0, 0, 0, 0 );
  this->status_report_label_->setStyleSheet( QString::fromUtf8( "QLabel#status_report_label_{\n"
    " text-align: left;\n"
    " font: bold; }\n" ) );

}

// -- public slots -- //

void AppStatusBar::set_coordinates_mode( bool is_world )
{
  if ( !is_world )
  {
    this->coordinates_label_->setToolTip( QString::fromUtf8(
        "Coordinate mode is set to Local, click to toggle to Global" ) );
  }
  else
  {
    this->coordinates_label_->setToolTip( QString::fromUtf8(
        "Coordinate mode is set to Global, click to toggle to Local" ) );
  }

  this->show_world_coord_ = is_world;
  this->update_data_point_label();
}

void AppStatusBar::set_status_report_label( std::string& status )
{
  QString report = QString::fromStdString( status );
  this->status_report_label_->setText( QString::fromUtf8( "Status: " ) + report );

}

void AppStatusBar::activate_history( bool is_active_ )
{
  if ( is_active_ )
  {
    this->history_widget_->show();
  }
  else
  {
    this->history_widget_->hide();
  }

}
  
void AppStatusBar::fix_icon_status()
{
  this->status_report_label_->setText( QString::fromUtf8( "Status = true " ) );
}

void AppStatusBar::update_data_point_info( DataPointInfoHandle data_point )
{
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &AppStatusBar::update_data_point_info, this, data_point ) );
    return;
  }

  this->data_point_info_ = *data_point;
  this->update_data_point_label();
}

void AppStatusBar::update_data_point_label()
{
  QString x_str, y_str, z_str, value_str;
  QTextStream text_stream( &x_str );
  text_stream.setFieldWidth( 20 );
  text_stream.setRealNumberPrecision( 4 );
  text_stream.setFieldAlignment( QTextStream::AlignLeft );
  text_stream.setRealNumberNotation( QTextStream::FixedNotation );

  if ( this->show_world_coord_ )
  {
    text_stream << this->data_point_info_.world_coord().x();
    text_stream.setString( &y_str );
    text_stream.setFieldWidth( 20 );
    text_stream.setFieldAlignment( QTextStream::AlignLeft );
    text_stream << this->data_point_info_.world_coord().y();
    text_stream.setString( &z_str );
    text_stream.setFieldWidth( 20 );
    text_stream.setFieldAlignment( QTextStream::AlignLeft );
    text_stream << this->data_point_info_.world_coord().z();
  }
  else
  {
    text_stream << static_cast< int >( this->data_point_info_.index_coord().x() );
    text_stream.setString( &y_str );
    text_stream.setFieldWidth( 20 );
    text_stream.setFieldAlignment( QTextStream::AlignLeft );
    text_stream << static_cast< int >( this->data_point_info_.index_coord().y() );
    text_stream.setString( &z_str );
    text_stream.setFieldWidth( 20 );
    text_stream.setFieldAlignment( QTextStream::AlignLeft );
    text_stream << static_cast< int >( this->data_point_info_.index_coord().z() );
  }

  text_stream.setString( &value_str );
  text_stream.setRealNumberNotation( QTextStream::SmartNotation );
  text_stream.setFieldWidth( 20 );
  text_stream.setFieldAlignment( QTextStream::AlignLeft );
  text_stream << this->data_point_info_.value();

  this->coordinates_label_->setText( QString( "X: %1 Y: %2 Z: %3 Value: %4\t" )
    .arg( x_str, -25 ).arg( y_str, -25 ).arg( z_str, -25 ).arg( value_str, -25 ) );
}

void AppStatusBar::set_message( int msg_type, std::string message )
{
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &AppStatusBar::set_message, this,
        msg_type, message ) );
    return;
  }

  QColor color_ = QColor(255, 255, 255);
  std::string status_message = message;
  boost::char_separator< char > separater( " " );
  boost::tokenizer< boost::char_separator< char > > tok( message, separater );

  for ( boost::tokenizer< boost::char_separator< char > >::iterator beg = ++tok.begin(); beg
      != tok.end(); ++beg )
  {
    status_message = status_message + " " + *beg;
  }

  switch( msg_type )
  {
  case Utils::LogMessageType::ERROR_E:
    this->status_report_label_->setStyleSheet( QString::fromUtf8(
        "QLabel#status_report_label_{\n"
          " text-align: left;\n"
          " color: rgb(121, 0, 0);\n"
          " font: bold; }\n" ) );
    color_ = QColor(121, 0, 0);
    break;

  case Utils::LogMessageType::WARNING_E:
    this->status_report_label_->setStyleSheet( QString::fromUtf8(
        "QLabel#status_report_label_{\n"
          " text-align: left;\n"
          " color: rgb(165, 161, 34);\n"
          " font: bold; }\n" ) );
    color_ = QColor(165, 161, 34);
    break;
  case Utils::LogMessageType::MESSAGE_E:
    this->status_report_label_->setStyleSheet( QString::fromUtf8(
        "QLabel#status_report_label_{\n"
          " text-align: left;\n"
          " color: rgb(3, 86, 2);\n"
          " font: bold; \n}" ) );
    color_ = QColor(3, 86, 2);
    break;

  case Utils::LogMessageType::DEBUG_E:
    this->status_report_label_->setStyleSheet( QString::fromUtf8(
        "QLabel#status_report_label_{\n"
          " text-align: left;\n"
          " color: purple;\n"
          " font: bold; }\n" ) );
    color_ = QColor("purple");
    break;
  default:
    break;
  }

  this->status_report_label_->setText( QString::fromStdString( message ) );
  this->history_widget_->add_history_item( QString::fromStdString( message ), color_ );
}

} // end namespace Seg3D
