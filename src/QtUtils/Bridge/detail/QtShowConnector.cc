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
#include <Core/State/Actions/ActionSet.h>

// QtUtils includes
#include <QtUtils/Bridge/detail/QtShowConnector.h>

namespace QtUtils
{

QtShowConnector::QtShowConnector( QWidget* parent, 
  Core::StateBoolHandle& state, bool opposite_logic, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  opposite_logic_( opposite_logic )
{
  QPointer< QtShowConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setVisible( state->get() ^ this->opposite_logic_ );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtShowConnector::ShowWidget, qpointer, _1 ) ) );
  }
}

QtShowConnector::QtShowConnector( QtCustomDockWidget* parent, 
  Core::StateBoolHandle& state, bool opposite_logic, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state ),
  opposite_logic_( opposite_logic )
{
  QPointer< QtShowConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setVisible( state->get() ^ this->opposite_logic_ );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtShowConnector::ShowWidget, qpointer, _1 ) ) );
  }
  
  this->connect( parent, SIGNAL( closed() ), SLOT( set_state() ) );
  this->connect( parent, SIGNAL( opened() ), SLOT( set_state_visible() ) );
}

QtShowConnector::QtShowConnector( QtCustomDialog* parent, 
  Core::StateBoolHandle& state, bool opposite_logic, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state ),
  opposite_logic_( opposite_logic )
{
  QPointer< QtShowConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setVisible( state->get() ^ this->opposite_logic_ );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtShowConnector::ShowWidget, qpointer, _1 ) ) );
  }

  this->connect( parent, SIGNAL( closed() ), SLOT( set_state() ) );
  this->connect( parent, SIGNAL( opened() ), SLOT( set_state_visible() ) );
}

QtShowConnector::QtShowConnector( QWidget* parent, 
  Core::StateBaseHandle state, 
  boost::function< bool () > condition, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  condition_( condition )
{
  QPointer< QtShowConnector > qpointer( this );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  this->add_connection( state->state_changed_signal_.connect(
    boost::bind( &QtShowConnector::ShowWidget, qpointer ) ) );
  parent->setVisible( condition() );
}

QtShowConnector::QtShowConnector( QWidget* parent, 
  std::vector< Core::StateBaseHandle >& states, 
  boost::function< bool () > condition, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  condition_( condition )
{
  assert( states.size() > 0 );
  QPointer< QtShowConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    for ( size_t i = 0; i < states.size(); ++i )
    {
      this->add_connection( states[ i ]->state_changed_signal_.connect(
        boost::bind( &QtShowConnector::ShowWidget, qpointer ) ) );
    }
    parent->setVisible( condition() );
  }
}

QtShowConnector::~QtShowConnector()
{
  this->disconnect_all();
}

void QtShowConnector::ShowWidget( QPointer< QtShowConnector > qpointer, bool visible )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtShowConnector::ShowWidget, qpointer, visible ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }
  
  qpointer->parent_->setVisible( visible ^ qpointer->opposite_logic_ );
}

void QtShowConnector::ShowWidget( QPointer< QtShowConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtShowConnector::ShowWidget, qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  qpointer->parent_->setVisible( qpointer->condition_() );
}


void QtShowConnector::set_state()
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(), this->state_, false );
  }
}

void QtShowConnector::set_state_visible()
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(), this->state_, true );
  }
}


} // end namespace QtUtils
