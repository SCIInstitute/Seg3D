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

#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Bridge/detail/QtColorBarWidgetConnector.h>

namespace QtUtils
{

QtColorBarWidgetConnector::QtColorBarWidgetConnector( 
    QtColorBarWidget* parent, Core::StateIntHandle& state, 
    std::vector< Core::StateColorHandle >& colors, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtColorBarWidgetConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    // Create color buttons for color bar
    for ( size_t j = 0; j < colors.size(); j++ )
    {
      QtColorButton* color_button =  new QtColorButton( parent, 
        static_cast< int >( j ), colors[ j ]->get(), 16, 16 );
      QtBridge::Connect( color_button, colors[ j ] );
      parent->add_color_button( color_button, static_cast< int >( j ) );
    }
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtColorBarWidgetConnector::SetColorIndex, qpointer, _1, _2 ) ) );
  }

  this->connect( parent, SIGNAL( color_index_changed( int ) ), SLOT( set_state( int ) ) );
}

QtColorBarWidgetConnector::~QtColorBarWidgetConnector()
{
  this->disconnect_all();
}

void QtColorBarWidgetConnector::SetColorIndex( 
    QPointer< QtColorBarWidgetConnector > qpointer, 
    int index, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtColorBarWidgetConnector::SetColorIndex,
      qpointer, index, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->set_color_index( index );
  qpointer->unblock();
}

void QtColorBarWidgetConnector::set_state( int value )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, value );
  }
}

} // end namespace QtUtils
