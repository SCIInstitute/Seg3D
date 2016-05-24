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

// Qt includes
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QIcon>

// Core includes
#include <Core/State/Actions/ActionRemove.h>
#include <Core/State/Actions/ActionSetAt.h>
#include <Core/Interface/Interface.h>

// Interface includes
#include <Interface/Application/StyleSheet.h>
#include <Interface/ToolInterface/detail/MeasurementTableModel.h>

// Application includes
#include <Application/Layer/LayerManager.h>

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

  std::string cached_active_name_;
  bool use_cached_active_name_;
  MeasurementToolHandle measurement_tool_;

  MeasurementTableModel * model_;

  int visibility_; 
};

void MeasurementTableModelPrivate::set_active_index( int active_index )
{
  // Ensure that state won't change between getting active index and measurements
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  // NOTE: It is possible that some other thread could change the measurements vector after
  // we get it but before the ActionSet is processed, thereby invalidating the active index that
  // is being set and the validation checks that are done below.  However, we have decided that 
  // the interface will be disabled while scripts run, so we won't worry about this case.

  // If this is already the active measurement, return
  if( active_index == this->measurement_tool_->active_index_state_->get() ) return;

  // Active index has changed, so don't want to use cached name for old active index
  this->use_cached_active_name_ = false;

  // Make sure index is in valid range
  const std::vector< Core::Measurement >& measurements = 
    this->measurement_tool_->measurements_state_->get();
  if( active_index < 0 || active_index >= static_cast< int >( measurements.size() ) ) return;

  // Set the active index in the measurement list
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
    this->measurement_tool_->active_index_state_, active_index );
}

void MeasurementTableModelPrivate::update_visibility()
{
  ASSERT_IS_INTERFACE_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::vector< Core::Measurement >& measurements = 
    this->measurement_tool_->measurements_state_->get();
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
  this->private_->cached_active_name_ = "";
  this->private_->use_cached_active_name_ = false;
  this->private_->visibility_ = MeasurementVisibility::ALL_VISIBLE_E;
}

MeasurementTableModel::~MeasurementTableModel()
{
  // Measurements vector gets destroyed automatically
}

int MeasurementTableModel::rowCount( const QModelIndex& /*index*/) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return static_cast< int >( this->private_->measurement_tool_->measurements_state_->get().size() );
}

int MeasurementTableModel::columnCount( const QModelIndex& /*index*/) const
{
  return MeasurementColumns::COMMENT_E + 1; 
}

QVariant MeasurementTableModel::data( const QModelIndex& index, int role ) const
{
  ASSERT_IS_INTERFACE_THREAD();

  if ( !index.isValid() ) return QVariant();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::vector< Core::Measurement >& measurements = 
    this->private_->measurement_tool_->measurements_state_->get();

  if ( role == Qt::TextAlignmentRole )
  {
    return int( Qt::AlignLeft | Qt::AlignVCenter );
  }
  else if ( role == Qt::DecorationRole )
  {
    if ( index.column() == MeasurementColumns::VISIBLE_E ) 
    {
      if( index.row() < static_cast< int >( measurements.size() ) )
      {
        if( measurements[ index.row() ].get_visible() )
        {
          return QIcon( QString::fromUtf8( ":/Images/VisibleWhite.png" ) );
        }
        else
        { 
          return QIcon( QString::fromUtf8( ":/Images/VisibleOff.png" ) );
        }
      }
    }
    else if ( index.column() == MeasurementColumns::COLOR_E ) 
    {
      Core::Color color;
      measurements[ index.row() ].get_color( color );
      return QColor( color.r() * 255, color.g() * 255, color.b() * 255 );
    }
  }
  else if ( role == Qt::DisplayRole )
  {
    int sz = this->rowCount( index );
    if ( index.row() < sz )
    {
      if( index.row() < static_cast< int >( measurements.size() ) )
      {
        
        if ( index.column() == MeasurementColumns::NAME_E ) 
        {
          if( index.row() == this->private_->measurement_tool_->active_index_state_->get()
            && this->private_->use_cached_active_name_ )
          {
            return QString::fromStdString( this->private_->cached_active_name_ );
          }
          else
          {
            return QString::fromStdString( measurements[ index.row() ].get_name() );
          }
        }
        else if ( index.column() == MeasurementColumns::LENGTH_E ) 
        {
          double world_length = measurements[ index.row() ].get_length();
          return QString::fromStdString( 
            this->private_->measurement_tool_->convert_world_to_unit_string( world_length ) );
        }
        else if ( index.column() == MeasurementColumns::COMMENT_E ) // Used only for export
        {
          return QString::fromStdString( measurements[ index.row() ].get_comment() );
        }
      }
    }
  }
  else if ( role == Qt::BackgroundRole )
  {
    if ( index.row() == this->private_->measurement_tool_->active_index_state_->get() )
    {
      // The active index is always selected, but if one column is highlighted it may not be
      // obvious which is the active index, so color it.
      return QBrush( QColor( StyleSheet::ACCENT_COLOR_C ) );
    }
  }
  return QVariant(); 
  
}

