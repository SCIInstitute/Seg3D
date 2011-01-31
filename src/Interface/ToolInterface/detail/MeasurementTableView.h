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

#ifndef INTERFACE_TOOLINTERFACE_MEASUREMENTTABLEVIEW_H
#define INTERFACE_TOOLINTERFACE_MEASUREMENTTABLEVIEW_H

// Qt includes
#include <QtGui/QHeaderView>
#include <QtGui/QScrollBar>

// Interface includes
#include <Interface/ToolInterface/detail/CopyTableView.h>

namespace Seg3D
{

class DeleteMeasurementDialog;

// QTableView with support for copyable measurements
class MeasurementTableView : public CopyTableView
{ 
  Q_OBJECT
public: 
  MeasurementTableView( QWidget* parent );

  //
  // Extended functions
  //

  void get_deletion_candidates( std::vector< int >& deletion_candidates ) const;  

Q_SIGNALS:
  void delete_table_measurements();

private Q_SLOTS:
  void handle_model_reset();
  void scroll_to_active_index();

private:
  QAction* delete_action_;
}; 

// Derived scroll bar that accepts all wheelEvent events rather than passing them on to the 
// parent when scrollbar is at min/max.
class MeasurementScrollBar : public QScrollBar
{
public:
  MeasurementScrollBar( QWidget * parent = 0 ) :
    QScrollBar( parent ) {}

  void wheelEvent( QWheelEvent * e );
};

} // end namespace Seg3D

#endif 
