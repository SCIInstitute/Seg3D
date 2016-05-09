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

#ifndef INTERFACE_APPLICATION_PROVENANCETREEMODEL_H
#define INTERFACE_APPLICATION_PROVENANCETREEMODEL_H

#ifndef Q_MOC_RUN

// Qt includes
#include <QAbstractItemModel>

// Application includes
#include <Application/Provenance/ProvenanceStep.h>

#endif

namespace Seg3D
{
  
class ProvenanceTreeModelPrivate;

class ProvenanceTreeModel : public QAbstractItemModel
{
  Q_OBJECT
  
public:
  ProvenanceTreeModel( QObject *parent = 0 );
  virtual ~ProvenanceTreeModel();

public:
  enum data_role
  {
    PID_OF_INTEREST_E = Qt::UserRole,
    ACTION_PARAMS_E,
    PROV_STEP_INDEX_E,
    TIMESTAMP_E,
    USER_ID_E,
    ACTION_NAME_E
  };

public:
  void set_provenance_trail( ProvenanceTrailHandle prov_trail );

  // -- Reimplemented virtual functions
public:
  virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const;
  virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
  virtual QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
  virtual QModelIndex parent( const QModelIndex& index ) const;
  virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
  
  virtual QVariant headerData( int section, Qt::Orientation orientation, 
    int role = Qt::DisplayRole ) const;

private:
  ProvenanceTreeModelPrivate* private_;
};
  
} // end namespace Seg3D

#endif 
