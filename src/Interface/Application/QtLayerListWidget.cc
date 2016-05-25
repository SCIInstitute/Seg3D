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

// UI includes
#include "ui_QtLayerListWidget.h"

// QtUtils includes
#include <Interface/Application/QtLayerListWidget.h>
#include <Interface/Application/StyleSheet.h>

namespace Seg3D
{

class QtLayerListWidgetPrivate
{
public:
    Ui::QtLayerListWidget ui_;
};

QtLayerListWidget::QtLayerListWidget( QWidget* parent ) :
  QWidget( parent ),
  private_( new QtLayerListWidgetPrivate )
{
    this->private_->ui_.setupUi( this );
    connect( this->private_->ui_.layer_spinbox_, SIGNAL( valueChanged( int ) ),
    this, SLOT( emit_changed_signal( int ) ) ); 
  this->private_->ui_.layer_spinbox_->setRange( 0, 255 );
}

QtLayerListWidget::~QtLayerListWidget()
{
}

int QtLayerListWidget::get_value() const
{
  return this->private_->ui_.layer_spinbox_->value();
}

QString QtLayerListWidget::get_label() const
{
  return this->private_->ui_.layer_name_label_->text();
}

bool QtLayerListWidget::counter_hidden() const
{
  return this->private_->ui_.layer_spinbox_->isHidden();
}
  
void QtLayerListWidget::set_validity( bool valid )
{
  if( valid )
  {
    this->setStyleSheet( StyleSheet::QTLAYERLISTWIDGET_VALID_C );
  }
  else
  {
    this->setStyleSheet( StyleSheet::QTLAYERLISTWIDGET_INVALID_C );
  }
}

void QtLayerListWidget::set_mask_name( const std::string& name )
{
  this->private_->ui_.layer_name_label_->setText( QString::fromStdString( name ) );
}

void QtLayerListWidget::set_mask_index( const int index )
{
  this->private_->ui_.layer_spinbox_->setValue( index );
}

void QtLayerListWidget::emit_changed_signal( int )
{
  Q_EMIT index_changed_signal();
}

void QtLayerListWidget::hide_counter( bool hide )
{
  if( hide )
  {
    this->private_->ui_.layer_spinbox_->hide();
  }
}

}  // end namespace Seg3D
