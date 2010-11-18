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
#include<boost/tokenizer.hpp>

// Core Includes
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>

// Application Includes
#include <Application/StatusBar/StatusBar.h>

// QtUtils includes
#include <QtUtils/Utils/QtPointer.h>
#include <QtUtils/Bridge/QtBridge.h>

// Interface Includes
#include <Interface/AppInterface/MessageWindow.h>

#include "ui_MessageWindow.h"

namespace Seg3D
{

class MessageWindowPrivate
{
public:

  Ui::MessageWindow ui_;

};

MessageWindow::MessageWindow( QWidget *parent ) :
  QtUtils::QtCustomDialog( parent ),
  private_( new MessageWindowPrivate )  
{
  // Set up the private internals of the MessageWindow class
  this->private_->ui_.setupUi( this );
  
  QIcon icon = windowIcon();
  Qt::WindowFlags flags = windowFlags();
  Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & ( ~helpFlag );
  this->setWindowFlags( flags );
  this->setWindowIcon( icon );

  this->add_connection( StatusBar::Instance()->message_updated_signal_.connect( 
    boost::bind( &MessageWindow::AddMessage, QPointer< MessageWindow >( this ), _1, _2 ) ) );
}

MessageWindow::~MessageWindow()
{
  this->disconnect_all();
}

void MessageWindow::AddMessage( QPointer< MessageWindow > qpointer, int msg_type, std::string message )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &MessageWindow::add_message, qpointer.data(), msg_type, message ) ) );
}

void MessageWindow::add_message( int msg_type, std::string message )
{
  QColor color_ = QColor( 255, 255, 255 );
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
    case Core::LogMessageType::ERROR_E:
      color_ = QColor(121, 0, 0);
      break;
    case Core::LogMessageType::WARNING_E:
      color_ = QColor(165, 161, 34);
      break;
    case Core::LogMessageType::MESSAGE_E:
      color_ = QColor(3, 86, 2);
      break;
    case Core::LogMessageType::DEBUG_E:
      color_ = QColor("purple");
      break;
    default:
      break;
  }
  
  QListWidgetItem* item = new QListWidgetItem( QString::fromStdString( message ), this->private_->ui_.history_list_ );
  item->setTextColor( color_ );
  this->private_->ui_.history_list_->addItem( item );
  this->private_->ui_.history_list_->setCurrentItem( item );
}

} // end namespace Seg3D