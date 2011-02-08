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

#ifndef INTERFACE_TOOLINTERFACE_MEASUREMENTSTABLEMODEL_H
#define INTERFACE_TOOLINTERFACE_MEASUREMENTSTABLEMODEL_H

// QT Includes
#include <QtCore/QAbstractTableModel>
#include <QtCore/QModelIndex>
#include <QtGui/QItemSelection>

// Application includes
#include <Application/Tools/MeasurementTool.h>

namespace Seg3D
{

enum MeasurementColumns
{
  MEASUREMENT_VISIBLE_E, 
  MEASUREMENT_LENGTH_E,
  MEASUREMENT_NOTE_E
};

class MeasurementTableModel : public QAbstractTableModel
{
  Q_OBJECT

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

  // UPDATE:
  // Update entire table.  Stops any editing that the user may be doing on the table.
  void update();

  void remove_rows( const std::vector< int >& rows );
  
  QString get_active_note() const;
  void set_active_note( const QString & note );
  // Returns -1 if there are no measurements
  int get_active_index() const; // Public because QTableView needs to scroll to this
  
Q_SIGNALS:
  void active_note_changed( const QString & note ) const;

private Q_SLOTS:
  void handle_click( const QModelIndex & index );
  void handle_selected( const QItemSelection & selected );
  void save_active_note();

private:
  void set_active_index( int active_index );

  std::string cached_active_note_;
  bool use_cached_active_note_;
  MeasurementToolHandle measurement_tool_;
};

} // end namespace Seg3D

#endif 
