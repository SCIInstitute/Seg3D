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
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QIcon>

// Interface includes
#include <Interface/ToolInterface/detail/MeasurementTableModel.h>

namespace Seg3D
{

MeasurementTableModel::MeasurementTableModel( MeasurementToolHandle measurement_tool, 
  QObject* parent ) :
  QAbstractTableModel( parent ),
  measurement_tool_( measurement_tool ),
  cached_active_note_( "" ),
  use_cached_active_note_( false )
{
}

MeasurementTableModel::~MeasurementTableModel()
{
  // Measurements vector gets destroyed automatically
}

int MeasurementTableModel::rowCount( const QModelIndex& /*index*/) const
{
  return static_cast< int >( this->measurement_tool_->get_measurements().size() );
}

int MeasurementTableModel::columnCount( const QModelIndex& /*index*/) const
{
  return MEASUREMENT_NOTE_E + 1; 
}

QVariant MeasurementTableModel::data( const QModelIndex& index, int role ) const
{
  if ( !index.isValid() ) return QVariant();

  if ( role == Qt::TextAlignmentRole )
  {
    return int( Qt::AlignLeft | Qt::AlignVCenter );
  }
  else if ( role == Qt::DecorationRole )
  {
    if ( index.column() == MEASUREMENT_VISIBLE_E ) 
    {
      std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
      if( index.row() < measurements.size() )
      {
        if( measurements[ index.row() ].get_visible() )
        {
          return QIcon( QString::fromUtf8( ":/Images/Visible.png" ) );
        }
        else
        { 
          return QIcon( QString::fromUtf8( ":/Images/VisibleOff.png" ) );
        }
      }
    }
  }
  else if ( role == Qt::DisplayRole )
  {
    int sz = this->rowCount( index );
    if ( index.row() < sz )
    {
      std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
      if( index.row() < measurements.size() )
      {
        Core::Measurement measurement = measurements[ index.row() ];
        if ( index.column() == MEASUREMENT_LENGTH_E ) 
        {
          return QString( "%1 mm" ).arg( measurement.get_length(), 0, 'f', 3 );
        }
        else if ( index.column() == MEASUREMENT_NOTE_E ) 
        {
          if( index.row() == this->get_active_index() && this->use_cached_active_note_ )
          {
            return QString::fromStdString( this->cached_active_note_ );
          }
          else
          {
            return QString::fromStdString( measurement.get_note() );
          }
          
        }
      }
    }
  }
  else if ( role == Qt::BackgroundRole )
  {
    if ( index.row() == this->get_active_index() )
    {
      //return QBrush( QColor( 193, 228, 248 ) ); // Medium blue
      return QBrush( QColor( 225, 243, 252 ) ); // Light blue
    }
  }
  return QVariant(); 
  
}

QVariant MeasurementTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role != Qt::DisplayRole )
  {
    return QVariant();
  }

  if( orientation == Qt::Vertical )
  {
    if( section < this->rowCount( QModelIndex() ) )
    {
      std::vector< Core::Measurement > measurements = 
        this->measurement_tool_->get_measurements();
      if( section < measurements.size() )
      { 
        Core::Measurement measurement = measurements[ section ];
        return QString::fromStdString( measurement.get_label() );
      }
      
    }
  }

  if ( section == MEASUREMENT_LENGTH_E ) 
  {
    return QString( "Length" );
  }
  else if ( section == MEASUREMENT_NOTE_E )
  {
    return QString( "Note" );
  }
  return QVariant();
}

Qt::ItemFlags MeasurementTableModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );
  if ( index.column() == MEASUREMENT_NOTE_E ) // Only Note column is editable
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

bool MeasurementTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  // Called only after enter is pressed or focus changed
  if ( index.isValid() && role == Qt::EditRole ) 
  { 
    std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
    if( index.row() < measurements.size() ) 
    {
      Core::Measurement measurement = measurements[ index.row() ];
      if ( index.column() == MEASUREMENT_VISIBLE_E ) 
      {
        measurement.set_visible( value.toBool() );
        this->measurement_tool_->set_measurement( index.row(), measurement );
      }
      else if( index.column() == MEASUREMENT_NOTE_E ) 
      {
        // Don't save note to state variable yet because we don't want to trigger a full update
        // for every character typed.  A full update interrupts editing so we're never
        // able to type more than one character before losing edit focus.  Instead, wait until 
        // the note editing is finished to save the state.  In the meantime save and use a 
        // cached copy of the note.
        this->cached_active_note_ = value.toString().toStdString();
        this->use_cached_active_note_ = true;

        // If we are editing this cell then it is by definition the active measurement
        Q_EMIT active_note_changed( value.toString() ); 

        // The note can only be modified on the Interface thread, so it is safe to update
        // the table view here.  Need to emit dataChanged so that table updates if 
        // user is editing note in text box. 
        Q_EMIT dataChanged( index, index ); 
      }
      else
      {
        return false;
      } 
      return true;
    }
  }
  return false;
}

