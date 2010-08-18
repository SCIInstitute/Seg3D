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

// QtUtils includes
#include <QCoreApplication>
#include <QPointer>
#include <QtGui/QWidget>

// Core includes
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Bridge/detail/QtEnableConnector.h>

namespace QtUtils
{

QtEnableConnector::QtEnableConnector( QWidget* parent, Core::StateBoolHandle& state ) :
  QObject( static_cast<QObject*>( parent ) ),
  parent_( parent )
{
  QPointer< QtEnableConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setEnabled( state->get() );
    
    //QtEnableConnector::EnableWidget( qpointer, state->get(), Core::ActionSource::NONE_E );

    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtEnableConnector::EnableWidget, qpointer, _1, _2 ) ) );
  }

}

QtEnableConnector::~QtEnableConnector()
{
  this->disconnect_all();
}

void QtEnableConnector::EnableWidget( QPointer< QtEnableConnector > qpointer,
    bool enabled, Core::ActionSource source )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtEnableConnector::EnableWidget,
      qpointer, enabled, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }
  
  qpointer->parent_->setEnabled( enabled );
}

} // end namespace QtUtils
