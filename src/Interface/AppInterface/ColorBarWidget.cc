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
  ColorButton* color_button_01_;
  ColorButton* color_button_02_;
  ColorButton* color_button_03_;
  ColorButton* color_button_04_;
  ColorButton* color_button_05_;
  ColorButton* color_button_06_;
  ColorButton* color_button_07_;
  ColorButton* color_button_08_;
  ColorButton* color_button_09_;
  ColorButton* color_button_10_;
  ColorButton* color_button_11_;
  ColorButton* color_button_12_;

};

ColorBarWidget::ColorBarWidget( QWidget *parent ) :
  QWidget( parent ),
  private_( new ColorBarWidgetPrivate )
{
  
  // Set up the private internals of the ColorBarWidget class
  this->private_->ui_.setupUi( this );
  this->initialize_button_list();
  this->make_connections();

}

ColorBarWidget::~ColorBarWidget()
{
} 
  
void ColorBarWidget::initialize_button_list()
{
  this->private_->color_button_01_ = new ColorButton( this, 0 );
  this->private_->ui_.color_1_layout_->addWidget( this->private_->color_button_01_ );
  this->private_->color_button_01_->setObjectName( QString::fromUtf8( "color_button_01_" ) );
  this->button_list_[ 1 ] = this->private_->color_button_01_;
  
  this->private_->color_button_02_ = new ColorButton( this, 1 );
  this->private_->ui_.color_2_layout_->addWidget( this->private_->color_button_02_ );
  this->private_->color_button_02_->setObjectName( QString::fromUtf8( "color_button_02_" ) );
  this->button_list_[ 2 ] = this->private_->color_button_02_;
  
  this->private_->color_button_03_ = new ColorButton( this, 2 );
  this->private_->ui_.color_3_layout_->addWidget( this->private_->color_button_03_ );
  this->private_->color_button_03_->setObjectName( QString::fromUtf8( "color_button_03_" ) );
  this->button_list_[ 3 ] = this->private_->color_button_03_;
  
  this->private_->color_button_04_ = new ColorButton( this, 3 );
  this->private_->ui_.color_4_layout_->addWidget( this->private_->color_button_04_ );
  this->private_->color_button_04_->setObjectName( QString::fromUtf8( "color_button_04_" ) );
  this->button_list_[ 4 ] = this->private_->color_button_04_;
  
  this->private_->color_button_05_ = new ColorButton( this, 4 );
  this->private_->ui_.color_5_layout_->addWidget( this->private_->color_button_05_ );
  this->private_->color_button_05_->setObjectName( QString::fromUtf8( "color_button_05_" ) );
  this->button_list_[ 5 ] = this->private_->color_button_05_;
  
  this->private_->color_button_06_ = new ColorButton( this, 5 );
  this->private_->ui_.color_6_layout_->addWidget( this->private_->color_button_06_ );
  this->private_->color_button_06_->setObjectName( QString::fromUtf8( "color_button_06_" ) );
  this->button_list_[ 6 ] = this->private_->color_button_06_;
  
  this->private_->color_button_07_ = new ColorButton( this, 6 );
  this->private_->ui_.color_7_layout_->addWidget( this->private_->color_button_07_ );
  this->private_->color_button_07_->setObjectName( QString::fromUtf8( "color_button_07_" ) );
  this->button_list_[ 7 ] = this->private_->color_button_07_;
  
  this->private_->color_button_08_ = new ColorButton( this, 7 );
  this->private_->ui_.color_8_layout_->addWidget( this->private_->color_button_08_ );
  this->private_->color_button_08_->setObjectName( QString::fromUtf8( "color_button_08_" ) );
  this->button_list_[ 8 ] = this->private_->color_button_08_;
  
  this->private_->color_button_09_ = new ColorButton( this, 8 );
  this->private_->ui_.color_9_layout_->addWidget( this->private_->color_button_09_ );
  this->private_->color_button_09_->setObjectName( QString::fromUtf8( "color_button_09_" ) );
  this->button_list_[ 9 ] = this->private_->color_button_09_;
  
  this->private_->color_button_10_ = new ColorButton( this, 9 );
  this->private_->ui_.color_10_layout_->addWidget( this->private_->color_button_10_ );
  this->private_->color_button_10_->setObjectName( QString::fromUtf8( "color_button_10_" ) );
  this->button_list_[ 10 ] = this->private_->color_button_10_;
  
  this->private_->color_button_11_ = new ColorButton( this, 10 );
  this->private_->ui_.color_11_layout_->addWidget( this->private_->color_button_11_ );
  this->private_->color_button_11_->setObjectName( QString::fromUtf8( "color_button_11_" ) );
  this->button_list_[ 11 ] = this->private_->color_button_11_;
  
  this->private_->color_button_12_ = new ColorButton( this, 11 );
  this->private_->ui_.color_12_layout_->addWidget( this->private_->color_button_12_ );
  this->private_->color_button_12_->setObjectName( QString::fromUtf8( "color_button_12_" ) );
  this->button_list_[ 12 ] = this->private_->color_button_12_;
  
  for( int i = 1; i < 13; ++i )
    this->button_list_[ i ]->setMaximumSize( 16, 16 );

}   
  
