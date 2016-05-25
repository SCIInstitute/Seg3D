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


//Core includes
#include <Core/Utils/Log.h>

// UI includes
#include "ui_QtSliderIntCombo.h"

// QtUtils includes
#include <QtUtils/Widgets/QtSliderIntCombo.h>

// Qt includes
#include <QProxyStyle>

namespace QtUtils
{

class QtSliderIntComboPrivate
{
public:
    Ui::SliderIntCombo ui_;
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

QtSliderIntCombo::QtSliderIntCombo( QWidget* parent, bool edit_range ) :
    QWidget( parent ),
  value_( 0 ),
  private_( new QtSliderIntComboPrivate )
{
  this->private_->ui_.setupUi( this );

  // Note that using setStyle here invalidates any QSlider stylesheet settings
  this->private_->ui_.horizontalSlider->setStyle( new CustomFocusStyle() );
  this->connect( this->private_->ui_.horizontalSlider, SIGNAL( valueChanged( int ) ), 
  this, SLOT( slider_signal( int ) ) );
  this->connect( this->private_->ui_.spinBox, SIGNAL( valueChanged( int ) ), 
  this, SLOT( spinner_signal( int ) ) );

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

QtSliderIntCombo::~QtSliderIntCombo()
{
}

void QtSliderIntCombo::set_description( const std::string& description )
{
  this->private_->ui_.description_->setText( QString::fromStdString( description ) );
}

void QtSliderIntCombo::set_tooltip( const std::string& tooltip )
{
  this->private_->ui_.horizontalSlider->setToolTip( QString::fromStdString( tooltip ) );
  this->private_->ui_.spinBox->setToolTip( QString::fromStdString( tooltip ) );
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
  this->private_->ui_.horizontalSlider->setPageStep( step );
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

  // NOTE: Fix the cases where there are less than 10 options to choose from
  if ( tick == 0 )
  {
    tick = this->private_->ui_.horizontalSlider->singleStep();
  }
  else
  {
    if ( (this->private_->ui_.horizontalSlider->singleStep() % 2 != 0) &&
         (tick % 2 != 0) )
    {
      tick += 1;
    }
  }

  this->private_->ui_.horizontalSlider->setTickInterval( tick );
  block_signals( false );
}

void QtSliderIntCombo::setCurrentValue( int value )
{
  if ( this->value_ == value ) return;

  block_signals( true );
  this->value_ = value;
  this->private_->ui_.horizontalSlider->setValue( this->value_ );
  this->private_->ui_.spinBox->setValue( this->value_ );
  block_signals( false );
  Q_EMIT valueAdjusted( this->value_ );
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

void QtSliderIntCombo::block_signals( bool block )
{
  this->private_->ui_.horizontalSlider->blockSignals( block );
  this->private_->ui_.spinBox->blockSignals( block ); 
}

}  // end namespace QtUtils
