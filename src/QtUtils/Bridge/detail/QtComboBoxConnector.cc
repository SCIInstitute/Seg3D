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

#include <QtUtils/Bridge/detail/QtComboBoxConnector.h>

namespace QtUtils
{

QtComboBoxConnector::QtComboBoxConnector( QComboBox* parent, 
          Core::StateOptionHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtComboBoxConnector > qpointer( this );
  
  // Populate the content of the state variable to the combo box
  UpdateComboBoxItems( qpointer );
  
  this->connect( parent, SIGNAL( currentIndexChanged( QString ) ),
    SLOT( set_state( QString ) ) );

  this->add_connection( state->value_changed_signal_.connect( boost::bind(
    &QtComboBoxConnector::SetComboBoxIndexByText, qpointer, _2 ) ) );
  this->add_connection( state->optionlist_changed_signal_.connect( boost::bind(
    &QtComboBoxConnector::UpdateComboBoxItems, qpointer ) ) );
}

QtComboBoxConnector::QtComboBoxConnector( QComboBox* parent, 
  Core::StateLabeledOptionHandle& state, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state )
{
  QPointer< QtComboBoxConnector > qpointer( this );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // Populate the content of the state variable to the combo box
  UpdateComboBoxItems( qpointer );

  this->connect( parent, SIGNAL( currentIndexChanged( QString ) ),
    SLOT( set_state( QString ) ) );

  this->add_connection( state->value_changed_signal_.connect( boost::bind(
    &QtComboBoxConnector::SetComboBoxIndexByData, qpointer, _3 ) ) );
  this->add_connection( state->optionlist_changed_signal_.connect( boost::bind(
    &QtComboBoxConnector::UpdateComboBoxItems, qpointer ) ) );
}

QtComboBoxConnector::~QtComboBoxConnector()
{
  this->disconnect_all();
}

void QtComboBoxConnector::set_state( const QString& value )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->state_, 
      value.toStdString() );
  }
}

void QtComboBoxConnector::SetComboBoxIndexByText( 
  QPointer< QtComboBoxConnector > qpointer,
  Core::ActionSource source )
{
  // NOTE: Not checking ActionSource so that multiple widgets can be connected to the same state 
  // and updated simultaneously.  This is safe because there will be at most one extra call to set
  // the state since the state will be unchanged in subsequent calls.

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtComboBoxConnector::SetComboBoxIndexByText,
      qpointer, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  // set the current index in the combo box by searching for matching text
  QComboBox* combobox = qpointer->parent_;
  std::string text;
  {
    // Get the current option state
    // NOTE: Get the value from the state variable directly, the value pass by the signal
    // might not be current.
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    text = dynamic_cast< Core::StateOption* >( qpointer->state_.get() )->get();
  }
  int index = combobox->findText( QString( text.c_str() ) );
  if ( index != -1 )
  {
    combobox->setCurrentIndex( index );
  }

  // unblock signals
  qpointer->unblock();
}

void QtComboBoxConnector::SetComboBoxIndexByData( 
  QPointer< QtComboBoxConnector > qpointer,
  Core::ActionSource source )
{
  // NOTE: Not checking ActionSource so that multiple widgets can be connected to the same state 
  // and updated simultaneously.  This is safe because there will be at most one extra call to set
  // the state since the state will be unchanged in subsequent calls.

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtComboBoxConnector::SetComboBoxIndexByData,
      qpointer, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  // set the current index in the combo box by searching for matching data
  QComboBox* combobox = qpointer->parent_;
  std::string data;
  {
    // Get the current option state
    // NOTE: Get the value from the state variable directly, the value pass by the signal
    // might not be current.
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    data = dynamic_cast< Core::StateLabeledOption* >( qpointer->state_.get() )->get();
  }
  int index = combobox->findData( QVariant( data.c_str() ) );
  if ( index != -1 )
  {
    combobox->setCurrentIndex( index );
  }

  // unblock signals
  qpointer->unblock();
}

void QtComboBoxConnector::UpdateComboBoxItems( QPointer< QtComboBoxConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtComboBoxConnector::UpdateComboBoxItems, qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  QComboBox* combobox = qpointer->parent_;

  // lock the state engine
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  Core::StateBase* state_base = qpointer->state_.get();
  if ( typeid( *state_base ) == typeid( Core::StateOption ) )
  {
    Core::StateOption* state_option = static_cast< Core::StateOption* >( state_base );
    std::vector< std::string > option_list = state_option->option_list();
    int num_items = static_cast< int >( option_list.size() );
    if ( combobox->count() == num_items )
    {
      for ( int i = 0; i < num_items; i++ )
      {
        combobox->setItemText( i, QString( option_list[ i ].c_str() ) );
      }   
    }
    else
    {
      combobox->clear();
      for ( int i = 0; i < num_items; i++ )
      {
        combobox->addItem( QString( option_list[ i ].c_str() ) );
      }
    }
    combobox->setCurrentIndex( state_option->index() );
  }
  else
  {
    Core::StateLabeledOption* state_labeled_option = 
      static_cast< Core::StateLabeledOption* >( state_base );
    std::vector< Core::OptionLabelPair > option_list = state_labeled_option->get_option_list();
    int num_items = static_cast< int >( option_list.size() );
    if ( combobox->count() == num_items )
    {
      for ( int i = 0; i < num_items; i++ )
      {
        combobox->setItemText( i, QString( option_list[ i ].second.c_str() ) );
        combobox->setItemData( i, QVariant( option_list[ i ].first.c_str() ) );
      }
    }
    else
    {
      combobox->clear();
      for ( int i = 0; i < num_items; i++ )
      {
        combobox->addItem( QString( option_list[ i ].second.c_str() ) );
        combobox->setItemData( i, QVariant( option_list[ i ].first.c_str() ) );
      }
    }
    combobox->setCurrentIndex( state_labeled_option->index() );
  }
  
  // unblock signals
  qpointer->unblock();
}

} // end namespace QtUtils
