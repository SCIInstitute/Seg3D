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

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tools/Actions/ActionSetMeasurementNote.h>
#include <Application/Tools/Actions/ActionSetMeasurementVisible.h>

// Core includes
#include <Core/State/Actions/ActionRemove.h>

namespace Seg3D
{

CORE_ENUM_CLASS 
(
  MeasurementVisibility,
  ALL_VISIBLE_E, 
  NONE_VISIBLE_E,
  SOME_VISIBLE_E
)

class MeasurementTableModelPrivate 
{
public:
  void set_active_index( int active_index );
  void update_visibility();

  //std::string cached_active_measurement_id_;
  std::string cached_active_note_;
  bool use_cached_active_note_;
  MeasurementToolHandle measurement_tool_;

  MeasurementTableModel * model_;

  int visibility_; 
};

void MeasurementTableModelPrivate::set_active_index( int active_index )
{
  // TODO: Lock state engine, get state directly
  // If this is already the active measurement, return
  if( active_index == this->model_->get_active_index() ) return;

  // Make sure index is in valid range
  int num_rows = this->model_->rowCount( QModelIndex() );
  if( active_index < 0 || active_index > num_rows - 1 ) return;

  // Set the active index in the measurement list
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
    this->measurement_tool_->active_index_state_, active_index );

  // Send signal to update note in text box
  // Can't use get_active_note() because active note is set on application thread asynchronously
  std::vector< Core::Measurement > measurements = this->measurement_tool_->get_measurements();
  if( active_index < static_cast< int >( measurements.size() ) )
  {
    QString active_note = QString::fromStdString( measurements[ active_index ].get_note() );
    Q_EMIT this->model_->active_note_changed( active_note ); 
  }

  // Send signal highlight only active row
  int num_columns = this->model_->columnCount( QModelIndex() );
}

void MeasurementTableModelPrivate::update_visibility()
{
  ASSERT_IS_INTERFACE_THREAD();

  std::vector< Core::Measurement > measurements = 
    this->measurement_tool_->get_measurements();
  size_t num_visible_measurements = 0;
  for( size_t i = 0; i < measurements.size(); i++ )
  {
    if( measurements[ i ].get_visible() )
    {
      num_visible_measurements++;
    }
  }
  if( num_visible_measurements == 0 )
  {
    this->visibility_ = MeasurementVisibility::NONE_VISIBLE_E;
  }
  else if( num_visible_measurements == measurements.size() )
  {
    this->visibility_ = MeasurementVisibility::ALL_VISIBLE_E;
  }
  else
  {
    this->visibility_ = MeasurementVisibility::SOME_VISIBLE_E;
  }
}

MeasurementTableModel::MeasurementTableModel( MeasurementToolHandle measurement_tool, 
  QObject* parent ) :
  QAbstractTableModel( parent ),
  private_( new MeasurementTableModelPrivate )
{
  this->private_->model_ = this;
  this->private_->measurement_tool_ = measurement_tool;
  //this->private_->cached_active_measurement_id_ = "";
  this->private_->cached_active_note_ = "";
  this->private_->use_cached_active_note_ = false;
  this->private_->visibility_ = MeasurementVisibility::ALL_VISIBLE_E;
}

MeasurementTableModel::~MeasurementTableModel()
{
  // Measurements vector gets destroyed automatically
}

int MeasurementTableModel::rowCount( const QModelIndex& /*index*/) const
{
  return static_cast< int >( this->private_->measurement_tool_->get_measurements().size() );
}

int MeasurementTableModel::columnCount( const QModelIndex& /*index*/) const
{
  return MeasurementColumns::NOTE_E + 1; 
}

