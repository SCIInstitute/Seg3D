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

#include <QtUtils/Bridge/detail/QtSliderIntComboConnector.h>

namespace QtUtils
{

QtSliderIntComboConnector::QtSliderIntComboConnector( QtSliderIntCombo* parent, 
  Core::StateRangedIntHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtSliderIntComboConnector > qpointer( this );
  
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    int min_val, max_val, step_size;
    state->get_range( min_val, max_val );
    state->get_step( step_size );
    parent->setRange( min_val, max_val );
    parent->setCurrentValue( state->get() );
    parent->setStep( step_size );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtSliderIntComboConnector::SetValue, qpointer, _1, _2 ) ) );
    this->add_connection( state->range_changed_signal_.connect(
      boost::bind( &QtSliderIntComboConnector::SetRange, qpointer, _1, _2, _3 ) ) );
  }

  this->connect( parent, SIGNAL( valueAdjusted( int ) ), SLOT( set_state_value( int ) ) );
  this->connect( parent, SIGNAL( rangeChanged( int, int ) ), SLOT( set_state_range( int, int ) ) );
}

QtSliderIntComboConnector::~QtSliderIntComboConnector()
{
  this->disconnect_all();
}

void QtSliderIntComboConnector::SetValue( QPointer< QtSliderIntComboConnector > qpointer, 
    int val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSliderIntComboConnector::SetValue,
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

void QtSliderIntComboConnector::SetRange( QPointer< QtSliderIntComboConnector > qpointer, 
    int min_val, int max_val, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtSliderIntComboConnector::SetRange,
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

void QtSliderIntComboConnector::set_state_value( int val )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, val );
  }
}

void QtSliderIntComboConnector::set_state_range( int min_val, int max_val )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSetRange::Dispatch( Core::Interface::GetWidgetActionContext(),this->state_, 
      min_val, max_val );
  }
}
  
} // end namespace QtUtils