void MeasurementTableModel::set_active_note( const QString & note )
{
  int active_index = this->get_active_index();
  if( active_index == MeasurementTool::INVALID_ACTIVE_INDEX_C ) return;
  QModelIndex index = this->index( active_index, MEASUREMENT_NOTE_E );
  this->setData( index, note, Qt::EditRole );
}

void MeasurementTableModel::handle_click( const QModelIndex & index )
{
  if ( !index.isValid() ) return;
  
  // If visible column was clicked, toggle visible
  if ( index.column() == MEASUREMENT_VISIBLE_E ) 
  {
    std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
    if( index.row() < measurements.size() )
    { 
      // Toggle visible
      bool visible = !( measurements[ index.row() ].get_visible() ); 
      this->setData( index, visible, Qt::EditRole );
    }
  } 
}

void MeasurementTableModel::update()
{
  QAbstractTableModel::reset();
}

QString MeasurementTableModel::get_active_note() const
{
  int active_index = this->get_active_index();
  std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
  if( active_index != MeasurementTool::INVALID_ACTIVE_INDEX_C && 
    active_index < measurements.size() ) 
  {
    return QString::fromStdString( measurements[ active_index ].get_note() );
  }
  return "";
}

int MeasurementTableModel::get_active_index() const
{
  return this->measurement_tool_->get_active_index();
  return 0;
}

bool MeasurementTableModel::removeRows( int row, int count, const QModelIndex & /* parent */ )
{
  if( row < 0 || row + count > this->rowCount( QModelIndex() ) ) return false;

  // Get all the measurements to be removed first, before indices change
  std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
  std::vector< Core::Measurement > remove_measurements;
  for( int row_index = row; row_index < row + count; row_index++ )
  {
    if( row_index < measurements.size() )
    {
      remove_measurements.push_back( measurements[ row_index ] );
    }
  }

  // Remove rows from measurement list
  bool ok = true;
  for( size_t i = 0; i < remove_measurements.size(); i++ )
  {
    ok = ok && this->measurement_tool_->remove_measurement( remove_measurements[ i ] );
  }

  return ok;
}

void MeasurementTableModel::set_active_index( int active_index )
{
  // If this is already the active measurement, return
  if( active_index == this->get_active_index() ) return;

  // Make sure index is in valid range
  int num_rows = this->rowCount( QModelIndex() );
  if( active_index < 0 || active_index > num_rows - 1 ) return;

  int old_active_index = this->get_active_index();
  int new_active_index = active_index;

  // Set the active index in the measurement list
  this->measurement_tool_->set_active_index( new_active_index );

  // Send signal to update note in text box
  // Can't use get_active_note() because active note is set on application thread asynchronously
  std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
  if( new_active_index < measurements.size() )
  {
    QString active_note = QString::fromStdString( measurements[ new_active_index ].get_note() );
    Q_EMIT active_note_changed( active_note ); 
  }

  // Send signal highlight only active row
  int num_columns = this->columnCount( QModelIndex() );
//  Q_EMIT dataChanged( this->index( old_active_index, 0 ), 
//    this->index( old_active_index, num_columns - 1 ) );
//  Q_EMIT dataChanged( this->index( new_active_index, 0 ), 
//    this->index( new_active_index, num_columns - 1 ) );
}

void MeasurementTableModel::handle_selected( const QItemSelection & selected )
{
  if( selected.isEmpty() ) return;

  this->set_active_index( selected.indexes().back().row() );
}

void MeasurementTableModel::remove_rows( const std::vector< int >& rows )
{
  // Copy the vector (it should be small)
  std::vector< int > sorted_rows = rows;

  // Sort rows in ascending order
  std::sort( sorted_rows.begin(), sorted_rows.end() );
  
  // Must delete in reverse since row numbers are adjusted during removal
  std::vector< int >::reverse_iterator reverse_iter;
  for ( reverse_iter = sorted_rows.rbegin(); reverse_iter != sorted_rows.rend(); ++reverse_iter )  
  {
    this->removeRow( *reverse_iter );
  }
}

void MeasurementTableModel::save_active_note() 
{
  int active_index = this->get_active_index();
  std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
  if( active_index != MeasurementTool::INVALID_ACTIVE_INDEX_C && 
    active_index < measurements.size() )
  {
    Core::Measurement measurement = measurements[ active_index ];
    measurement.set_note( this->cached_active_note_ );
    this->measurement_tool_->set_measurement( active_index, measurement );
  }
  
  this->use_cached_active_note_ = false;
}

} // end namespace Seg3D