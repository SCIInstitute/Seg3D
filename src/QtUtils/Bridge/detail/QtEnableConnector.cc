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

// QtUtils includes
#include <QCoreApplication>
#include <QPointer>
#include <QtGui/QWidget>

// Core includes
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Bridge/detail/QtEnableConnector.h>

namespace QtUtils
{

QtEnableConnector::QtEnableConnector( QWidget* parent, 
  Core::StateBoolHandle& state, bool opposite_logic ) :
  QObject( static_cast<QObject*>( parent ) ),
  parent_( parent ),
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

QtEnableConnector::QtEnableConnector( QWidget* parent, 
  std::vector< Core::StateBoolHandle >& states ) :
  QObject( parent ),
  parent_( parent ),
  bool_states_( states )
{
  assert( states.size() > 0 );
  QPointer< QtEnableConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    bool enabled = true;
    for ( size_t i = 0; i < states.size(); ++i )
    {
      enabled = enabled && states[ i ]->get();
      this->add_connection( states[ i ]->value_changed_signal_.connect(
        boost::bind( &QtEnableConnector::EnableWidget, qpointer ) ) );
    }
    parent->setEnabled( enabled );
  }
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
  bool enabled = true;
  for ( size_t i = 0; i < qpointer->bool_states_.size() && enabled; ++i )
  {
    enabled = enabled && qpointer->bool_states_[ i ]->get();
  }
  qpointer->parent_->setEnabled( enabled );
}


} // end namespace QtUtils
