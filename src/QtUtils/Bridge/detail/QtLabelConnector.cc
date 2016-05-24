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
#include <Core/State/StateEngine.h>
#include <Core/State/StateValue.h>
#include <Core/State/Actions/ActionSet.h>

#include <QtUtils/Bridge/detail/QtLabelConnector.h>

namespace QtUtils
{

QtLabelConnector::QtLabelConnector( QLabel* parent, 
    Core::StateBaseHandle& state ) :
  QtConnectorBase( parent, false ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtLabelConnector > qpointer( this );

  this->is_string_state_ = ( dynamic_cast< Core::StateString* >( state.get() ) != 0 );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    UpdateLabelText( qpointer );
    this->add_connection( state->state_changed_signal_.connect(
      boost::bind( &QtLabelConnector::UpdateLabelText, qpointer ) ) );
  }
}

QtLabelConnector::~QtLabelConnector()
{
  this->disconnect_all();
}

void QtLabelConnector::UpdateLabelText( QPointer< QtLabelConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtLabelConnector::UpdateLabelText, qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  std::string state_str = qpointer->state_->export_to_string();
  if ( qpointer->is_string_state_ )
  {
    if ( state_str.size() > 0 && state_str[ 0 ] == '[' )
    {
      state_str = state_str.substr( 1 );
    }
    if ( state_str.size() > 0 && state_str[ state_str.size() - 1 ] == ']' )
    {
      state_str = state_str.substr( 0, state_str.size() - 1 );
    }
  }
  
  qpointer->parent_->setText( QString::fromStdString( state_str ) );
}

} // end namespace QtUtils
