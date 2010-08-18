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


//Core includes
#include <Core/Utils/Log.h>

// UI includes
#include "ui_QtSliderIntCombo.h"

// QtUtils includes
#include <QtUtils/Widgets/QtSliderIntCombo.h>

namespace QtUtils
{

class QtSliderIntComboPrivate
{
public:
    Ui::SliderIntCombo ui_;
};

QtSliderIntCombo::QtSliderIntCombo( QWidget* parent, bool edit_range ) :
     QWidget( parent ),
  private_( new QtSliderIntComboPrivate )
{
    this->private_->ui_.setupUi( this );
    
    this->private_->ui_.decrease_range_button_->hide();
    this->private_->ui_.increase_range_button_->hide();
    
    if( !edit_range )
    {
        this->private_->ui_.edit_button_->setEnabled( edit_range );
        this->private_->ui_.edit_button_->hide();
    }
    
    connect( this->private_->ui_.edit_button_, SIGNAL( clicked( bool ) ), this, SLOT( edit_ranges(bool ) ) );
    connect( this->private_->ui_.decrease_range_button_, SIGNAL( clicked() ), this, SLOT( half_range() ) );
    connect( this->private_->ui_.increase_range_button_, SIGNAL( clicked()), this, SLOT( double_range() ) );
    
    connect( this->private_->ui_.horizontalSlider, SIGNAL( valueChanged( int ) ), this, SLOT( slider_signal( int ) ) );
    connect( this->private_->ui_.spinBox, SIGNAL( valueChanged( int ) ), this, SLOT( spinner_signal( int ) ) );
  
#if defined ( __APPLE__ )  
  QFont font;
  font.setPointSize( 10 );
  this->private_->ui_.min_->setFont( font );
  this->private_->ui_.max_->setFont( font );
  this->private_->ui_.spinBox->setFont( font );
#endif
  
}

QtSliderIntCombo::~QtSliderIntCombo()
{
}

void QtSliderIntCombo::spinner_signal( int value )
{
    this->private_->ui_.horizontalSlider->blockSignals( true );
    this->private_->ui_.horizontalSlider->setValue( value );
    Q_EMIT valueAdjusted( value );
  this->private_->ui_.horizontalSlider->blockSignals( false );
  
  this->value_ = value;
}

void QtSliderIntCombo::slider_signal( int value )
{
    this->private_->ui_.spinBox->blockSignals( true );
    this->private_->ui_.spinBox->setValue( value );
    Q_EMIT valueAdjusted( value );
  this->private_->ui_.spinBox->blockSignals( false );
  
  this->value_ = value;
}


void QtSliderIntCombo::setStep(int step)
{
  block_signals( true );
    this->private_->ui_.horizontalSlider->setSingleStep( step );
    this->private_->ui_.spinBox->setSingleStep( step );
    block_signals( false );
}

void QtSliderIntCombo::setRange( int min, int max)
{
  block_signals( true );
    this->private_->ui_.horizontalSlider->setRange( min, max );
    this->private_->ui_.spinBox->setRange( min, max );
    this->private_->ui_.min_->setNum( min );
    this->private_->ui_.max_->setNum( max );
    
    int tick = (max - min)/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
    block_signals( false );
}

void QtSliderIntCombo::setCurrentValue( int value )
{
  block_signals( true );
    this->private_->ui_.horizontalSlider->setValue( value );
    this->private_->ui_.spinBox->setValue( value );
    block_signals( false );
}


void QtSliderIntCombo::change_min( int new_min )
{
  block_signals( true );
    this->private_->ui_.horizontalSlider->setMinimum( new_min );
    this->private_->ui_.spinBox->setMinimum( new_min );
    this->private_->ui_.min_->setNum(new_min);
    int tick = (this->private_->ui_.max_->text().toInt() - this->private_->ui_.min_->text().toInt())/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
    block_signals( false );
}

void QtSliderIntCombo::change_max( int new_max )
{
  block_signals( true );
    this->private_->ui_.horizontalSlider->setMaximum( new_max );
    this->private_->ui_.spinBox->setMaximum( new_max );
    this->private_->ui_.max_->setNum( new_max );
    int tick = (this->private_->ui_.max_->text().toInt() - this->private_->ui_.min_->text().toInt())/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
    block_signals( false );
}

void QtSliderIntCombo::double_range()
{
    int new_max = this->private_->ui_.max_->text().toInt() * 2;
    change_max( new_max );
    rangeChanged( this->private_->ui_.min_->text().toInt(), new_max );
}
void QtSliderIntCombo::half_range()
{
    int new_max = this->private_->ui_.max_->text().toInt() / 2;
    change_max( new_max );
    rangeChanged( this->private_->ui_.min_->text().toInt(), new_max );
}

void QtSliderIntCombo::edit_ranges( bool edit )
{
  if( edit )
  {
    this->private_->ui_.decrease_range_button_->show();
    this->private_->ui_.increase_range_button_->show();
  }
  else
  {
    this->private_->ui_.decrease_range_button_->hide();
    this->private_->ui_.increase_range_button_->hide();
  }
}

void QtSliderIntCombo::block_signals( bool block )
{
  this->private_->ui_.horizontalSlider->blockSignals( block );
  this->private_->ui_.spinBox->blockSignals( block ); 
}

}  // end namespace QtUtils
