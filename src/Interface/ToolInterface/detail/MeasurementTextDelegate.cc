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
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>

// Interface includes
#include <Interface/ToolInterface/detail/MeasurementTextDelegate.h>

namespace Seg3D
{

MeasurementTextDelegate::MeasurementTextDelegate( int text_column, QObject * parent /*= 0 */ ) : 
  text_column_( text_column ),
  QItemDelegate( parent ) {}

void MeasurementTextDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, 
  const QModelIndex &index ) const
{
  if ( index.column() == text_column_ )
  { 
    QString text = index.model()->data( index, Qt::DisplayRole ).toString();
    painter->save();
    painter->setBackgroundMode( Qt::OpaqueMode );
    painter->setBackground( index.model()->data( index, Qt::BackgroundRole ).value<QBrush>() );
    drawBackground( painter, option, index );
    painter->restore();
    drawDisplay( painter, option, option.rect, text );
    drawFocus( painter, option, option.rect );
  }
  else
  {
    QItemDelegate::paint( painter, option, index );
  }
}

QWidget* MeasurementTextDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, 
const QModelIndex &index ) const
{
  if ( index.column() == text_column_ )
  {
    QLineEdit* line_edit = new QLineEdit( parent );
    connect( line_edit, SIGNAL( textEdited( QString ) ), this, SLOT( commit_editor() ) );
    return line_edit;
  }
  else
  {
    return QItemDelegate::createEditor( parent, option, index );
  }
}

void MeasurementTextDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  if ( index.column() == text_column_ )
  {
    QString note = index.model()->data( index, Qt::DisplayRole ).toString();
    QLineEdit* line_edit = qobject_cast< QLineEdit* >( editor );
    line_edit->setText( note );
  }
  else
  {
    QItemDelegate::setEditorData( editor, index );
  }
}

void MeasurementTextDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, 
  const QModelIndex &index ) const
{
  if ( index.column() == text_column_ )
  {
    QLineEdit* line_edit = qobject_cast< QLineEdit* >( editor );
    QString note = line_edit->text();
    model->setData( index, note );
  }
  else
  {
    QItemDelegate::setModelData( editor, model, index );
  }
}

QSize MeasurementTextDelegate::sizeHint( const QStyleOptionViewItem & option, 
  const QModelIndex & index ) const
{
  QSize size_hint = QItemDelegate::sizeHint( option, index );
  // We don't want a tall cell when the text has line breaks, so set height to 0 and rely 
  // on other columns to set reasonable size.
  size_hint.setHeight( 0 );
  return size_hint;
}

void MeasurementTextDelegate::commit_editor()
{
  QLineEdit* editor = qobject_cast<QLineEdit *>( sender() );
  Q_EMIT commitData( editor );
}

} // end namespace Seg3D