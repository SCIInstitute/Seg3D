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


// Util includes
#include <Core/Utils/Log.h>

// UI includes
#include "ui_QtColorBarWidget.h"

// QtUtils includes
#include <QtUtils/Widgets/QtColorBarWidget.h>
#include <QButtonGroup>

namespace QtUtils
{

class QtColorBarWidgetPrivate
{
public:
  Ui::ColorBarWidget ui_;
  QButtonGroup* color_button_group_;

};

QtColorBarWidget::QtColorBarWidget( QWidget *parent ) :
  QWidget( parent ),
  private_( new QtColorBarWidgetPrivate )
{
  // Set up the private internals of the ColorBarWidget class
  this->private_->ui_.setupUi( this );
  this->private_->color_button_group_ = new QButtonGroup( this );
  
  this->connect( this->private_->color_button_group_, SIGNAL( buttonClicked( int ) ),
    this, SLOT( signal_activation( int ) ) ); 
}

QtColorBarWidget::~QtColorBarWidget()
{
} 

int QtColorBarWidget::get_active_index()
{
  return this->private_->color_button_group_->checkedId();
}
  
void QtColorBarWidget::add_color_button( QtColorButton* color_button, int index )
{
  this->private_->color_button_group_->addButton( color_button, index );
  this->connect( color_button, SIGNAL( color_changed( int ) ), 
    this, SLOT( color_only_changed( int ) ) );
  this->private_->ui_.button_layout_->addWidget( color_button );  
}

void QtColorBarWidget::signal_activation( int active )
{
  Q_EMIT this->color_index_changed( active );
}

void QtColorBarWidget::set_color_index( int index )
{
  dynamic_cast< QtColorButton* >( this->private_->color_button_group_->button( index ) )->
    setChecked( true );
  this->signal_activation( index );
  
}

void QtColorBarWidget::color_only_changed( int button_index )
{
  Q_EMIT this->color_changed( button_index );
}

void QtColorBarWidget::mousePressEvent( QMouseEvent* event )
{
  // do nothing.
}
      

} // end namespace Seg3D
