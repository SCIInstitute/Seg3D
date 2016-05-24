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

#include <QCoreApplication>

#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>

#include <QtUtils/Bridge/detail/QtPlainTextEditConnector.h>

namespace QtUtils
{

QtPlainTextEditConnector::QtPlainTextEditConnector( QPlainTextEdit* parent, 
    Core::StateStringHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  string_state_( state )
{
  QPointer< QtPlainTextEditConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setPlainText( QString::fromStdString( state->get() ) );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtPlainTextEditConnector::SetPlainTextEditText, qpointer, _1, _2 ) ) );
  }

  this->connect( parent, SIGNAL( textChanged() ), SLOT( set_state() ) );
}

QtPlainTextEditConnector::~QtPlainTextEditConnector()
{
  this->disconnect_all();
}

void QtPlainTextEditConnector::SetPlainTextEditText( QPointer< QtPlainTextEditConnector > qpointer, 
    std::string text, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtPlainTextEditConnector::SetPlainTextEditText,
      qpointer, text, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->setPlainText( QString::fromStdString( text ) );
  qpointer->unblock();
}

void QtPlainTextEditConnector::set_state()
{
  if ( !this->is_blocked() )
  {
    std::string text = this->parent_->toPlainText().toStdString();
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->string_state_, text );
  }
}

} // end namespace QtUtils
