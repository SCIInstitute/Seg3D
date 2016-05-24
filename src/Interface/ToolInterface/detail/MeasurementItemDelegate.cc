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
#include <QColorDialog>
#include <QtGui/QDoubleValidator>
#include <QLineEdit>
#include <QtGui/QPainter>

// Interface includes
#include <Interface/ToolInterface/detail/MeasurementItemDelegate.h>

// Core includes
#include <Core/Geometry/Measurement.h>

namespace Seg3D
{

  class MeasurementItemDelegatePrivate{
  public:
    QColor new_color_;

  };

MeasurementItemDelegate::MeasurementItemDelegate( int color_column, int name_column, int length_column, 
  QObject * parent /*= 0 */ ) : 
  color_column_( color_column ),
  name_column_( name_column ),
  length_column_( length_column ),
  QItemDelegate( parent ),
  private_( new MeasurementItemDelegatePrivate ){}

void MeasurementItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, 
  const QModelIndex &index ) const
{
  if( index.column() == this->color_column_ )
  {
    // Use paint function to set color of cell so that color doesn't change when cell is
    // highlighted (selected).
    QColor measurement_color = index.model()->data( index, Qt::DecorationRole ).value< QColor >();
    painter->fillRect( option.rect, measurement_color );
  }
  else if ( index.column() == this->name_column_ || index.column() == this->length_column_ )
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

QWidget* MeasurementItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, 
const QModelIndex &index ) const
{
  if ( index.column() == this->length_column_ )
  {
    QLineEdit* line_edit = new QLineEdit( parent );
    QDoubleValidator* double_validator = new QDoubleValidator( parent );
    line_edit->setValidator( double_validator );
    return line_edit;
  }
  else if( index.column() == this->color_column_ )
  {
    QColor measurement_color = index.model()->data( index, Qt::DecorationRole ).value< QColor >();
    this->private_->new_color_ = QColorDialog::getColor( measurement_color, parent->parentWidget()->parentWidget() );
    
    // In the case that the user presses cancel instead, we'll abort the edit
    if( !this->private_->new_color_.isValid() )
    {
      return 0;
    }
    // Here we are going to pass a dummy widget so that we can take advantage of some of this 
    // editing process.
    QWidget *dummy = new QWidget( parent );
    return dummy;
  }
  else if( index.column() == this->name_column_ )
  {
    QLineEdit* line_edit = new QLineEdit( parent );
    line_edit->setValidator( new QRegExpValidator( 
      QRegExp( QString::fromStdString( Core::Measurement::REGEX_VALIDATOR_C ) ), parent ) );
    return line_edit;
  }
  else
  {
    return QItemDelegate::createEditor( parent, option, index );
  }
}

void MeasurementItemDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  if ( index.column() == this->length_column_ || index.column() == this->name_column_ )
  {
    QString model_text = index.model()->data( index, Qt::DisplayRole ).toString();
    QLineEdit* line_edit = qobject_cast< QLineEdit* >( editor );
    line_edit->setText( model_text );
  }
  else if( index.column() == this->color_column_ )
  {
    //Here we do nothing since our editor has already finished.
  }
  else
  {
    QItemDelegate::setEditorData( editor, index );
  }
}

void MeasurementItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, 
  const QModelIndex &index ) const
{
  if ( index.column() == this->name_column_ || index.column() == this->length_column_ )
  {
    QLineEdit* line_edit = qobject_cast< QLineEdit* >( editor );
    QString editor_text = line_edit->text();
    model->setData( index, editor_text );
  }
  else if( index.column() == this->color_column_ )
  {
    // finally, if the user has selected a valid color we set it in the model.
    if( this->private_->new_color_.isValid() )
    {
      model->setData( index, this->private_->new_color_ );
    }
  }
  else
  {
    QItemDelegate::setModelData( editor, model, index );
  }
}

QSize MeasurementItemDelegate::sizeHint( const QStyleOptionViewItem & option, 
  const QModelIndex & index ) const
{
  QSize size_hint = QItemDelegate::sizeHint( option, index );
  // We don't want a tall cell when the text has line breaks, so set height to 0 and rely 
  // on other columns to set reasonable size.
  size_hint.setHeight( 0 );
  return size_hint;
}

} // end namespace Seg3D
