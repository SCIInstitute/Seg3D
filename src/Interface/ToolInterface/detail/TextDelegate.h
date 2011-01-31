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

#ifndef INTERFACE_TOOLINTERFACE_TEXTDELEGATE_H
#define INTERFACE_TOOLINTERFACE_TEXTDELEGATE_H

// Qt includes
#include <QtGui/QItemDelegate>

namespace Seg3D
{

class TextDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  TextDelegate( int text_column, QObject * parent = 0 );

  void paint( QPainter *painter, const QStyleOptionViewItem &option, 
    const QModelIndex &index ) const;
  QWidget* createEditor( QWidget *parent, const QStyleOptionViewItem &option, 
    const QModelIndex &index ) const;
  void setEditorData( QWidget *editor, const QModelIndex &index ) const ;
  void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

private Q_SLOTS:
  void commit_editor();

private:
  int text_column_;
};

} // end namespace Seg3D

#endif 
