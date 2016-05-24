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

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/Application/ColorPickerWidget.h>

// The interface from the designer
#include "ui_ColorPickerWidget.h"

namespace Seg3D
{

class ColorPickerWidgetPrivate
{
public:
  Ui::ColorPickerWidget ui_;
  QtUtils::QtSliderIntCombo* r_adjuster_;
  QtUtils::QtSliderIntCombo* g_adjuster_;
  QtUtils::QtSliderIntCombo* b_adjuster_;

};

ColorPickerWidget::ColorPickerWidget( QWidget *parent ) :
    QWidget( parent ),
    private_( new ColorPickerWidgetPrivate )
{
    private_->ui_.setupUi( this );
  
  // add the SliderCombo Widgets
  this->private_->r_adjuster_ = new QtUtils::QtSliderIntCombo( this );
  this->private_->ui_.r_h_layout_->addWidget( this->private_->r_adjuster_ );
  this->private_->r_adjuster_->setObjectName( QString::fromUtf8( "r_adjuster_" ) );
  this->private_->r_adjuster_->setRange( 0, 255 );
  this->private_->r_adjuster_->setCurrentValue( 0 );
  
  this->private_->g_adjuster_ = new QtUtils::QtSliderIntCombo( this );
  this->private_->ui_.g_h_layout_->addWidget( this->private_->g_adjuster_ );
  this->private_->g_adjuster_->setObjectName( QString::fromUtf8( "g_adjuster_" ) );
  this->private_->g_adjuster_->setRange( 0, 255 );
  this->private_->g_adjuster_->setCurrentValue( 0 );
  
  this->private_->b_adjuster_ = new QtUtils::QtSliderIntCombo( this );
  this->private_->ui_.b_h_layout_->addWidget( this->private_->b_adjuster_ );
  this->private_->b_adjuster_->setObjectName( QString::fromUtf8( "b_adjuster_" ) );
  this->private_->b_adjuster_->setRange( 0, 255 );
  this->private_->b_adjuster_->setCurrentValue( 0 );
  
  this->private_->r_adjuster_->set_description( "R" );
  this->private_->g_adjuster_->set_description( "G" );
  this->private_->b_adjuster_->set_description( "B" );
  

  connect( this, SIGNAL( color_changed() ), this, SLOT( set_color() ) );

  connect( this->private_->r_adjuster_, SIGNAL( valueAdjusted( int ) ), 
    this, SLOT( set_r( int ) ) );
  connect( this->private_->g_adjuster_, SIGNAL( valueAdjusted( int ) ), 
    this, SLOT( set_g( int ) ) );
  connect( this->private_->b_adjuster_, SIGNAL( valueAdjusted( int ) ),
    this, SLOT( set_b( int ) ) );
  connect( this->private_->ui_.set_color_button_, SIGNAL( clicked() ), 
    this, SLOT( signal_color_set() ) );


}

ColorPickerWidget::~ColorPickerWidget()
{
}
  
void ColorPickerWidget::set_color()
{
  QString style_sheet = QString::fromUtf8( "QWidget#color_sample_{"
    "background-color: rgb(" ) + QString::number( this->r_ ) +
    QString::fromUtf8( ", " ) + QString::number( this->g_ ) +
    QString::fromUtf8( ", " ) + QString::number( this->b_ ) +
    QString::fromUtf8( "); }" );

  this->private_->ui_.color_sample_->setStyleSheet( style_sheet );
  this->private_->ui_.color_sample_->repaint();
}
  
void ColorPickerWidget::hide_show( Core::Color color, bool show )
{
  if ( !show )
  {
    this->setVisible( false );
    return;
  }

  this->r_ = static_cast< int >( color.r() );
  this->g_ = static_cast< int >( color.g() );
  this->b_ = static_cast< int >( color.b() );
  
  this->private_->r_adjuster_->setCurrentValue( this->r_ );
  this->private_->g_adjuster_->setCurrentValue( this->g_ );
  this->private_->b_adjuster_->setCurrentValue( this->b_ );
  
  this->set_color();
  
  this->setVisible( true );
  
}
  
void ColorPickerWidget::set_r( int r )
{
  this->r_ = r;
  Q_EMIT color_changed();
}

void ColorPickerWidget::set_g( int g )
{
  this->g_ = g;
  Q_EMIT color_changed();
}

void ColorPickerWidget::set_b( int b )
{
  this->b_ = b;
  Q_EMIT color_changed();
}

void ColorPickerWidget::signal_color_set()
{
  Q_EMIT color_set( Core::Color( this->r_, this->g_, this->b_ ) );
}
  
} // end namespace Seg3D