bool MeasurementTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  // Called only after enter is pressed or focus changed
  if ( index.isValid() && role == Qt::EditRole ) 
  { 
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    const std::vector< Core::Measurement >& measurements = 
      this->private_->measurement_tool_->measurements_state_->get();

    if( index.row() < static_cast< int >( measurements.size() ) ) 
    {
      if ( index.column() == MeasurementColumns::VISIBLE_E ) 
      {
        Core::Measurement m = measurements[ index.row() ];
        m.set_visible( value.toBool() );
        Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
          this->private_->measurement_tool_->measurements_state_, index.row(), m );
      }
      if ( index.column() == MeasurementColumns::COLOR_E ) 
      {
        Core::Measurement m = measurements[ index.row() ];
        QColor color = value.value< QColor >();
        Core::Color m_color( color.red() / 255.0f, color.green() / 255.0f, 
          color.blue() / 255.0f );
        m.set_color( m_color );
        Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
          this->private_->measurement_tool_->measurements_state_, index.row(), m );
      }
      else if( index.column() == MeasurementColumns::NAME_E ) 
      {
        // Don't save name to state variable yet because we don't want to trigger a full update
        // for every character typed.  A full update interrupts editing so we're never
        // able to type more than one character before losing edit focus.  Instead, wait until 
        // the name editing is finished to save the state.  In the meantime save and use a 
        // cached copy of the name.
        this->private_->cached_active_name_ = value.toString().toStdString();
        this->private_->use_cached_active_name_ = true;
      }
      else if( index.column() == MeasurementColumns::LENGTH_E ) 
      { 
        // Convert length to world coordinates before setting length in measurement
        std::string unit_length_str = value.toString().toStdString();
        double world_length = 
          this->private_->measurement_tool_->convert_unit_string_to_world( unit_length_str );

        // Set length in measurement state vector
        Core::Measurement m = measurements[ index.row() ];
        m.set_length( world_length );
        Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
          this->private_->measurement_tool_->measurements_state_, index.row(), m );
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
          return QIcon( QString::fromUtf8( ":/Images/VisibleWhite.png" ) );
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
      else if( section == MeasurementColumns::COLOR_E )
      {
        return QIcon( QString::fromUtf8( ":/Palette_Icons/brush_01.png" ) );
      }
    }
  }
  else if ( role == Qt::DisplayRole )
  {   
    if( orientation == Qt::Vertical )
    {
      if( section < this->rowCount( QModelIndex() ) )
      {
        return section;
      }
    }
    else if( orientation == Qt::Horizontal )
    {
      if ( section == MeasurementColumns::NAME_E )
      {
        return QString( "Name" );
      }
      else if ( section == MeasurementColumns::LENGTH_E ) 
      {
        return QString( "Length" );
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags MeasurementTableModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );
  if ( index.column() == MeasurementColumns::COLOR_E ||  // TODO: Fix color dialog position on Windows
    index.column() == MeasurementColumns::NAME_E || 
    index.column() == MeasurementColumns::LENGTH_E ) 
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

bool MeasurementTableModel::removeRows( int row, int count, const QModelIndex & /* parent */ )
{
  // Lock StateEngine here so that measurements vector doesn't change after rowCount check
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  if( row < 0 || row + count > this->rowCount( QModelIndex() ) ) return false;

  // Get all the measurements to be removed first, before indices change
  const std::vector< Core::Measurement >& measurements = 
    this->private_->measurement_tool_->measurements_state_->get();
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
  this->private_->use_cached_active_name_ = false;

  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();

  // Column resize doesn't work properly without this call
  this->update_cells();
}

void MeasurementTableModel::update_cells()
{
  this->private_->update_visibility();
  this->private_->use_cached_active_name_ = false;

  int rows = this->rowCount( QModelIndex() );
  int columns = this->columnCount( QModelIndex() );
  QModelIndex top_left = this->index( 0, 0 );
  QModelIndex bottom_right = this->index( rows - 1, columns - 1 );
  Q_EMIT dataChanged( top_left, bottom_right );
  Q_EMIT headerDataChanged( Qt::Horizontal, 0, 0 );
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

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::vector< Core::Measurement >& measurements = 
    this->private_->measurement_tool_->measurements_state_->get();
  for( size_t i = 0; i < measurements.size(); i++ )
  {
    Core::Measurement m = measurements[ i ];
    m.set_visible( visible );
    Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->measurement_tool_->measurements_state_, i, m );
  }
}

void MeasurementTableModel::handle_click( const QModelIndex & index )
{
  if ( !index.isValid() ) return;

  // If visible column was clicked, toggle visible
  if ( index.column() == MeasurementColumns::VISIBLE_E ) 
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    const std::vector< Core::Measurement >& measurements = 
      this->private_->measurement_tool_->measurements_state_->get();
    if( index.row() < static_cast< int >( measurements.size() ) )
    { 
      // Toggle visible
      bool visible = !( measurements[ index.row() ].get_visible() ); 
      this->setData( index, visible, Qt::EditRole );
    }
  } 
}

void MeasurementTableModel::save_cached_active_name() 
{
  if( this->private_->use_cached_active_name_ )
  {
    // Lock state engine so measurements list doesn't change between getting active index
    // and getting list.
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    const std::vector< Core::Measurement >& measurements = 
      this->private_->measurement_tool_->measurements_state_->get();
    int active_index = this->private_->measurement_tool_->active_index_state_->get();

    if( 0 <= active_index && active_index < static_cast< int >( measurements.size() ) )
    {
      Core::Measurement m = measurements[ active_index ];
      m.set_name( this->private_->cached_active_name_ );
      Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
        this->private_->measurement_tool_->measurements_state_, active_index, m );
    }
  }
}

} // end namespace Seg3D