QVariant MeasurementTableModel::data( const QModelIndex& index, int role ) const
{
  // TODO ASSERT_IS_INTERFACE_THREAD();

  if ( !index.isValid() ) return QVariant();

  if ( role == Qt::TextAlignmentRole )
  {
    return int( Qt::AlignLeft | Qt::AlignVCenter );
  }
  else if ( role == Qt::DecorationRole )
  {
    if ( index.column() == MeasurementColumns::VISIBLE_E ) 
    {
      std::vector< Core::Measurement > measurements = 
        this->private_->measurement_tool_->get_measurements();
      if( index.row() < static_cast< int >( measurements.size() ) )
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
      std::vector< Core::Measurement > measurements = 
        this->private_->measurement_tool_->get_measurements();
      if( index.row() < static_cast< int >( measurements.size() ) )
      {
        Core::Measurement measurement = measurements[ index.row() ];

        if ( index.column() == MeasurementColumns::LENGTH_E ) 
        {
          // Thread-safety: We get a handle to the active layer 
          // (get_active_layer is thread-safe), so it can't be deleted out from under 
          // us.
          LayerHandle active_layer = LayerManager::Instance()->get_active_layer();

          Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
          if( !this->private_->measurement_tool_->show_world_units_state_->get() && 
            active_layer ) 
          {
            // Index units
            
            // Convert world units to index units
            // Use grid transform from active layer
            Core::GridTransform grid_transform = active_layer->get_grid_transform();

            // Grid transfrom takes index coords to world coords, so we need inverse
            Core::Transform inverse_transform = grid_transform.get_inverse();

            Core::Point p0, p1;
            measurement.get_point( 0 , p0 );
            measurement.get_point( 1 , p1 );
            Core::Vector measure_vec = p1 - p0;
            measure_vec = inverse_transform.project( measure_vec );
            double index_length = measure_vec.length();

            // Use same formatting policy as status bar for coordinates
            if( 10000 < index_length ) 
            {
              // Use scientific notation
              return QString( "%1" ).arg( index_length, 0, 'e', 2 );
            }
            else 
            {
              // Format normally
              return  QString( "%1" ).arg( index_length, 0, 'f', 3 );
            }
          }
          else
          {
            // World units
            double world_length = measurement.get_length();

            // Use same formatting policy as status bar for coordinates
            if( ( 0.0 < world_length && world_length < 0.0001 ) || 1000 < world_length ) 
            {
              // Use scientific notation
              return QString( "%1" ).arg( world_length, 0, 'e', 2 );
            }
            else 
            {
              // Format normally
              return  QString( "%1" ).arg( world_length, 0, 'f', 3 );
            }
          }
        }
        else if ( index.column() == MeasurementColumns::NOTE_E ) 
        {
          if( index.row() == this->get_active_index() && 
            this->private_->use_cached_active_note_ )
          {
            return QString::fromStdString( this->private_->cached_active_note_ );
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
      return QBrush( QColor( 225, 243, 252 ) ); // Light blue
    }
  }
  return QVariant(); 
  
}

bool MeasurementTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  // Called only after enter is pressed or focus changed
  if ( index.isValid() && role == Qt::EditRole ) 
  { 
    std::vector< Core::Measurement > measurements = 
      this->private_->measurement_tool_->get_measurements();
    if( index.row() < static_cast< int >( measurements.size() ) ) 
    {
      Core::Measurement measurement = measurements[ index.row() ];
      if ( index.column() == MeasurementColumns::VISIBLE_E ) 
      {
        //measurement.set_visible( value.toBool() );
        //this->private_->measurement_tool_->set_measurement( index.row(), measurement );
        ActionSetMeasurementVisible::Dispatch( Core::Interface::GetWidgetActionContext(),
          this->private_->measurement_tool_->measurements_state_, measurement.get_id(), 
          value.toBool() );
      }
      else if( index.column() == MeasurementColumns::NOTE_E ) 
      {
        // Don't save note to state variable yet because we don't want to trigger a full update
        // for every character typed.  A full update interrupts editing so we're never
        // able to type more than one character before losing edit focus.  Instead, wait until 
        // the note editing is finished to save the state.  In the meantime save and use a 
        // cached copy of the note.
        this->private_->cached_active_note_ = value.toString().toStdString();
        if( !this->private_->use_cached_active_note_ )
        {
          //this->private_->cached_active_measurement_id_ = measurement.get_id();
          this->private_->use_cached_active_note_ = true;
        }

        // If we are editing this cell then it is by definition the active measurement
        Q_EMIT active_note_changed( value.toString() ); 
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

QVariant MeasurementTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if( role == Qt::DecorationRole )
  {
    if( orientation == Qt::Horizontal )
    {
      if( section == MeasurementColumns::VISIBLE_E )
      {
        if( this->private_->visibility_ == MeasurementVisibility::ALL_VISIBLE_E )
        {
          return QIcon( QString::fromUtf8( ":/Images/Visible.png" ) );
        }
        else if( this->private_->visibility_ == MeasurementVisibility::NONE_VISIBLE_E )
        {
          return QIcon( QString::fromUtf8( ":/Images/VisibleOff.png" ) );
        }
        else if( this->private_->visibility_ == MeasurementVisibility::SOME_VISIBLE_E )
        {
          return QIcon( QString::fromUtf8( ":/Images/VisibleGray.png" ) );
        }
      }
    }
  }
  else if ( role == Qt::DisplayRole )
  {   
    if( orientation == Qt::Vertical )
    {
      if( section < this->rowCount( QModelIndex() ) )
      {
        std::vector< Core::Measurement > measurements = 
          this->private_->measurement_tool_->get_measurements();
        if( section < static_cast< int >( measurements.size() ) )
        { 
          Core::Measurement measurement = measurements[ section ];
          return QString::fromStdString( measurement.get_id() );
        }   
      }
    }
    else if( orientation == Qt::Horizontal )
    {
      if ( section == MeasurementColumns::LENGTH_E ) 
      {
        return QString( "Length" );
      }
      else if ( section == MeasurementColumns::NOTE_E )
      {
        return QString( "Note" );
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags MeasurementTableModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );
  if ( index.column() == MeasurementColumns::NOTE_E ) // Only Note column is editable
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

bool MeasurementTableModel::removeRows( int row, int count, const QModelIndex & /* parent */ )
{
  if( row < 0 || row + count > this->rowCount( QModelIndex() ) ) return false;

  // Get all the measurements to be removed first, before indices change
  std::vector< Core::Measurement > measurements = 
    this->private_->measurement_tool_->get_measurements();
  std::vector< Core::Measurement > remove_measurements;
  for( int row_index = row; row_index < row + count; row_index++ )
  {
    if( row_index < static_cast< int >( measurements.size() ) )
    {
      remove_measurements.push_back( measurements[ row_index ] );
    }
  }

  // Remove rows from measurement list
  for( size_t i = 0; i < remove_measurements.size(); i++ )
  {
    // Remove is done on application thread -- no way to know if it succeeds
    Core::ActionRemove::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->private_->measurement_tool_->measurements_state_, remove_measurements[ i ] );
  }

  return true;
}


void MeasurementTableModel::update_table()
{
  this->private_->update_visibility();

  QAbstractTableModel::reset();

  // Needed for initial update after loading measurements from project file
  Q_EMIT active_note_changed( this->get_active_note() ); 
  this->private_->use_cached_active_note_ = false;
}

void MeasurementTableModel::update_cells()
{
  this->private_->update_visibility();

  int rows = this->rowCount( QModelIndex() );
  int columns = this->columnCount( QModelIndex() );
  QModelIndex top_left = this->index( 0, 0 );
  QModelIndex bottom_right = this->index( rows - 1, columns - 1 );
  Q_EMIT dataChanged( top_left, bottom_right );
  Q_EMIT headerDataChanged( Qt::Horizontal, 0, 0 );

  Q_EMIT active_note_changed( this->get_active_note() ); 
  this->private_->use_cached_active_note_ = false;
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

QString MeasurementTableModel::get_active_note() const
{
  // Lock state engine so that active index can't change between getting active index and list
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  int active_index = this->private_->measurement_tool_->active_index_state_->get();
  std::vector< Core::Measurement > measurements = 
    this->private_->measurement_tool_->measurements_state_->get();
  if( active_index != -1 && active_index < static_cast< int >( measurements.size() ) ) 
  {
    return QString::fromStdString( measurements[ active_index ].get_note() );
  }
  return "";
}

void MeasurementTableModel::set_active_note( const QString & note )
{
  int active_index = this->get_active_index();
  if( active_index == -1 ) return;
  QModelIndex index = this->index( active_index, MeasurementColumns::NOTE_E );
  this->setData( index, note, Qt::EditRole );

  // The note can only be modified on the Interface thread, so it is safe to update
  // the table view here.  Need to emit dataChanged so that table updates if 
  // user is editing note in text box.  Moved this out of setData() because we don't want to 
  // emit the signal if the note is being edited through the table since the cursor gets reset
  // to the end of the line.  
  Q_EMIT dataChanged( index, index ); 
}

int MeasurementTableModel::get_active_index() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->measurement_tool_->active_index_state_->get();
}

void MeasurementTableModel::handle_selected( const QItemSelection & selected )
{
  if( selected.isEmpty() ) return;

  this->private_->set_active_index( selected.indexes().back().row() );
}

void MeasurementTableModel::toggle_visible()
{
  // Run from interface thread
  bool visible = true;
  if( this->private_->visibility_ == MeasurementVisibility::ALL_VISIBLE_E )
  {
    visible = false;
  }
  else if( this->private_->visibility_ == MeasurementVisibility::NONE_VISIBLE_E || 
    this->private_->visibility_ == MeasurementVisibility::SOME_VISIBLE_E )
  {
    visible = true;
  }

  std::vector< Core::Measurement > measurements = 
    this->private_->measurement_tool_->get_measurements();
  for( size_t i = 0; i < measurements.size(); i++ )
  {
    ActionSetMeasurementVisible::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->private_->measurement_tool_->measurements_state_, measurements[ i ].get_id(), 
      visible );
  }
}

void MeasurementTableModel::handle_click( const QModelIndex & index )
{
  if ( !index.isValid() ) return;

  // If visible column was clicked, toggle visible
  if ( index.column() == MeasurementColumns::VISIBLE_E ) 
  {
    std::vector< Core::Measurement > measurements = 
      this->private_->measurement_tool_->get_measurements();
    if( index.row() < static_cast< int >( measurements.size() ) )
    { 
      // Toggle visible
      bool visible = !( measurements[ index.row() ].get_visible() ); 
      this->setData( index, visible, Qt::EditRole );
    }
  } 
}

void MeasurementTableModel::save_cached_active_note() 
{
  if( this->private_->use_cached_active_note_ )
  {
    // Lock state engine so measurements list doesn't change between getting active index
    // and getting list.
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    std::vector< Core::Measurement > measurements = 
      this->private_->measurement_tool_->measurements_state_->get();
    size_t active_index = this->private_->measurement_tool_->active_index_state_->get();

    if( 0 <= active_index && active_index < measurements.size() )
    {
      std::string measurement_id = measurements[ active_index ].get_id();
      ActionSetMeasurementNote::Dispatch( Core::Interface::GetWidgetActionContext(), 
        this->private_->measurement_tool_->measurements_state_,
        measurement_id, this->private_->cached_active_note_ );
    }
  }
}

} // end namespace Seg3D

