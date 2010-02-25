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
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>

// Interface includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/AppStatusBar.h>
#include <Application/Interface/Interface.h>

// boost includes
#include<boost/tokenizer.hpp>

namespace Seg3D
{

AppStatusBar::AppStatusBar( QMainWindow* parent ) :
  QObject( parent )
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
  history_widget_ = new HistoryWidget();

  build_coordinates_label();
  build_status_report_label();
  build_buttons();

  qpointer_type statusbar_pointer( this );

  statusbar->setContentsMargins( 4, 0, 0, 0 );
  statusbar->addWidget( status_report_label_, 5 );
  statusbar->addWidget( info_button_, 0 );
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

  Seg3D::Interface::Instance()->interface_action_context()->action_message_signal_.connect(
      boost::bind( &AppStatusBar::UpdateStatusBar, statusbar_pointer, _1, _2 ) );
  connect(info_button_, SIGNAL(clicked(bool)), this, SLOT(activate_history(bool)));
  connect( history_widget_, SIGNAL( destroyed() ), this, SLOT( fix_icon_status() ) );
  connect(world_button_, SIGNAL(clicked(bool)), this, SLOT(set_coordinates_mode(bool)));

  set_coordinates_mode( 1 );
  set_coordinates_label( 234, 232, 100 );

}

AppStatusBar::~AppStatusBar()
{
}

// -- build status bar widgets -- //

void AppStatusBar::build_buttons()
{
  world_button_ = new QToolButton;
  world_button_->setCheckable( true );
  world_button_->setIcon( world_icon_ );
  world_button_->setIconSize( QSize( 16, 16 ) );
  world_button_->setContentsMargins( 0, 0, 0, 0 );

  info_button_ = new QToolButton;
  info_button_->setCheckable( true );
  info_button_->setIcon( text_icon_ );
  info_button_->setIconSize( QSize( 16, 16 ) );
  info_button_->setContentsMargins( 0, 0, 0, 0 );

}

void AppStatusBar::build_coordinates_label()
{
  coordinates_label_ = new QLabel( "x: 0000  y: 0000 z: 0000" );
  coordinates_label_->setObjectName( QString::fromUtf8( "coordinates_label_" ) );
  coordinates_label_->setContentsMargins( 0, 0, 0, 0 );
  coordinates_label_->setStyleSheet(
      QString::fromUtf8(
          "QStatusBar QLabel#coordinates_label_{ \n"
            " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
            " border: 1px solid  qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
            "  color: black; \n"
            "  text-align: center; \n}" ) );
}

void AppStatusBar::build_status_report_label()
{
  status_report_label_ = new QLabel();
  status_report_label_->setObjectName( QString::fromUtf8( "status_report_label_" ) );
  status_report_label_->setContentsMargins( 0, 0, 0, 0 );
  status_report_label_->setStyleSheet( QString::fromUtf8( "QLabel#status_report_label_{\n"
    " text-align: left;\n"
    " font: bold; }\n" ) );

}

// -- public slots -- //

void AppStatusBar::set_coordinates_label( int x_coord_, int y_coord_, int z_coord_ )
{
  QString mouseLocation = QString().sprintf( "x: %04d  y: %04d z: %04d\t", x_coord_, y_coord_,
      z_coord_ );
  coordinates_label_->setText( mouseLocation );
}

void AppStatusBar::set_coordinates_mode( bool is_local_ )
{
  if ( is_local_ )
  {
    coordinates_label_->setToolTip( QString::fromUtf8(
        "Coordinate mode is set to Local, click to toggle to Global" ) );
  }
  else
  {
    coordinates_label_->setToolTip( QString::fromUtf8(
        "Coordinate mode is set to Global, click to toggle to Local" ) );
  }

  coordinates_mode_ = is_local_;

}

void AppStatusBar::set_status_report_label( std::string& status )
{
  QString report = QString::fromStdString( status );
  status_report_label_->setText( QString::fromUtf8( "Status: " ) + report );

}

void AppStatusBar::activate_history( bool is_active_ )
{
  if ( is_active_ )
  {
    history_widget_->show();
  }
  else
  {
    history_widget_->hide();
  }

}
  
void AppStatusBar::fix_icon_status()
{
  status_report_label_->setText( QString::fromUtf8( "Status = true " ) );
}

void AppStatusBar::UpdateStatusBar( qpointer_type statusbar_pointer, int message_type,
    std::string message )
{
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &AppStatusBar::UpdateStatusBar, statusbar_pointer,
        message_type, message ) );
    return;
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( statusbar_pointer.data() )
  {

    std::string status_message = message;
    boost::char_separator< char > separater( " " );
    boost::tokenizer< boost::char_separator< char > > tok( message, separater );

    for ( boost::tokenizer< boost::char_separator< char > >::iterator beg = ++tok.begin(); beg
        != tok.end(); ++beg )
    {
      status_message = status_message + " " + *beg;
    }

    switch( message_type )
    {
    case 0x01:
      statusbar_pointer->status_report_label_->setStyleSheet( QString::fromUtf8(
          "QLabel#status_report_label_{\n"
            " text-align: left;\n"
            " color: rgb(121, 0, 0);\n"
            " font: bold; }\n" ) );
      break;

    case 0x02:
      statusbar_pointer->status_report_label_->setStyleSheet( QString::fromUtf8(
          "QLabel#status_report_label_{\n"
            " text-align: left;\n"
            " color: rgb(165, 161, 34);\n"
            " font: bold; }\n" ) );
      break;
    case 0x04:
      statusbar_pointer->status_report_label_->setStyleSheet( QString::fromUtf8(
          "QLabel#status_report_label_{\n"
            " text-align: left;\n"
            " color: rgb(3, 86, 2);\n"
            " font: bold; \n}" ) );
      break;

    case 0x08:
      statusbar_pointer->status_report_label_->setStyleSheet( QString::fromUtf8(
          "QLabel#status_report_label_{\n"
            " text-align: left;\n"
            " color: purple;\n"
            " font: bold; }\n" ) );
      break;
    default:
      break;
    }

    statusbar_pointer->status_report_label_->setText( QString::fromStdString( message ) );

  }
}

} // end namespace Seg3d
