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


//Core Includes - for logging
#include <Utils/Core/Log.h>

#include <Interface/ToolInterface/CustomWidgets/SliderDoubleCombo.h>
#include "ui_SliderDoubleCombo.h"

namespace Seg3D
{

class SliderDoubleComboPrivate
{
public:
    Ui::SliderDoubleCombo ui_;
};

SliderDoubleCombo::SliderDoubleCombo( QWidget* parent, bool edit_range ) :
    private_( new SliderDoubleComboPrivate ), QWidget( parent )
{
    this->private_->ui_.setupUi( this );
    
    this->private_->ui_.decrease_range_button_->hide();
    this->private_->ui_.increase_range_button_->hide();
    
    if( !edit_range )
    {
        this->private_->ui_.edit_button_->setEnabled( edit_range );
        this->private_->ui_.edit_button_->hide();
    }
    
    connect( this->private_->ui_.edit_button_, SIGNAL( clicked( bool ) ), this, SLOT(edit_ranges( bool )) );
    connect( this->private_->ui_.decrease_range_button_, SIGNAL( clicked() ), this, SLOT( half_range() ) );
    connect( this->private_->ui_.increase_range_button_, SIGNAL( clicked() ), this, SLOT( double_range() ) );
    
    connect( this->private_->ui_.horizontalSlider, SIGNAL( valueChanged( int )), this, SLOT( slider_signal( int )) );
    connect( this->private_->ui_.spinBox, SIGNAL( valueChanged( double )), this, SLOT( spinner_signal( double )) );
}

SliderDoubleCombo::~SliderDoubleCombo()
{
}

void SliderDoubleCombo::spinner_signal( double value )
{   
    int int_version = value * 100;
    this->private_->ui_.horizontalSlider->blockSignals( true );
    this->private_->ui_.horizontalSlider->setValue( int_version );
    Q_EMIT valueAdjusted( value );
  this->private_->ui_.horizontalSlider->blockSignals( false );
    
}


void SliderDoubleCombo::slider_signal( int value )
{
    double double_version = (double)value / 100;
    this->private_->ui_.spinBox->blockSignals( true );
    this->private_->ui_.spinBox->setValue( double_version );
    Q_EMIT valueAdjusted( double_version );
  this->private_->ui_.spinBox->blockSignals( false );
    
}


void SliderDoubleCombo::setStep(double step)
{
    this->private_->ui_.horizontalSlider->setSingleStep( step * 100 );
    this->private_->ui_.spinBox->setSingleStep( step );
}
void SliderDoubleCombo::setRange( double min, double max)
{
    this->private_->ui_.horizontalSlider->setRange( min * 100, max * 100 );
    this->private_->ui_.spinBox->setRange( min, max );
    this->private_->ui_.min_->setNum( min );
    this->private_->ui_.max_->setNum( max );
    
    double tick = (max - min)/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick * 100 );
}
void SliderDoubleCombo::setCurrentValue( double value )
{
    this->private_->ui_.horizontalSlider->setValue( value * 100 );
    this->private_->ui_.spinBox->setValue( value );
}

void SliderDoubleCombo::edit_ranges( bool edit )
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

void SliderDoubleCombo::change_min( double new_min )
{
    this->private_->ui_.horizontalSlider->setMinimum( new_min * 100 );
    this->private_->ui_.spinBox->setMinimum( new_min );
    this->private_->ui_.min_->setNum(new_min);
    int tick = (this->private_->ui_.max_->text().toInt() - this->private_->ui_.min_->text().toInt())/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick * 100 );
}

void SliderDoubleCombo::change_max( double new_max )
{
    this->private_->ui_.horizontalSlider->setMaximum( new_max * 100 );
    this->private_->ui_.spinBox->setMaximum( new_max );
    this->private_->ui_.max_->setNum( new_max );
    int tick = (this->private_->ui_.max_->text().toInt() - this->private_->ui_.min_->text().toInt())/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick * 100 );
}

void SliderDoubleCombo::set_all( double min, double max, double value )
{
    double tick = (max - min)/10;
        
    this->private_->ui_.horizontalSlider->setRange( min * 100, max * 100 );
    this->private_->ui_.horizontalSlider->setValue( value * 100 );
    this->private_->ui_.horizontalSlider->setTickInterval( tick * 100 );
    this->private_->ui_.spinBox->setRange(min, max);
    this->private_->ui_.spinBox->setValue( value );
    this->private_->ui_.min_->setNum( min );
    this->private_->ui_.max_->setNum( max );

}

void SliderDoubleCombo::double_range()
{
    double new_max = this->private_->ui_.max_->text().toDouble() * 2;
    change_max( new_max );
    Q_EMIT rangeChanged( this->private_->ui_.min_->text().toDouble(), new_max );
}
void SliderDoubleCombo::half_range()
{
   double new_max = this->private_->ui_.max_->text().toDouble() / 2;
   change_max( new_max );
   Q_EMIT rangeChanged( this->private_->ui_.min_->text().toDouble(), new_max );
}



}  // end namespace Seg3D