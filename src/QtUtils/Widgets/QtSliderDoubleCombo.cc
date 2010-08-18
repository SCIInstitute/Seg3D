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


// Core Includes 
#include <Core/Utils/Log.h>
#include <Core/Math/MathFunctions.h>

// UI includes
#include "ui_QtSliderDoubleCombo.h"

// QtUtils includes
#include <QtUtils/Widgets/QtSliderDoubleCombo.h>

namespace QtUtils
{

class QtSliderDoubleComboPrivate
{
public:
    Ui::SliderDoubleCombo ui_;
};

QtSliderDoubleCombo::QtSliderDoubleCombo( QWidget* parent, bool edit_range ) :
  QWidget( parent ),
    private_( new QtSliderDoubleComboPrivate )
{
    this->private_->ui_.setupUi( this );
    
    this->private_->ui_.decrease_range_button_->hide();
    this->private_->ui_.increase_range_button_->hide();
    
    if( !edit_range )
    {
        this->private_->ui_.edit_button_->setEnabled( edit_range );
        this->private_->ui_.edit_button_->hide();
    }
    
    this->connect( this->private_->ui_.edit_button_, SIGNAL( clicked( bool ) ), 
    this, SLOT(edit_ranges( bool )) );
    this->connect( this->private_->ui_.decrease_range_button_, SIGNAL( clicked() ), 
    this, SLOT( half_range() ) );
    this->connect( this->private_->ui_.increase_range_button_, SIGNAL( clicked() ), 
    this, SLOT( double_range() ) );
    this->connect( this->private_->ui_.horizontalSlider, SIGNAL( valueChanged( int )), 
    this, SLOT( slider_signal( int )) );
    this->connect( this->private_->ui_.spinBox, SIGNAL( valueChanged( double )), 
    this, SLOT( spinner_signal( double )) );
  
#if defined ( __APPLE__ )  
  QFont font;
  font.setPointSize( 10 );
  this->private_->ui_.min_->setFont( font );
  this->private_->ui_.max_->setFont( font );
  this->private_->ui_.spinBox->setFont( font );
#endif
  
  
}

QtSliderDoubleCombo::~QtSliderDoubleCombo()
{
}

// signal from the spinner
void QtSliderDoubleCombo::spinner_signal( double value )
{   
    this->private_->ui_.horizontalSlider->blockSignals( true );
    int int_version = Core::Round( value ) * 100;
    this->private_->ui_.horizontalSlider->setValue( int_version );
    Q_EMIT valueAdjusted( value );
  this->private_->ui_.horizontalSlider->blockSignals( false );

  value_ = value;
}

// signal from the slider
void QtSliderDoubleCombo::slider_signal( int value )
{
    this->private_->ui_.spinBox->blockSignals( true );
    double double_version = static_cast<double>(value) / 100.0;
    this->private_->ui_.spinBox->setValue( double_version );
    Q_EMIT valueAdjusted( double_version );
  this->private_->ui_.spinBox->blockSignals( false );

  value_ = value;
}

void QtSliderDoubleCombo::setStep(double step)
{
    this->block_signals( true );
    int int_step = static_cast<int>( step * 100 );
    this->private_->ui_.horizontalSlider->setSingleStep( int_step );
    this->private_->ui_.spinBox->setSingleStep( step );
    this->block_signals( false );
}

void QtSliderDoubleCombo::setRange( double min, double max)
{
    this->block_signals( true );
    this->private_->ui_.horizontalSlider->setRange( static_cast<int>( min * 100.0 ), 
    static_cast<int>( max * 100 ) );
    this->private_->ui_.spinBox->setRange( min, max );
    this->private_->ui_.min_->setNum( min );
    this->private_->ui_.max_->setNum( max );
    
    double tick = ( ( max * 100.0 ) - ( min * 100.0 ) ) / 10.0;
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
    this->block_signals( false );
}
void QtSliderDoubleCombo::setCurrentValue( double value )
{
    this->block_signals( true );
    this->private_->ui_.horizontalSlider->setValue( static_cast<int>( value * 100.0 ) );
    this->private_->ui_.spinBox->setValue( value );
    this->block_signals( false );
}

void QtSliderDoubleCombo::change_min( double new_min )
{
    this->block_signals( true );
    this->private_->ui_.horizontalSlider->setMinimum( static_cast<int>( new_min * 100.0 ) );
    this->private_->ui_.spinBox->setMinimum( new_min );
    this->private_->ui_.min_->setNum(new_min);
    int tick = ( this->private_->ui_.max_->text().toInt() - 
    this->private_->ui_.min_->text().toInt()) / 10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick * 100 );
    this->block_signals( false );
}

void QtSliderDoubleCombo::change_max( double new_max )
{
    this->block_signals( true );
    this->private_->ui_.horizontalSlider->setMaximum( static_cast<int>( new_max * 100.0 ) );
    this->private_->ui_.spinBox->setMaximum( new_max );
    this->private_->ui_.max_->setNum( new_max );
    int tick = (this->private_->ui_.max_->text().toInt() - 
    this->private_->ui_.min_->text().toInt()) / 10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick * 100 );
    this->block_signals( false );
}

void QtSliderDoubleCombo::double_range()
{
    double new_max = this->private_->ui_.max_->text().toDouble() * 2;
    this->change_max( new_max );
    Q_EMIT rangeChanged( this->private_->ui_.min_->text().toDouble(), new_max );
}
void QtSliderDoubleCombo::half_range()
{
   double new_max = this->private_->ui_.max_->text().toDouble() / 2;
   this->change_max( new_max );
   Q_EMIT rangeChanged( this->private_->ui_.min_->text().toDouble(), new_max );
}

void QtSliderDoubleCombo::edit_ranges( bool edit )
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

void QtSliderDoubleCombo::block_signals( bool block )
{
    this->private_->ui_.horizontalSlider->blockSignals( block );
    this->private_->ui_.spinBox->blockSignals( block ); 
}

}  // end namespace QtUtils
