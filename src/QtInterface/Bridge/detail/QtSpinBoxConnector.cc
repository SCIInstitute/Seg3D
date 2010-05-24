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

#include <QtInterface/Bridge/detail/QtSpinBoxConnector.h>

namespace QtUtils
{

QtSpinBoxConnector::QtSpinBoxConnector( QSpinBox* parent, 
  Core::StateIntHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  spinbox_( parent ),
  state_( state )
{
  QPointer< QtSpinBoxConnector > qpointer( this );
  
  UpdateIntDefaults( qpointer );

  this->connect( parent, SIGNAL( valueChanged( int ) ), SLOT( set_state( int ) ) );
  this->add_connection( state->value_changed_signal_.connect(
    boost::bind( &QtSpinBoxConnector::SetSpinBoxValue, qpointer, _1, _2 ) ) );
}

QtSpinBoxConnector::QtSpinBoxConnector( QDoubleSpinBox* parent, 
  Core::StateDoubleHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  double_spinbox_( parent ),
  state_( state )
{
  QPointer< QtSpinBoxConnector > qpointer( this );
  
  UpdateDoubleDefaults( qpointer );

  this->connect( parent, SIGNAL( valueChanged( double ) ), SLOT( set_state( double ) ) );
  this->add_connection( state->value_changed_signal_.connect(
    boost::bind( &QtSpinBoxConnector::SetDoubleSpinBoxValue, qpointer, _1, _2 ) ) );
}

QtSpinBoxConnector::~QtSpinBoxConnector()
{
  this->disconnect_all();
}

void QtSpinBoxConnector::SetSpinBoxValue( QPointer< QtSpinBoxConnector > qpointer,
    int val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::SetSpinBoxValue,
      qpointer, val, source ) );
    return;
  }

  if ( qpointer.isNull() )
  {
    return;
  }

  qpointer->block();
  qpointer->spinbox_->setValue( val );
  qpointer->unblock();
}

void QtSpinBoxConnector::SetDoubleSpinBoxValue( QPointer< QtSpinBoxConnector > qpointer,
    double val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::SetDoubleSpinBoxValue,
      qpointer, val, source ) );
    return;
  }

  if ( qpointer.isNull() )
  {
    return;
  }

  qpointer->block();
  qpointer->double_spinbox_->setValue( val );
  qpointer->unblock();
}

void QtSpinBoxConnector::set_state( int val )
{
  if( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( this->state_, val );
  }
}

void QtSpinBoxConnector::set_state( double val )
{
  if( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( this->state_, val );
  }
}
  
void QtSpinBoxConnector::UpdateIntDefaults( QPointer< QtSpinBoxConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::UpdateIntDefaults, qpointer ) );
    return;
  }
  
  if ( qpointer.isNull() )
  {
    return;
  }
  
  // block signals back to the application thread
  qpointer->block();
  
  QSpinBox* spinbox = qpointer->spinbox_;
  
  // lock the state engine
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  Core::StateBase* state_base = qpointer->state_.get();
  
  if ( typeid( *state_base ) == typeid( Core::StateInt ) )
  {
    Core::StateInt* state_int = static_cast< Core::StateInt* > ( state_base );
    spinbox->setValue( state_int->get() );
  }
  
  // unblock signals
  qpointer->unblock();  
}
void QtSpinBoxConnector::UpdateDoubleDefaults( QPointer< QtSpinBoxConnector > qpointer )  
{ 
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::UpdateDoubleDefaults, qpointer ) );
    return;
  }
  
  if ( qpointer.isNull() )
  {
    return;
  }
  
  // block signals back to the application thread
  qpointer->block();
  
  QDoubleSpinBox* spinbox = qpointer->double_spinbox_;
  
  // lock the state engine
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  Core::StateBase* state_base = qpointer->state_.get();
  
  if ( typeid( *state_base ) == typeid( Core::StateDouble ) )
  {
    Core::StateDouble* state_double = static_cast< Core::StateDouble* > ( state_base );
    spinbox->setValue( state_double->get() );
  }
  
  // unblock signals
  qpointer->unblock();  
  
    
}

} // end namespace QtUtils