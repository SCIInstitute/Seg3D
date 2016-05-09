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
#include <Core/State/StateEngine.h>

#include <QtUtils/Bridge/detail/QtTristateToolButtonConnector.h>

namespace QtUtils
{

QtTristateToolButtonConnector::QtTristateToolButtonConnector( 
  QtTristateToolButton* parent, 
  Core::StateOptionHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtTristateToolButtonConnector > qpointer( this );
  parent->setCheckable( false );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    std::vector< std::string > option_list = state->option_list();
    assert( option_list.size() == 3 );
    parent->set_state_strings( option_list[ 0 ], option_list[ 1 ], option_list[ 2 ] );
    parent->set_state( state->get() );

    this->add_connection( state->value_changed_signal_.connect( boost::bind( 
      &QtTristateToolButtonConnector::SetButtonState, qpointer, _1, _2 ) ) );
  }

  this->connect( parent, SIGNAL( state_changed( std::string ) ), 
    SLOT( set_state( std::string ) ) );
}

QtTristateToolButtonConnector::~QtTristateToolButtonConnector()
{
  this->disconnect_all();
}

void QtTristateToolButtonConnector::set_state( std::string value )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, value );
  }
}

void QtTristateToolButtonConnector::SetButtonState( 
  QPointer< QtTristateToolButtonConnector > qpointer, 
  std::string state, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtTristateToolButtonConnector::SetButtonState,
      qpointer, state, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->set_state( state );
  qpointer->unblock();
}


} // end namespace QtUtils
