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

#include <QtUtils/Bridge/detail/QtActionGroupConnector.h>

namespace QtUtils
{

QtActionGroupConnector::QtActionGroupConnector( QActionGroup* parent, 
    Core::StateOptionHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtActionGroupConnector > qpointer( this );

  // TODO: Create QAction's for each option in the state variable and add them to 
  // the QActionGroup. Should not add anything to the QActionGroup outside.

  this->connect( parent, SIGNAL( triggered( QAction* ) ), SLOT( set_state( QAction* ) ) );
  this->add_connection( state->value_changed_signal_.connect(
    boost::bind( &QtActionGroupConnector::TriggerAction, qpointer, _1, _2 ) ) );
}

QtActionGroupConnector::~QtActionGroupConnector()
{
  this->disconnect_all();
}

void QtActionGroupConnector::TriggerAction( 
    QPointer< QtActionGroupConnector > qpointer, 
    std::string option, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtActionGroupConnector::TriggerAction,
      qpointer, option, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();

  QList< QAction* > actions = qpointer->parent_->actions();
  for ( QList< QAction* >::iterator it = actions.begin(); it != actions.end(); it++ )
  {
    if ( ( *it )->objectName().toStdString() == option )
    {
      ( *it )->trigger();
      break;
    }
  }

  qpointer->unblock();
}

void QtActionGroupConnector::set_state( QAction* action )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->state_, action->objectName().toStdString() );
  }
}

} // end namespace QtUtils
