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

#include <QtUtils/Bridge/detail/QtColorPickerConnector.h>

namespace QtUtils
{

QtColorPickerConnector::QtColorPickerConnector( QtColorPicker* parent, 
                     Core::StateColorHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtColorPickerConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->set_color( state->get() );
    this->add_connection( state->value_changed_signal_.connect(
      boost::bind( &QtColorPickerConnector::SetColorPickerColor, qpointer, _1, _2 ) ) );
  }

  this->connect( parent, SIGNAL( color_changed( Core::Color ) ), SLOT( set_state( Core::Color ) ) );
}

QtColorPickerConnector::~QtColorPickerConnector()
{
  this->disconnect_all();
}

void QtColorPickerConnector::SetColorPickerColor( QPointer< QtColorPickerConnector > qpointer, 
    Core::Color val, Core::ActionSource source )
{
  // NOTE: Not checking ActionSource so that multiple widgets can be connected to the same state 
  // and updated simultaneously.  For example, a INTERFACE_WIDGET_E source may be another widget, 
  // not this one.  This is safe because there will be at most one extra call to set
  // the state since the state will be unchanged in subsequent calls.

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtColorPickerConnector::SetColorPickerColor,
      qpointer, val, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->set_color( val );
  qpointer->unblock();
}

void QtColorPickerConnector::set_state( Core::Color val )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, val );
  }
}

} // end namespace QtUtils
