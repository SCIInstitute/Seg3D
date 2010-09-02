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
#include <QWidget>

// Core includes
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Bridge/detail/QtShowConnector.h>

namespace QtUtils
{

QtShowConnector::QtShowConnector( QWidget* parent, 
  Core::StateBoolHandle& state, bool opposite_logic ) :
  QObject( parent ),
  parent_( parent ),
  opposite_logic_( opposite_logic )
{
  QPointer< QtShowConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->setVisible( state->get() ^ this->opposite_logic_ );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtShowConnector::ShowWidget, qpointer, _1, _2 ) ) );
  }
}

QtShowConnector::~QtShowConnector()
{
  this->disconnect_all();
}

void QtShowConnector::ShowWidget( QPointer< QtShowConnector > qpointer,
    bool visible, Core::ActionSource source )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtShowConnector::ShowWidget,
      qpointer, visible, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }
  
  qpointer->parent_->setVisible( visible ^ qpointer->opposite_logic_ );
}

} // end namespace QtUtils
