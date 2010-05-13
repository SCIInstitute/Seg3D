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


// Util includes
#include <Core/Utils/Log.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>

// Interface includes
#include <Interface/AppInterface/ColorBarWidget.h>
#include <Interface/AppInterface/StyleSheet.h>
#include <Interface/QtInterface/QtBridge.h>
#include "ui_ColorBarWidget.h"

namespace Seg3D
{

class ColorBarWidgetPrivate
{
public:
  Ui::ColorBarWidget ui_;
  QButtonGroup* color_button_group_;

};

ColorBarWidget::ColorBarWidget( QWidget *parent ) :
  QWidget( parent ),
  private_( new ColorBarWidgetPrivate )
{
  // Set up the private internals of the ColorBarWidget class
  this->private_->ui_.setupUi( this );
  this->private_->color_button_group_ = new QButtonGroup( this );
  this->initialize_buttons();
}

ColorBarWidget::~ColorBarWidget()
{
} 
  
void ColorBarWidget::initialize_buttons()
{ 
  for( int i = 0; i < 12; ++i )
  {
    this->private_->color_button_group_->addButton( new ColorButton( 
      this, i, PreferencesManager::Instance()->color_states_[ i ]->get(), 16, 16 ), i );
    QtBridge::Connect( dynamic_cast< ColorButton* >( this->private_->color_button_group_->button( i ) ), 
      PreferencesManager::Instance()->color_states_[ i ] );
    connect(  dynamic_cast< ColorButton* >( this->private_->color_button_group_->button( i ) ), 
      SIGNAL( color_changed( int ) ), this, SLOT( color_only_changed( int ) ) );
    this->private_->ui_.button_layout_->addWidget( this->private_->color_button_group_->button( i ) );
  }

  connect( this->private_->color_button_group_, SIGNAL( buttonClicked( int ) ),
    this, SLOT( signal_activation( int ) ) );
}   

void ColorBarWidget::signal_activation( int active )
{
  Q_EMIT this->color_index_changed( active );
}

void ColorBarWidget::set_color_index( int index )
{
  dynamic_cast< ColorButton* >( this->private_->color_button_group_->button( index ) )->
    setChecked( true );
  this->signal_activation( index );
  
}

void ColorBarWidget::color_only_changed( int button_index )
{
  Q_EMIT this->color_changed( button_index );
}

void ColorBarWidget::mousePressEvent( QMouseEvent* event )
{
  // do nothing.
}
      

} // end namespace Seg3D
