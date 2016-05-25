/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2016 Scientific Computing and Imaging Institute,
 University of Utah.
 
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitationø
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
#include <QtUtils/Widgets/QtLogSliderIntCombo.h>

// Qt includes
#include <QProxyStyle>

namespace QtUtils
{

class QtLogSliderIntComboPrivate
{
public:
    Ui::SliderIntCombo ui_;
  std::vector<QLabel> steps_;
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

QtLogSliderIntCombo::QtLogSliderIntCombo( QWidget* parent ) :
  QWidget( parent ),
  value_( 0 ),
  private_( new QtLogSliderIntComboPrivate )
{
    this->private_->ui_.setupUi( this );
        
    // Note that using setStyle here invalidates any QSlider stylesheet settings
    this->private_->ui_.horizontalSlider->setStyle( new CustomFocusStyle() );
    this->connect( this->private_->ui_.horizontalSlider, SIGNAL( valueChanged( int ) ), 
    this, SLOT( slider_signal( int ) ) );
    this->connect( this->private_->ui_.spinBox, SIGNAL( valueChanged( int ) ), 
    this, SLOT( spinner_signal( int ) ) );
  
    this->private_->ui_.horizontalSlider->setTickInterval( 100 );

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
}

QtLogSliderIntCombo::~QtLogSliderIntCombo()
{
}
  
void QtLogSliderIntCombo::set_description( std::string description )
{
  this->private_->ui_.description_->setText( QString::fromStdString( description ) );
}

void QtLogSliderIntCombo::spinner_signal( int value )
{
    this->private_->ui_.horizontalSlider->blockSignals( true );
  
  int log_value = static_cast<int>( 100.0 * log( static_cast<double>( value ) ) / log ( 10.0 ) );
  this->private_->ui_.horizontalSlider->setValue( log_value );
  
    Q_EMIT valueAdjusted( value );
  this->private_->ui_.horizontalSlider->blockSignals( false );
  
  this->value_ = value;
}

void QtLogSliderIntCombo::slider_signal( int value )
{
  int real_value = static_cast<int>( pow( 10.0, static_cast<double>( value / 100.0 ) ) );

    this->private_->ui_.spinBox->blockSignals( true );
    this->private_->ui_.spinBox->setValue( real_value );
    Q_EMIT valueAdjusted( real_value );

  this->private_->ui_.spinBox->blockSignals( false );
  
  this->value_ = real_value;
}

void QtLogSliderIntCombo::setStep( int step )
{
  block_signals( true );

    this->private_->ui_.spinBox->setSingleStep( step );
      
  block_signals( false );
}

void QtLogSliderIntCombo::setRange( int min, int max )
{
  block_signals( true );
  
  min = std::max( 1, min );
  int log_min = static_cast<int>( 100.0 * log( static_cast<double>( min ) ) / log( 10.0 ) );
  int log_max = static_cast<int>( 100.0 * log( static_cast<double>( max ) ) / log( 10.0 ) );
    this->private_->ui_.horizontalSlider->setRange( log_min, log_max );
    this->private_->ui_.spinBox->setRange( min, max );
    this->private_->ui_.min_->setNum( min );
    this->private_->ui_.max_->setNum( max );
    this->private_->ui_.horizontalSlider->setTickInterval( max - min );
    
    block_signals( false );
}

void QtLogSliderIntCombo::setCurrentValue( int value )
{
  block_signals( true );
  
  int log_value = static_cast<int>( 100.0 * log( static_cast<double>( value ) ) / log( 10.0 ) );
    this->private_->ui_.horizontalSlider->setValue( log_value );
    this->private_->ui_.spinBox->setValue( value );
    
  block_signals( false );
}


void QtLogSliderIntCombo::change_min( int new_min )
{
  block_signals( true );

  new_min = std::max( 1, new_min );

  int log_min = static_cast<int>( 100.0 * log( static_cast<double>( new_min ) ) / log( 10.0 ) );
    this->private_->ui_.horizontalSlider->setMinimum( log_min );
    this->private_->ui_.spinBox->setMinimum( new_min );
    this->private_->ui_.min_->setNum(new_min);
    
  block_signals( false );
}

void QtLogSliderIntCombo::change_max( int new_max )
{
  block_signals( true );

  int log_max = static_cast<int>( 100.0 * log( static_cast<double>( new_max ) ) / log( 10.0 ) );
    this->private_->ui_.horizontalSlider->setMaximum( log_max );
    this->private_->ui_.spinBox->setMaximum( new_max );
    this->private_->ui_.max_->setNum( new_max );

    block_signals( false );
}

void QtLogSliderIntCombo::block_signals( bool block )
{
  this->private_->ui_.horizontalSlider->blockSignals( block );
  this->private_->ui_.spinBox->blockSignals( block ); 
}

}  // end namespace QtUtils