void ColorBarWidget::make_connections()
{
  connect( this->private_->color_button_01_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_01() ) );
  connect( this->private_->color_button_02_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_02() ) );
  connect( this->private_->color_button_03_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_03() ) );
  connect( this->private_->color_button_04_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_04() ) );
  connect( this->private_->color_button_05_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_05() ) );
  connect( this->private_->color_button_06_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_06() ) );
  connect( this->private_->color_button_07_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_07() ) );
  connect( this->private_->color_button_08_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_08() ) );
  connect( this->private_->color_button_09_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_09() ) );
  connect( this->private_->color_button_10_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_10() ) );
  connect( this->private_->color_button_11_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_11() ) );
  connect( this->private_->color_button_12_, SIGNAL( clicked() ), this, 
      SLOT( activate_color_12() ) );
  
  this->private_->color_button_01_->set_color( PreferencesManager::Instance()->color_01_state_->get() );
  QtBridge::Connect( this->private_->color_button_01_, PreferencesManager::Instance()->color_01_state_ );
  
  this->private_->color_button_02_->set_color( PreferencesManager::Instance()->color_02_state_->get() );
  QtBridge::Connect( this->private_->color_button_02_, PreferencesManager::Instance()->color_02_state_ );
  
  this->private_->color_button_03_->set_color( PreferencesManager::Instance()->color_03_state_->get() );
  QtBridge::Connect( this->private_->color_button_03_, PreferencesManager::Instance()->color_03_state_ );
  
  this->private_->color_button_04_->set_color( PreferencesManager::Instance()->color_04_state_->get() );
  QtBridge::Connect( this->private_->color_button_04_, PreferencesManager::Instance()->color_04_state_ );
  
  this->private_->color_button_05_->set_color( PreferencesManager::Instance()->color_05_state_->get() );
  QtBridge::Connect( this->private_->color_button_05_, PreferencesManager::Instance()->color_05_state_ );
  
  this->private_->color_button_06_->set_color( PreferencesManager::Instance()->color_06_state_->get() );
  QtBridge::Connect( this->private_->color_button_06_, PreferencesManager::Instance()->color_06_state_ );
  
  this->private_->color_button_07_->set_color( PreferencesManager::Instance()->color_07_state_->get() );
  QtBridge::Connect( this->private_->color_button_07_, PreferencesManager::Instance()->color_07_state_ );
  
  this->private_->color_button_08_->set_color( PreferencesManager::Instance()->color_08_state_->get() );
  QtBridge::Connect( this->private_->color_button_08_, PreferencesManager::Instance()->color_08_state_ );
  
  this->private_->color_button_09_->set_color( PreferencesManager::Instance()->color_09_state_->get() );
  QtBridge::Connect( this->private_->color_button_09_, PreferencesManager::Instance()->color_09_state_ );
  
  this->private_->color_button_10_->set_color( PreferencesManager::Instance()->color_10_state_->get() );
  QtBridge::Connect( this->private_->color_button_10_, PreferencesManager::Instance()->color_10_state_ );
  
  this->private_->color_button_11_->set_color( PreferencesManager::Instance()->color_11_state_->get() );
  QtBridge::Connect( this->private_->color_button_11_, PreferencesManager::Instance()->color_11_state_ );
  
  this->private_->color_button_12_->set_color( PreferencesManager::Instance()->color_12_state_->get() );
  QtBridge::Connect( this->private_->color_button_12_, PreferencesManager::Instance()->color_12_state_ );
}

  
void ColorBarWidget::set_index_color( int index, QString rgb_color )
{
  if ( index > 12 ) 
  {
    return;
  }
  this->button_list_[ index ]->setStyleSheet( QString::fromUtf8( 
      "QToolButton#color_button_01_{ background-color: " ) + 
      rgb_color + QString::fromUtf8( "; }" ) );

}
      
void ColorBarWidget::set_color_index( int index )
{
  if ( index > 12 ) 
  {
    return;
  }

  for( int i = 1; i <= 12; ++i )
  {
    if( i == index ) 
    {
      this->button_list_[ i ]->setChecked( true );
    }
    else 
    {
      this->button_list_[ i ]->setChecked( false );
    }
  }
}
  

void ColorBarWidget::activate_color_01()
{
  this->current_color_index_ = 1;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_02()
{
  this->current_color_index_ = 2;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_03()
{
  this->current_color_index_ = 3;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_04()
{
  this->current_color_index_ = 4;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_05()
{
  this->current_color_index_ = 5;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_06()
{
  this->current_color_index_ = 6;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_07()
{
  this->current_color_index_ = 7;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_08()
{
  this->current_color_index_ = 8;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_09()
{
  this->current_color_index_ = 9;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_10()
{
  this->current_color_index_ = 10;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_11()
{
  this->current_color_index_ = 11;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}

void ColorBarWidget::activate_color_12()
{
  this->current_color_index_ = 12;
  this->set_color_index( this->current_color_index_ );
  Q_EMIT color_index_changed( this->current_color_index_ );
}
  
void ColorBarWidget::mousePressEvent( QMouseEvent* event )
{
  // do nothing.
}
      

} // end namespace Seg3D
