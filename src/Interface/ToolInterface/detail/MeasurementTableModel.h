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

#ifndef INTERFACE_TOOLINTERFACE_DETAIL_MEASUREMENTTABLEMODEL_H
#define INTERFACE_TOOLINTERFACE_DETAIL_MEASUREMENTTABLEMODEL_H

#ifndef Q_MOC_RUN

// QT Includes
#include <QtCore/QAbstractTableModel>
#include <QtCore/QModelIndex>
#include <QItemSelection>

// Application includes
#include <Application/Tools/MeasurementTool.h>

// Core includes
#include <Core/Utils/EnumClass.h>

#endif

namespace Seg3D
{

CORE_ENUM_CLASS 
(
  MeasurementColumns,
  VISIBLE_E, 
  COLOR_E,
  NAME_E,
  LENGTH_E,
  COMMENT_E
)

// Hide header includes, private interface and implementation
class MeasurementTableModelPrivate;
typedef boost::shared_ptr< MeasurementTableModelPrivate > MeasurementTableModelPrivateHandle;

class MeasurementTableModel : public QAbstractTableModel
{
  Q_OBJECT

  friend class MeasurementTableModelPrivate;

public:
  MeasurementTableModel( MeasurementToolHandle measurement_tool, QObject* parent = 0 );

  virtual ~MeasurementTableModel();

  //
  // Inherited functions
  //

  int rowCount( const QModelIndex &index ) const;
  int columnCount( const QModelIndex &index ) const;

  QVariant data( const QModelIndex& index, int role ) const;
  bool setData( const QModelIndex &index, const QVariant &value, int role );
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  Qt::ItemFlags flags( const QModelIndex &index ) const;
  bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() ); 
  
  //
  // Extended functions
  //

  // UPDATE_TABLE:
  // Update entire table including dimensions.  Scroll to active index.  
  // Stops any editing that the user may be doing on the table.  Clears currently selected cells.
  void update_table();

  // UPDATE_CELLS:
  // Update only table cells, not table dimensions.
  void update_cells();

  // REMOVE_ROWS:
  // Remove measurements with specified row indices.
  void remove_rows( const std::vector< int >& rows );

  // GET_ACTIVE_INDEX:
  // Return row index of active measurement. Returns -1 if there are no measurements.
  // Needed by MeasurementTableView, which doesn't have direct access to MeasurementTool.
  // Locks: StateEngine
  int get_active_index() const; 
  
  // HANDLE_SELECTED:
  // Handler for when table cells are selected.
  void handle_selected( const QItemSelection & selected );

  // TOGGLE_VISIBLE:
  // Toggle tri-state button for visibility in horizontal header
  void toggle_visible();

private Q_SLOTS:

  // HANDLE_CLICK:
  // Handler for when table cells are clicked.
  void handle_click( const QModelIndex & index );

  // SAVE_ACTIVE_NOTE:
  // Save cached active note to state vector.  Avoids saving to state vector and triggering
  // updates on every keystroke.
  void save_cached_active_name();

private:
  MeasurementTableModelPrivateHandle private_;
};

} // end namespace Seg3D

#endif 
