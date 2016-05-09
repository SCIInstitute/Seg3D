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

#include <QAbstractButton>
#include <QCoreApplication>

#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>
#include <Core/State/StateEngine.h>

#include <QtUtils/Bridge/detail/QtButtonGroupConnector.h>

namespace QtUtils
{

QtButtonGroupConnector::QtButtonGroupConnector( QButtonGroup* parent, 
          Core::StateOptionHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  option_state_( state )
{
  QPointer< QtButtonGroupConnector > qpointer( this );
  QList< QAbstractButton* > buttons = parent->buttons();
  parent->setExclusive( true );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    std::vector< std::string > option_list = state->option_list();
    int num_of_options = static_cast< int >( option_list.size() );
    assert( buttons.size() == num_of_options );
    for ( int i = 0; i < num_of_options; ++i )
    {
      buttons[ i ]->setCheckable( true );
      parent->setId( buttons[ i ], i );
      buttons[ i ]->setText( QString::fromStdString( option_list[ i ] ) );
      if ( i == state->index() )
      {
        buttons[ i ]->setChecked( true );
      }
    }

    this->add_connection( state->value_changed_signal_.connect( boost::bind(
      &QtButtonGroupConnector::SetCheckedButton, qpointer, _2 ) ) );
  }
  
  this->connect( parent, SIGNAL( buttonClicked ( QAbstractButton* ) ),
    SLOT( set_state( QAbstractButton* ) ) );
}

QtButtonGroupConnector::QtButtonGroupConnector( QButtonGroup* parent, 
      Core::StateLabeledOptionHandle& state, bool blocking /*= true */ ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  labeled_option_state_( state )
{
  QPointer< QtButtonGroupConnector > qpointer( this );
  QList< QAbstractButton* > buttons = parent->buttons();
  parent->setExclusive( true );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    std::vector< Core::OptionLabelPair > option_list = state->get_option_list();
    int num_of_options = static_cast< int >( option_list.size() );
    assert( buttons.size() == num_of_options );
    for ( int i = 0; i < num_of_options; ++i )
    {
      buttons[ i ]->setCheckable( true );
      parent->setId( buttons[ i ], i );
      buttons[ i ]->setText( QString::fromStdString( option_list[ i ].second ) );
      if ( i == state->index() )
      {
        buttons[ i ]->setChecked( true );
      }
    }

    this->add_connection( state->value_changed_signal_.connect( boost::bind(
      &QtButtonGroupConnector::SetCheckedButton, qpointer, _3 ) ) );
  }

  this->connect( parent, SIGNAL( buttonClicked ( QAbstractButton* ) ),
    SLOT( set_state( QAbstractButton* ) ) );
}

QtButtonGroupConnector::~QtButtonGroupConnector()
{
  this->disconnect_all();
}

void QtButtonGroupConnector::set_state( QAbstractButton* button )
{
  if ( !this->is_blocked() )
  {
    if ( this->option_state_ )
    {
      Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
        this->option_state_, button->text().toStdString() );
    }
    else
    {
      Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
        this->labeled_option_state_, button->text().toStdString() );
    }
  }
}

void QtButtonGroupConnector::SetCheckedButton( 
  QPointer< QtButtonGroupConnector > qpointer,
  Core::ActionSource source )
{
  // NOTE: Not checking ActionSource so that multiple widgets can be connected to the same state 
  // and updated simultaneously.  This is safe because there will be at most one extra call to set
  // the state since the state will be unchanged in subsequent calls.

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtButtonGroupConnector::SetCheckedButton,
      qpointer, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  if ( qpointer->option_state_ )
  {
    qpointer->parent_->button( qpointer->option_state_->index() )->setChecked( true );
  }
  else
  {
    qpointer->parent_->button( qpointer->labeled_option_state_->index() )->setChecked( true );
  }
  
  // unblock signals
  qpointer->unblock();
}

} // end namespace QtUtils
