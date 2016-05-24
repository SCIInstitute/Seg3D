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

// boost includes
#include <boost/algorithm/string/replace.hpp>

// Qt includes
#include <QAction>
#include <QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>

// Core includes
#include <Core/Interface/Interface.h>

// Interface includes
#include <Interface/ToolInterface/detail/MeasurementTableView.h>
#include <Interface/ToolInterface/detail/MeasurementTableModel.h>
#include <Interface/ToolInterface/detail/MeasurementItemDelegate.h>

namespace Seg3D
{

// Derived scroll bar that accepts all wheelEvent events rather than passing them on to the 
// parent when scrollbar is at min/max.
class MeasurementScrollBar : public QScrollBar
{
public:
  MeasurementScrollBar( QWidget * parent = 0 ) :
    QScrollBar( parent ) {}

    void wheelEvent( QWheelEvent * e );
};

void MeasurementScrollBar::wheelEvent( QWheelEvent * e )
{
  QScrollBar::wheelEvent( e ); 
  e->accept();
}

class MeasurementTableViewPrivate 
{
public:
  void get_deletion_candidates( std::vector< int >& deletion_candidates ) const;
  QString export_measurement( int index );

  MeasurementTableView * view_;
  QAction* delete_action_;
};

void MeasurementTableViewPrivate::get_deletion_candidates( std::vector< int >& deletion_candidates ) const
{
  MeasurementTableModel* model = qobject_cast< MeasurementTableModel* >( this->view_->model() );
  if( model->rowCount( QModelIndex() ) == 0 ) return;

  // Get selected indices from QTableView
  QModelIndexList selected_rows_list = this->view_->selectionModel()->selectedRows();

  // Make list of candidates for deletion
  deletion_candidates.clear();
  if( selected_rows_list.size() > 0 ) // Rows are selected
  {
    Q_FOREACH ( QModelIndex index, selected_rows_list )
    {
      deletion_candidates.push_back( index.row() );
    }
    std::sort( deletion_candidates.begin(), deletion_candidates.end() );
  }
  else // No rows are selected -- delete active measurement
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    int active_index = model->get_active_index();
    if( active_index != -1 )
    {
      deletion_candidates.push_back( active_index );
    }
  }
}

QString MeasurementTableViewPrivate::export_measurement( int index )
{
  MeasurementTableModel* model = qobject_cast< MeasurementTableModel* >( this->view_->model() );
  QString export_text;

  // Name
  export_text.append( model->data( model->index( index, MeasurementColumns::NAME_E ), 
    Qt::DisplayRole ).toString() ); 
  export_text.append( QLatin1Char('\t') ); 

  // Comment
  std::string comment = model->data( model->index( index, MeasurementColumns::COMMENT_E ), 
    Qt::DisplayRole ).toString().toStdString();
  // Remove line breaks
  boost::replace_all( comment, "\n", " " );
  boost::replace_all( comment, "\r", " " );
  export_text.append( QString::fromStdString( comment ) ); 
  export_text.append( QLatin1Char('\t') ); 

  // Length
  export_text.append( model->data( model->index( index, MeasurementColumns::LENGTH_E ), 
    Qt::DisplayRole ).toString() ); 
  export_text.append( QLatin1Char('\n') );

  return export_text;
}

MeasurementTableView::MeasurementTableView( QWidget* parent ) : 
  QTableView( parent ),
  private_( new MeasurementTableViewPrivate )
{
  this->private_->view_ = this;
  this->private_->delete_action_ = new QAction( tr( "&Delete" ), this );
  
  // Custom editors for length and note columns
  this->setItemDelegate( 
    new MeasurementItemDelegate( MeasurementColumns::COLOR_E, MeasurementColumns::NAME_E, 
    MeasurementColumns::LENGTH_E, this ) );
  this->horizontalHeader()->setSectionsClickable( true );
  QObject::connect( this->horizontalHeader(), SIGNAL( sectionClicked( int ) ), 
    this, SLOT( handle_header_clicked( int ) ) );
  this->setVerticalScrollBar( new MeasurementScrollBar( this ) );
}

void MeasurementTableView::set_measurement_model( MeasurementTableModel* measurement_model )
{
  this->setModel( measurement_model );

  QObject::connect( measurement_model, SIGNAL( modelReset() ), 
    this, SLOT( handle_model_reset() ) );
  QObject::connect( this, SIGNAL( clicked( QModelIndex ) ), 
    measurement_model, SLOT( handle_click( QModelIndex ) ) );
  QObject::connect( this->selectionModel(), 
    SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), 
    this, SLOT( handle_selected() ) );

  // Wait until text editing is finished to save the note for the active measurement.  This 
  // way we avoid updating the model for every keystroke.
  QAbstractItemDelegate* text_delegate = this->itemDelegate();
  QObject::connect( text_delegate, SIGNAL( closeEditor( QWidget* ) ), measurement_model, 
    SLOT( save_cached_active_name() ) );
}

