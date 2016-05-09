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

#include <QtUtils/Bridge/detail/QtListWidgetConnector.h>

namespace QtUtils
{

QtListWidgetConnector::QtListWidgetConnector( QListWidget* parent, 
  Core::StateLabeledMultiOptionHandle& state, bool blocking /*= true */ ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  option_state_( state )
{
  QPointer< QtListWidgetConnector > qpointer( this );
  parent->setSelectionMode( QAbstractItemView::MultiSelection );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // Populate the content of the state variable to the combo box
  UpdateListWidgetOptionItems( qpointer );

  this->connect( parent, SIGNAL( itemSelectionChanged() ), SLOT( set_state() ) );

  this->add_connection( state->value_changed_signal_.connect( boost::bind(
    &QtListWidgetConnector::SetListWidgetSelectedItems, qpointer, _1, _2 ) ) );
  this->add_connection( state->optionlist_changed_signal_.connect( boost::bind(
    &QtListWidgetConnector::UpdateListWidgetOptionItems, qpointer ) ) );
}

QtListWidgetConnector::QtListWidgetConnector( QListWidget* parent, 
                       Core::StateStringVectorHandle& state ) :
  QtConnectorBase( parent, true ),
  parent_( parent ),
  string_vector_state_( state )
{
  QPointer< QtListWidgetConnector > qpointer( this );
  parent->setSelectionMode( QAbstractItemView::SingleSelection );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  UpdateListWidgetStringItems( qpointer );
  this->add_connection( state->state_changed_signal_.connect( boost::bind(
    &QtListWidgetConnector::UpdateListWidgetStringItems, qpointer ) ) );
}

QtListWidgetConnector::~QtListWidgetConnector()
{
  this->disconnect_all();
}

void QtListWidgetConnector::set_state()
{
  if ( this->is_blocked() )
  {
    return;
  }
  
  std::vector< std::string > selections;
  int num_items = this->parent_->count();
  for ( int i = 0; i < num_items; ++i )
  {
    QListWidgetItem* item = this->parent_->item( i );
    if ( item->isSelected() )
    {
      item->setCheckState( Qt::Checked );
      selections.push_back( item->data( Qt::UserRole ).toString().toStdString() );
    }
    else
    {
      item->setCheckState( Qt::Unchecked );
    }
  }
  
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), this->option_state_, 
    selections );
}

static void SetSelectedItemsInternal( QListWidget* listwidget, 
  const std::vector< std::string >& selections )
{
  listwidget->setUpdatesEnabled( false );
  listwidget->clearSelection();
  int num_items = listwidget->count();
  for ( int i = 0; i < num_items; ++i )
  {
    QListWidgetItem* item = listwidget->item( i );
    if ( std::find( selections.begin(), selections.end(), 
      item->data( Qt::UserRole ).toString().toStdString() ) != selections.end() )
    {
      item->setSelected( true );
      item->setCheckState( Qt::Checked );
    }
    else
    {
      item->setCheckState( Qt::Unchecked );
    }
  }
  listwidget->setUpdatesEnabled( true );
}

void QtListWidgetConnector::SetListWidgetSelectedItems( 
  QPointer< QtListWidgetConnector > qpointer,
  std::vector< std::string > selections, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtListWidgetConnector::SetListWidgetSelectedItems,
      qpointer, selections, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  SetSelectedItemsInternal( qpointer->parent_, selections );

  // unblock signals
  qpointer->unblock();
}

void QtListWidgetConnector::UpdateListWidgetOptionItems( 
  QPointer< QtListWidgetConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( 
      &QtListWidgetConnector::UpdateListWidgetOptionItems, qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  // block signals back to the application thread
  qpointer->block();

  QListWidget* listwidget = qpointer->parent_;
  listwidget->clear();

  // lock the state engine
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  std::vector< Core::OptionLabelPair > option_list = qpointer->option_state_->get_option_list();
  int num_items = static_cast< int >( option_list.size() );
  for ( int i = 0; i < num_items; ++i )
  {
    listwidget->addItem( QString::fromStdString( option_list[ i ].second ) );
    QListWidgetItem* item = listwidget->item( i );
    item->setData( Qt::UserRole, QVariant( option_list[ i ].first.c_str() ) );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  }
  SetSelectedItemsInternal( listwidget, qpointer->option_state_->get() );
  
  // unblock signals
  qpointer->unblock();
}

void QtListWidgetConnector::UpdateListWidgetStringItems( 
  QPointer< QtListWidgetConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( 
      &QtListWidgetConnector::UpdateListWidgetStringItems, qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  QListWidget* listwidget = qpointer->parent_;
  listwidget->clear();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::vector< std::string >& strs = qpointer->string_vector_state_->get();
  lock.unlock();
  for ( size_t i = 0; i < strs.size(); ++i )
  {
    listwidget->addItem( QString::fromStdString( strs[ i ] ) );
  }
}

} // end namespace QtUtils
