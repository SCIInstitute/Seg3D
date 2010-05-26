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

#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>

#include <QtInterface/Bridge/detail/QtLineEditConnector.h>

namespace QtUtils
{

QtLineEditConnector::QtLineEditConnector( QLineEdit* parent, 
    Core::StateStringHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtLineEditConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setText( QString( state->get().c_str() ) );
  }

  this->connect( parent, SIGNAL( editingFinished() ), SLOT( set_state() ) );
  this->add_connection( state->value_changed_signal_.connect(
    boost::bind( &QtLineEditConnector::SetLineEditText, qpointer, _1, _2 ) ) );
}

QtLineEditConnector::QtLineEditConnector( QLineEdit* parent, 
    Core::StateNameHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtLineEditConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setText( QString( state->get().c_str() ) );
  }

  this->connect( parent, SIGNAL( editingFinished() ), SLOT( set_state() ) );

  // NOTE: for StateName, always update the QLineEdit no matter what the source
  // of the change is, because the actual value set in the state may be different from
  // user input.
  this->add_connection( state->value_changed_signal_.connect( boost::bind( 
    &QtLineEditConnector::SetLineEditText, qpointer, _2, Core::ActionSource::NONE_E ) ) );
}


QtLineEditConnector::~QtLineEditConnector()
{
  this->disconnect_all();
}

void QtLineEditConnector::SetLineEditText( QPointer< QtLineEditConnector > qpointer, 
    std::string text, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtLineEditConnector::SetLineEditText,
      qpointer, text, source ) );
    return;
  }

  if ( qpointer.isNull() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->setText( QString( text.c_str() ) );
  qpointer->unblock();
}

void QtLineEditConnector::set_state()
{
  if ( !this->is_blocked() )
  {
    std::string text = this->parent_->text().trimmed().toStdString();
    Core::ActionSet::Dispatch( this->state_, text );
  }
}

} // end namespace QtUtils