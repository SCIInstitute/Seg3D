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

// QtUtils includes
#include <QCoreApplication>
#include <QPointer>
#include <QWidget>

// Core includes
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Bridge/detail/QtEnableConnector.h>

namespace QtUtils
{

QtEnableConnector::QtEnableConnector( QWidget* parent, 
  Core::StateBoolHandle& state, bool opposite_logic ) :
  QtConnectorBase( static_cast<QObject*>( parent ) ),
  parent_( parent ),
  histogram_widget_( false ),
  opposite_logic_( opposite_logic )
{
  QPointer< QtEnableConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setEnabled( state->get() ^ this->opposite_logic_ );
    
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtEnableConnector::EnableWidget, qpointer, _1 ) ) );
  }

}
  
QtEnableConnector::QtEnableConnector( QAction* action_parent, 
  Core::StateBoolHandle& state, bool opposite_logic ) :
  action_parent_( action_parent ),
  opposite_logic_( opposite_logic )
{
  QPointer< QtEnableConnector > qpointer( this );
  
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    action_parent->setEnabled( state->get() ^ this->opposite_logic_ );
    
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtEnableConnector::EnableAction, qpointer, _1 ) ) );
  }
  
}

QtEnableConnector::QtEnableConnector( QtHistogramWidget* histogram, 
  Core::StateBoolHandle& state, bool opposite_logic ) :
  QtConnectorBase( static_cast<QObject*>( histogram ) ),
  parent_( histogram ),
  histogram_widget_( true ),
  opposite_logic_( opposite_logic )
{
  QPointer< QtEnableConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    histogram->setEnabled( state->get() ^ this->opposite_logic_ );

    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtEnableConnector::EnableWidget, qpointer, _1 ) ) );
  }
}

QtEnableConnector::QtEnableConnector( QWidget* parent, 
  std::vector< Core::StateBaseHandle >& states, 
  boost::function< bool () > condition ) :
  QtConnectorBase( parent ),
  parent_( parent ),
  histogram_widget_( false ),
  condition_( condition )
{
  assert( states.size() > 0 );
  QPointer< QtEnableConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    for ( size_t i = 0; i < states.size(); ++i )
    {
      this->add_connection( states[ i ]->state_changed_signal_.connect(
        boost::bind( &QtEnableConnector::EnableWidget, qpointer ) ) );
    }
    parent->setEnabled( condition() );
  }
}

QtEnableConnector::QtEnableConnector( QWidget* parent, 
  Core::StateBaseHandle state, 
  boost::function< bool () > condition ) :
  QtConnectorBase( parent ),
  parent_( parent ),
  histogram_widget_( false ),
  condition_( condition )
{
  QPointer< QtEnableConnector > qpointer( this );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  this->add_connection( state->state_changed_signal_.connect(
    boost::bind( &QtEnableConnector::EnableWidget, qpointer ) ) );
  parent->setEnabled( condition() );
}


QtEnableConnector::~QtEnableConnector()
{
  this->disconnect_all();
}

void QtEnableConnector::EnableWidget( QPointer< QtEnableConnector > qpointer,
    bool enabled )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtEnableConnector::EnableWidget,
      qpointer, enabled ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }
  
  qpointer->parent_->setEnabled( enabled ^ qpointer->opposite_logic_ );
  if( qpointer->histogram_widget_ && !enabled )
  {
    ( static_cast< QtHistogramWidget* >( qpointer->parent_ ) )->reset_histogram();
  }
}

void QtEnableConnector::EnableWidget( QPointer< QtEnableConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtEnableConnector::EnableWidget,
      qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  qpointer->parent_->setEnabled( qpointer->condition_() );
}
  
void QtEnableConnector::EnableAction( 
   QPointer< QtEnableConnector > qpointer, 
   bool enabled )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtEnableConnector::EnableAction,
      qpointer, enabled ) );
    return;
  }
  
  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  qpointer->action_parent_->setEnabled( enabled ^ qpointer->opposite_logic_ );
}


} // end namespace QtUtils
