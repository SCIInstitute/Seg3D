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

#ifndef INTERFACE_TOOLINTERFACE_DETAIL_MEASUREMENTTABLEVIEW_H
#define INTERFACE_TOOLINTERFACE_DETAIL_MEASUREMENTTABLEVIEW_H

#ifndef Q_MOC_RUN

// Qt includes
#include <QTableView>

// Boost includes
#include <boost/shared_ptr.hpp>

#endif

namespace Seg3D
{

// Forward declarations
class MeasurementTableModel;

// Hide header includes, private interface and implementation
class MeasurementTableViewPrivate;
typedef boost::shared_ptr< MeasurementTableViewPrivate > MeasurementTableViewPrivateHandle;

// QTableView with support for copyable measurements
class MeasurementTableView : public QTableView 
{ 
  Q_OBJECT
public: 
  MeasurementTableView( QWidget* parent );

  //
  // Extended functions
  //

  // SET_MEASUREMENT_MODEL:
  // Set the measurement model.  Can't be set in constructor because table view is constructed
  // by GUI prior to model creation.
  void set_measurement_model( MeasurementTableModel* measurement_model );

  // DELETE_SELECTED_MEASUREMENTS:
  // Delete all measurements that are selected or active measurement if none are selected. 
  void delete_selected_measurements();

  // COPY_SELECTED_CELLS:
  // Copy selected cells to clipboard, or active measurement if no cells are selected.  
  // Copied cells can be pasted directly into Excel spreadsheet.
  void export_selected_measurements() const;

  // UPDATE_ACTIVE_INDEX:
  // Select active index and ensure that active index is visible in table.
  // Locks: StateEngine
  void update_active_index();

private Q_SLOTS:

  // HANDLE_MODEL_RESET:
  // Locks: StateEngine
  void handle_model_reset();

  void handle_selected();
  void handle_header_clicked( int index );

private:
  MeasurementTableViewPrivateHandle private_;
}; 

} // end namespace Seg3D

#endif 
