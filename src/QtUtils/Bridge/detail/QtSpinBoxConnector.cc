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

#include <QtUtils/Bridge/detail/QtSpinBoxConnector.h>

namespace QtUtils
{

QtSpinBoxConnector::QtSpinBoxConnector( QSpinBox* parent, 
  Core::StateIntHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  spinbox_( parent ),
  state_( state )
{
  QPointer< QtSpinBoxConnector > qpointer( this );
  
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setValue( state->get() );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtSpinBoxConnector::SetSpinBoxValue, qpointer, _1, _2 ) ) );
  }

  this->connect( parent, SIGNAL( valueChanged( int ) ), SLOT( set_state( int ) ) );
}

QtSpinBoxConnector::QtSpinBoxConnector( QSpinBox* parent, 
                     Core::StateRangedIntHandle& state, 
                     bool blocking /*= true */ ) :
  QtConnectorBase( parent, blocking ),
  spinbox_( parent ),
  state_( state )
{
  QPointer< QtSpinBoxConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    int min_val, max_val, step;
    state->get_range( min_val, max_val );
    state->get_step( step );
    parent->setValue( state->get() );
    parent->setRange( min_val, max_val );
    parent->setSingleStep( step );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtSpinBoxConnector::SetSpinBoxValue, qpointer, _1, _2 ) ) );
    this->add_connection( state->range_changed_signal_.connect( boost::bind(
      &QtSpinBoxConnector::SetSpinBoxRange, qpointer, _1, _2, _3 ) ) );
  }

  this->connect( parent, SIGNAL( valueChanged( int ) ), SLOT( set_state( int ) ) );
}

QtSpinBoxConnector::QtSpinBoxConnector( QDoubleSpinBox* parent, 
  Core::StateDoubleHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  double_spinbox_( parent ),
  state_( state )
{
  QPointer< QtSpinBoxConnector > qpointer( this );
  
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setValue( state->get() );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtSpinBoxConnector::SetDoubleSpinBoxValue, qpointer, _1, _2 ) ) );
  }

  this->connect( parent, SIGNAL( valueChanged( double ) ), SLOT( set_state( double ) ) );
}

QtSpinBoxConnector::QtSpinBoxConnector( QDoubleSpinBox* parent, 
  Core::StateRangedDoubleHandle& state, bool blocking /*= true */ ) :
  QtConnectorBase( parent, blocking ),
  double_spinbox_( parent ),
  state_( state )
{
  QPointer< QtSpinBoxConnector > qpointer( this );
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    double min_val, max_val, step;
    state->get_range( min_val, max_val );
    state->get_step( step );
    parent->setMinimum( min_val );
    parent->setMaximum( max_val );
    parent->setValue( state->get() );
    parent->setSingleStep( step );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtSpinBoxConnector::SetDoubleSpinBoxValue, qpointer, _1, _2 ) ) );
    this->add_connection( state->range_changed_signal_.connect(
      boost::bind( &QtSpinBoxConnector::SetDoubleSpinBoxRange, qpointer, _1, _2, _3 ) ) );
  }
  this->connect( parent, SIGNAL( valueChanged( double ) ), SLOT( set_state( double ) ) );
}

QtSpinBoxConnector::~QtSpinBoxConnector()
{
  this->disconnect_all();
}

void QtSpinBoxConnector::SetSpinBoxValue( QPointer< QtSpinBoxConnector > qpointer,
    int val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::SetSpinBoxValue,
      qpointer, val, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->spinbox_->setValue( val );
  qpointer->unblock();
}

void QtSpinBoxConnector::SetSpinBoxRange( QPointer< QtSpinBoxConnector > qpointer,
                     int min_val, int max_val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::SetSpinBoxRange,
      qpointer, min_val, max_val, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->spinbox_->setRange( min_val, max_val );
  qpointer->unblock();
}

void QtSpinBoxConnector::SetDoubleSpinBoxValue( QPointer< QtSpinBoxConnector > qpointer,
    double val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::SetDoubleSpinBoxValue,
      qpointer, val, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->double_spinbox_->setValue( val );
  qpointer->unblock();
}

void QtSpinBoxConnector::SetDoubleSpinBoxRange( QPointer< QtSpinBoxConnector > qpointer, 
    double min_val, double max_val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSpinBoxConnector::SetDoubleSpinBoxRange,
      qpointer, min_val, max_val, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->double_spinbox_->setMinimum( min_val );
  qpointer->double_spinbox_->setMaximum( max_val );
  qpointer->unblock();
}

void QtSpinBoxConnector::set_state( int val )
{
  if( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, val );
  }
}

void QtSpinBoxConnector::set_state( double val )
{
  if( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, val );
  }
}

} // end namespace QtUtils
