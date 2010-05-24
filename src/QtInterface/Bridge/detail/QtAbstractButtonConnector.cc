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

#include <QtInterface/Bridge/detail/QtAbstractButtonConnector.h>

namespace QtUtils
{

QtAbstractButtonConnector::QtAbstractButtonConnector( QAbstractButton* parent, 
    Core::StateBoolHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtAbstractButtonConnector > qpointer( this );
  
  this->UpdateChecked( qpointer );

  parent->setCheckable( true );
  this->connect( parent, SIGNAL( toggled( bool ) ), SLOT( set_state( bool ) ) );
  this->add_connection( state->value_changed_signal_.connect(
    boost::bind( &QtAbstractButtonConnector::SetButtonChecked, qpointer, _1, _2 ) ) );
}

QtAbstractButtonConnector::QtAbstractButtonConnector( QAbstractButton* parent, 
                           boost::function< void() > func ) :
  QtConnectorBase( parent ),
  parent_( parent ),
  func_( func )
{
  this->connect( parent, SIGNAL( clicked() ), SLOT( call_func() ) );
}

QtAbstractButtonConnector::~QtAbstractButtonConnector()
{
  this->disconnect_all();
}

void QtAbstractButtonConnector::SetButtonChecked( 
    QPointer< QtAbstractButtonConnector > qpointer, 
    bool checked, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtAbstractButtonConnector::SetButtonChecked,
      qpointer, checked, source ) );
    return;
  }

  if ( qpointer.isNull() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->setChecked( checked );
  qpointer->unblock();
}

void QtAbstractButtonConnector::set_state( bool value )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( this->state_, value );
  }
}

void QtAbstractButtonConnector::call_func()
{
  this->func_();
}

void QtAbstractButtonConnector::UpdateChecked( QPointer< QtAbstractButtonConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtAbstractButtonConnector::UpdateChecked, qpointer ) );
    return;
  }
  
  if ( qpointer.isNull() )
  {
    return;
  }
  
  // block signals back to the application thread
  qpointer->block();
  
  QAbstractButton* button_or_checkbox = qpointer->parent_;
  
  // lock the state engine
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  Core::StateBase* state_base = qpointer->state_.get();
  
  if ( typeid( *state_base ) == typeid( Core::StateBool ) )
  {
    Core::StateBool* state_bool = static_cast< Core::StateBool* > ( state_base );
    
    button_or_checkbox->setChecked( state_bool->get() );
    
  }
  
  // unblock signals
  qpointer->unblock();    
}
  
  
  
} // end namespace QtUtils