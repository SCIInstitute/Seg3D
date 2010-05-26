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

#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>
#include <Core/State/StateEngine.h>

#include <QtInterface/Bridge/detail/QtComboBoxConnector.h>

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
    &QtComboBoxConnector::SetComboBoxIndexByText, qpointer, _1, _2 ) ) );
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
  
  // Populate the content of the state variable to the combo box
  UpdateComboBoxItems( qpointer );

  this->connect( parent, SIGNAL( currentIndexChanged( QString ) ),
    SLOT( set_state( QString ) ) );

  this->add_connection( state->value_changed_signal_.connect( boost::bind(
    &QtComboBoxConnector::SetComboBoxIndexByData, qpointer, _2, _3 ) ) );
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
    Core::ActionSet::Dispatch( this->state_, value.toStdString() );
  }
}

void QtComboBoxConnector::SetComboBoxIndexByText( 
  QPointer< QtComboBoxConnector > qpointer,
  std::string text, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtComboBoxConnector::SetComboBoxIndexByText,
      qpointer, text, source ) );
    return;
  }

  if ( qpointer.isNull() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  // set the current index in the combo box by searching for matching text
  QComboBox* combobox = qpointer->parent_;
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
  std::string data, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtComboBoxConnector::SetComboBoxIndexByData,
      qpointer, data, source ) );
    return;
  }

  if ( qpointer.isNull() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  // set the current index in the combo box by searching for matching data
  QComboBox* combobox = qpointer->parent_;
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

  if ( qpointer.isNull() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  QComboBox* combobox = qpointer->parent_;
  combobox->clear();

  // lock the state engine
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  Core::StateBase* state_base = qpointer->state_.get();
  if ( typeid( *state_base ) == typeid( Core::StateOption ) )
  {
    Core::StateOption* state_option = static_cast< Core::StateOption* >( state_base );
    std::vector< std::string > option_list = state_option->option_list();
    size_t num_items = option_list.size();
    for ( size_t i = 0; i < num_items; i++ )
    {
      combobox->addItem( QString( option_list[ i ].c_str() ) );
    }
    combobox->setCurrentIndex( state_option->index() );
  }
  else
  {
    Core::StateLabeledOption* state_labeled_option = 
      static_cast< Core::StateLabeledOption* >( state_base );
    std::vector< Core::OptionLabelPair > option_list = state_labeled_option->get_option_list();
    size_t num_items = option_list.size();
    for ( size_t i = 0; i < num_items; i++ )
    {
      combobox->addItem( QString( option_list[ i ].second.c_str() ),
        QVariant( option_list[ i ].first.c_str() ) );
    }
    combobox->setCurrentIndex( state_labeled_option->index() );
  }
  
  // unblock signals
  qpointer->unblock();
}

} // end namespace QtUtils