void MeasurementTableView::handle_model_reset()
{
  // The table and headers are reset in response to modelReset() signal, but the reset slots
  // aren't guaranteed to be called before this slot.  We want to ensure that the table and
  // headers have been reset before resizing rows/columns to contents and scrolling to the active
  // index. 
  this->reset();
  this->horizontalHeader()->reset();
  this->verticalHeader()->reset();

  // Hidden comment column is only used for export
  this->setColumnHidden( MeasurementColumns::COMMENT_E, true );

  // Have to resize rows/columns *after* model has been populated
  this->horizontalHeader()->setSectionResizeMode( MeasurementColumns::NAME_E, QHeaderView::Stretch );
  this->resizeColumnsToContents();
  this->resizeRowsToContents();
  
  // Scroll to active measurement
  this->update_active_index();
}

void MeasurementTableView::export_selected_measurements() const
{
  MeasurementTableModel* model = qobject_cast< MeasurementTableModel* >( this->model() );
  QItemSelectionModel* selection = this->selectionModel(); 
  QModelIndexList indexes = selection->selectedIndexes();

  QString selected_text;

  if( indexes.size() > 0 ) // Cells are selected
  {
    // QModelIndex::operator < sorts first by row, then by column. This is what we need. 
    qSort( indexes.begin(), indexes.end() ); 

    int current_row = -1;
    QModelIndex current_index; 
    Q_FOREACH( current_index, indexes ) 
    { 
      if( current_index.row() > current_row )
      {
        current_row = current_index.row();
        selected_text.append( this->private_->export_measurement( current_row ) );
      }
    }
  }
  else // No cells are selected -- export active measurement
  {
    int active_index = model->get_active_index();
    if( 0 <= active_index )
    {
      selected_text.append( this->private_->export_measurement( active_index ) );
    }
  }

  if( !selected_text.isEmpty() )
  {
    qApp->clipboard()->setText( selected_text );
  }
}

void MeasurementTableView::update_active_index()
{
  ASSERT_IS_INTERFACE_THREAD();

  // Lock StateEngine so that state doesn't change between access to active index and 
  // measurements vector
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  MeasurementTableModel* model = 
    qobject_cast< MeasurementTableModel* >( this->model() );
  int active_index = model->get_active_index();
  if( 0 <= active_index && active_index < model->rowCount( QModelIndex() ) )
  {
    // If the active index isn't already selected, select it.  There may be multiple rows
    // selected, in which case we don't want to select the active index because that would
    // deselect everything else.
    bool active_index_selected = false;
    QModelIndexList selected_rows_list = this->selectionModel()->selectedRows();
    Q_FOREACH ( QModelIndex index, selected_rows_list )
    {
      if( index.row() == active_index )
      {
        active_index_selected = true;
      }
    }
    if( !active_index_selected )
    {
      this->selectRow( active_index );
    }
    
    this->scrollTo( model->index( active_index, 0 ) );
  }
}

void MeasurementTableView::delete_selected_measurements()
{
  std::vector< int > deletion_candidates;
  this->private_->get_deletion_candidates( deletion_candidates );
  if( deletion_candidates.size() == 0 ) 
  {
    return;
  }

  bool delete_confirmed = true;
  if( deletion_candidates.size() > 1 )
  {
    int ret = QMessageBox::warning( this, "Delete Warning",
      "Are you sure you want to delete these measurements?",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No  );

    delete_confirmed = ( ret == QMessageBox::Yes );
  }

  if ( delete_confirmed )
  {
    // Delete selected/active measurements
    MeasurementTableModel* measurement_model = 
      dynamic_cast< MeasurementTableModel* >( this->model() );
    measurement_model->remove_rows( deletion_candidates );
  }
}

void MeasurementTableView::handle_selected()
{
  // Work around the fact that for some reason QItemSelectionModel::selectionChanged() passes
  // an empty list if you select a single row after having selected multiple rows.  Instead,
  // just pass the current selection.
  MeasurementTableModel* model = qobject_cast< MeasurementTableModel* >( this->model() );
  model->handle_selected( this->selectionModel()->selection() );
}

void MeasurementTableView::handle_header_clicked( int index )
{
  if( index == 0 )
  {
    MeasurementTableModel* model = qobject_cast< MeasurementTableModel* >( this->model() );
    model->toggle_visible();
  }
}

} // end namespace Seg3D
