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
#include <Core/State/Actions/ActionSetRange.h>

#include <QtUtils/Bridge/detail/QtSliderDoubleComboConnector.h>

namespace QtUtils
{

QtSliderDoubleComboConnector::QtSliderDoubleComboConnector( QtSliderDoubleCombo* parent, 
    Core::StateRangedDoubleHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtSliderDoubleComboConnector > qpointer( this );
  
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    double min_val, max_val, step_size;
    state->get_range( min_val, max_val );
    state->get_step( step_size );
    parent->setRange( min_val, max_val );
    parent->setCurrentValue( state->get() );
    parent->setStep( step_size );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtSliderDoubleComboConnector::SetValue, qpointer, _1, _2 ) ) );
    this->add_connection( state->range_changed_signal_.connect(
      boost::bind( &QtSliderDoubleComboConnector::SetRange, qpointer, _1, _2, _3 ) ) );
    this->add_connection( state->step_changed_signal_.connect(
      boost::bind( &QtSliderDoubleComboConnector::SetStep, qpointer, _1, _2 ) ) );

  }

  this->connect( parent, SIGNAL( valueAdjusted( double ) ), SLOT( set_state_value( double ) ) );
  this->connect( parent, SIGNAL( rangeChanged( double, double ) ), SLOT( set_state_range( double, double ) ) );
}

QtSliderDoubleComboConnector::~QtSliderDoubleComboConnector()
{
  this->disconnect_all();
}

void QtSliderDoubleComboConnector::SetValue( QPointer< QtSliderDoubleComboConnector > qpointer, 
    double val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSliderDoubleComboConnector::SetValue,
      qpointer, val, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->setCurrentValue( val );
  qpointer->unblock();
}

void QtSliderDoubleComboConnector::SetRange( QPointer< QtSliderDoubleComboConnector > qpointer, 
    double min_val, double max_val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSliderDoubleComboConnector::SetRange,
      qpointer, min_val, max_val, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->setRange( min_val, max_val );
  qpointer->unblock();
}

void QtSliderDoubleComboConnector::SetStep( QPointer< QtSliderDoubleComboConnector > qpointer, 
                      double step, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSliderDoubleComboConnector::SetStep,
      qpointer, step, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->setStep( step );
  qpointer->unblock();
}

void QtSliderDoubleComboConnector::set_state_value( double val )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, val );
  }
}

void QtSliderDoubleComboConnector::set_state_range( double min_val, double max_val )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSetRange::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_,
      min_val, max_val );
  }
}
  
} // end namespace QtUtils
