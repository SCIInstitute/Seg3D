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

// Qt includes
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>

// Interface includes
#include <Interface/ToolInterface/detail/MeasurementTableModel.h>
#include <Interface/ToolInterface/detail/MeasurementTableView.h>
#include <Interface/ToolInterface/detail/TextDelegate.h>

namespace Seg3D
{

MeasurementTableView::MeasurementTableView( QWidget* parent ) : 
  QTableView( parent )
{
  this->setItemDelegate( new TextDelegate( MEASUREMENT_NOTE_E ) ); // Custom text editor for note column
  this->horizontalHeader()->setStretchLastSection( true ); // Stretch note section

  this->delete_action_ = new QAction( tr( "&Delete" ), this );
  
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
    measurement_model, SLOT( handle_selected( QItemSelection) ) );
  QObject::connect( measurement_model, SIGNAL( rowsRemoved( QModelIndex, int, int ) ), 
    this, SLOT( scroll_to_active_index() ) );
  QObject::connect( measurement_model, SIGNAL( rowsInserted( QModelIndex, int, int ) ), 
    this, SLOT( scroll_to_active_index() ) );

  // Wait until text editing is finished to save the note for the active measurement.  This 
  // way we avoid updating the model for every keystroke.
  QAbstractItemDelegate* text_delegate = this->itemDelegate();
  QObject::connect( text_delegate, SIGNAL( closeEditor( QWidget* ) ), measurement_model, 
    SLOT( save_active_note() ) );
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

  // Have to resize rows/columns *after* model has been populated
  this->resizeColumnsToContents();
  this->resizeRowsToContents();
  
  this->horizontalHeader()->setStretchLastSection( true ); // Stretch note section

  // Scroll to active measurement
  this->scroll_to_active_index();
}

void MeasurementTableView::scroll_to_active_index()
{
  /*MeasurementTableModel* model = 
    qobject_cast< MeasurementTableModel* >( this->model() );
  int active_index = model->get_active_index();
  if( active_index != Core::MeasurementList::INVALID_ACTIVE_INDEX_C )
  {
    this->scrollTo( model->index( active_index, 0 ) );
  }*/
}

void MeasurementTableView::get_deletion_candidates( std::vector< int >& deletion_candidates ) const
{
  MeasurementTableModel* model = qobject_cast< MeasurementTableModel* >( this->model() );
  if( model->rowCount( QModelIndex() ) == 0 ) return;

  // Get selected indices from QTableView
  QModelIndexList selected_rows_list = this->selectionModel()->selectedRows();

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
    int active_index = model->get_active_index();
    if( active_index != MeasurementTool::INVALID_ACTIVE_INDEX_C )
    {
      deletion_candidates.push_back( active_index );
    }
  }
}

void MeasurementTableView::copy() const
{
  QItemSelectionModel* selection = this->selectionModel(); 
  QModelIndexList indexes = selection->selectedIndexes();

  if( indexes.size() < 1 ) return; 

  // QModelIndex::operator < sorts first by row, then by column.  
  // this is what we need 
  std::sort( indexes.begin(), indexes.end() ); 

  // You need a pair of indexes to find the row changes 
  QModelIndex previous = indexes.first(); 
  indexes.removeFirst(); 
  QString selected_text; 
  QModelIndex current; 
  Q_FOREACH( current, indexes ) 
  { 
    // If this is the first column and this row is selected, append the row header to the string
    if ( previous.column() == 0 )
    {
      if( selection->isRowSelected( previous.row(), QModelIndex() ) )
      {
        QString header_text = 
          this->model()->headerData( previous.row(), Qt::Vertical ).toString();
        selected_text.append( header_text );
      }
    }

    QVariant data = this->model()->data( previous ); 
    QString text = data.toString(); 
    // At this point `text` contains the text in one cell 
    selected_text.append( text ); 
    // If you are at the start of the row the row number of the previous index 
    // isn't the same.  Text is followed by a row separator, which is a newline. 
    if ( current.row() != previous.row() ) 
    { 
      selected_text.append( QLatin1Char('\n') ); 
    } 
    // Otherwise it's the same row, so append a column separator, which is a tab. 
    else 
    { 
      selected_text.append( QLatin1Char('\t') ); 
    } 
    previous = current; 
  } 

  // add last element 
  selected_text.append( model()->data( previous ).toString() ); 
  selected_text.append( QLatin1Char('\n') ); 
  qApp->clipboard()->setText( selected_text );
}

void MeasurementTableView::delete_selected_measurements()
{
  std::vector< int > deletion_candidates;
  this->get_deletion_candidates( deletion_candidates );
  if( deletion_candidates.size() == 0 ) 
  {
    return;
  }

  /*
  QString measurement_list = this->build_measurement_string( deletion_candidates );

  bool delete_confirmed = true;
  if( this->delete_measure_dialog_->always_show() )
  {
    // Launch dialog allowing the user to confirm/cancel deletion
    this->delete_measure_dialog_->set_measurement_list( measurement_list );
    this->delete_measure_dialog_->exec();

    delete_confirmed = ( this->delete_measure_dialog_->result() == QDialog::Accepted );
  }

  if ( delete_confirmed )
  {
    // Delete selected/active measurements
    this->table_model_->remove_rows( deletion_candidates );
  }
  */
  // Delete selected/active measurements
  MeasurementTableModel* measurement_model = 
    dynamic_cast< MeasurementTableModel* >( this->model() );
  measurement_model->remove_rows( deletion_candidates );
}

void MeasurementScrollBar::wheelEvent( QWheelEvent * e )
{
  QScrollBar::wheelEvent( e ); 
  e->accept();
}

} // end namespace Seg3D