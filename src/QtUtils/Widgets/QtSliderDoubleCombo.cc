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


// Core Includes 
#include <Core/Utils/Log.h>
#include <Core/Math/MathFunctions.h>

// UI includes
#include "ui_QtSliderDoubleCombo.h"

// QtUtils includes
#include <QtUtils/Widgets/QtSliderDoubleCombo.h>

// Qt includes
#include <QProxyStyle>

namespace QtUtils
{

class QtSliderDoubleComboPrivate
{
public:
    Ui::SliderDoubleCombo ui_;
  double min_;
  double max_;
};

class CustomFocusStyle : public QProxyStyle
{
public:
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const
  {
    // do not draw focus rectangles for QSlider
    if ( element == QStyle::PE_FrameFocusRect )
    {
      return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
  }
};

QtSliderDoubleCombo::QtSliderDoubleCombo( QWidget* parent ) :
  QWidget( parent ),
  value_( 0 ),
    private_( new QtSliderDoubleComboPrivate )
{
  this->private_->ui_.setupUi( this );

  // Note that using setStyle here invalidates any QSlider stylesheet settings
  this->private_->ui_.horizontalSlider->setStyle( new CustomFocusStyle() );
  this->private_->ui_.horizontalSlider->setRange( 0, 100 );
  this->private_->ui_.horizontalSlider->setTickInterval( 10 );
  this->private_->ui_.horizontalSlider->setPageStep( 10 );

  QFont font = this->private_->ui_.min_->font();
#ifdef __APPLE__
  font.setPointSize( 10 );
#else
  font.setPointSize( 8 );
#endif
  this->private_->ui_.min_->setFont( font );
  this->private_->ui_.max_->setFont( font );
  this->private_->ui_.spinBox->setFont( font );

#ifdef __APPLE__
  font = this->private_->ui_.description_->font();
  font.setPointSize( 12 );
  this->private_->ui_.description_->setFont( font );
#endif

  this->connect( this->private_->ui_.horizontalSlider, SIGNAL( valueChanged( int ) ), 
  this, SLOT( slider_signal( int ) ) );
  this->connect( this->private_->ui_.spinBox, SIGNAL( valueChanged( double ) ), 
  this, SLOT( spinner_signal( double ) ) );
}

QtSliderDoubleCombo::~QtSliderDoubleCombo()
{
}
  
void QtSliderDoubleCombo::set_description( const std::string& description )
{
  this->private_->ui_.description_->setText( QString::fromStdString( description ) );
}

void QtSliderDoubleCombo::set_tooltip( const std::string& tooltip )
{
  this->private_->ui_.horizontalSlider->setToolTip( QString::fromStdString( tooltip ) );
  this->private_->ui_.spinBox->setToolTip( QString::fromStdString( tooltip ) );
}

// signal from the spinner
void QtSliderDoubleCombo::spinner_signal( double value )
{   
  this->value_ = Core::Clamp( value, this->private_->min_, this->private_->max_ );
  
    this->private_->ui_.horizontalSlider->blockSignals( true );

  double temp_max;
  double percentage;

  value = this->value_ - this->private_->min_;
  temp_max = this->private_->max_ - this->private_->min_;

  percentage = value / temp_max;

    this->private_->ui_.horizontalSlider->setValue( Core::Round( percentage * 100 ) );
    Q_EMIT valueAdjusted( this->value_ );
  this->private_->ui_.horizontalSlider->blockSignals( false );
}

// signal from the slider
void QtSliderDoubleCombo::slider_signal( int percentage )
{
    this->private_->ui_.spinBox->blockSignals( true );

  double temp_max = this->private_->max_ - this->private_->min_;

  double new_value = temp_max * ( static_cast<double>( percentage ) * 0.01 ) 
    + this->private_->min_;

  if( percentage == 0 )
  {
    new_value = this->private_->min_;
  }
  else if( percentage == 100 )
  {
    new_value = this->private_->max_;
  }

  this->value_ = new_value;

    this->private_->ui_.spinBox->setValue( this->value_ );
    Q_EMIT valueAdjusted( this->value_ );
  this->private_->ui_.spinBox->blockSignals( false );
}

void QtSliderDoubleCombo::setStep( double step )
{
    this->block_signals( true );
    this->private_->ui_.spinBox->setSingleStep( step );
    this->block_signals( false );
}

void QtSliderDoubleCombo::setRange( double min, double max )
{
    this->block_signals( true );
  this->private_->min_ = min;
  this->private_->max_ = max;
    this->private_->ui_.spinBox->setRange( min, max );
  QString min_string;
  min_string.setNum( min, 'g', 3 );
    this->private_->ui_.min_->setText( min_string );

  QString max_string;
  max_string.setNum( max, 'g', 3 );
    this->private_->ui_.max_->setText( max_string );

    this->block_signals( false );
}
void QtSliderDoubleCombo::setCurrentValue( double value )
{
  // Clamp the value to range
  value = Core::Clamp( value, this->private_->min_, this->private_->max_ );
  // The value is the same, do nothing
  if ( this->value_ == value ) return;
  
  this->block_signals( true );

  this->value_ = value;
    
  double temp_max;
  double percentage;

  value = this->value_ - this->private_->min_;
  temp_max = this->private_->max_ - this->private_->min_;

  percentage = value / temp_max;

  this->private_->ui_.horizontalSlider->setValue( Core::Round( percentage * 100 ) );
    this->private_->ui_.spinBox->setValue( this->value_ );
    this->block_signals( false );
    Q_EMIT valueAdjusted( this->value_ );
}

void QtSliderDoubleCombo::change_min( double new_min )
{
  this->private_->min_ = new_min;
    this->block_signals( true );
  this->private_->ui_.spinBox->setMinimum( new_min );
  QString min_string;
  min_string.setNum( new_min, 'g', 3 );
    this->private_->ui_.min_->setText( min_string );
    this->block_signals( false );
}

void QtSliderDoubleCombo::change_max( double new_max )
{
  this->private_->max_ = new_max;
    this->block_signals( true );
    this->private_->ui_.spinBox->setMaximum( new_max );
  QString max_string;
  max_string.setNum( new_max, 'g', 3 );
    this->private_->ui_.max_->setText( max_string );
    this->block_signals( false );
}

void QtSliderDoubleCombo::block_signals( bool block )
{
    this->private_->ui_.horizontalSlider->blockSignals( block );
    this->private_->ui_.spinBox->blockSignals( block ); 
}

void QtSliderDoubleCombo::connect_min( QtSliderDoubleCombo* min )
{
  connect( min, SIGNAL( valueAdjusted( double ) ), this, SLOT( handle_min_signal( double ) ) );
}

void QtSliderDoubleCombo::connect_max( QtSliderDoubleCombo* max )
{
  connect( max, SIGNAL( valueAdjusted( double ) ), this, SLOT( handle_max_signal( double ) ) );
}

void QtSliderDoubleCombo::handle_min_signal( double value )
{
  if( value < this->value_ )
  {
    this->setCurrentValue( value );
  }
}

void QtSliderDoubleCombo::handle_max_signal( double value )
{
  if( value > this->value_ )
  {
    this->setCurrentValue( value );
  }
}

}  // end namespace QtUtils
