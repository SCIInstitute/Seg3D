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

#include <QtUtils/Bridge/detail/QtActionConnector.h>

namespace QtUtils
{

QtActionConnector::QtActionConnector( QAction* parent, 
    Core::StateBoolHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtActionConnector > qpointer( this );

  parent->setCheckable( true );
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setChecked( state->get() );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtActionConnector::SetActionChecked, qpointer, _1, _2 ) ) );
  }

  this->connect( parent, SIGNAL( toggled( bool ) ), SLOT( set_state( bool ) ) );
}

QtActionConnector::QtActionConnector( QAction* parent, 
  boost::function< void() > func ) :
  QtConnectorBase( parent ),
  parent_( parent ),
  func_( func )
{
  this->connect( parent, SIGNAL( triggered() ), SLOT( call_func() ) );
}

QtActionConnector::~QtActionConnector()
{
  this->disconnect_all();
}

void QtActionConnector::SetActionChecked( 
    QPointer< QtActionConnector > qpointer, 
    bool checked, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtActionConnector::SetActionChecked,
      qpointer, checked, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->setChecked( checked );
  qpointer->unblock();
}

  
void QtActionConnector::set_state( bool value )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->state_, value );
  }
}

void QtActionConnector::call_func()
{
  this->func_();
}

} // end namespace QtUtils
