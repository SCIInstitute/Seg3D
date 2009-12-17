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

#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <Utils/Core/Log.h>

#include <Interface/ToolInterface/CustomWidgets/SliderSpinCombo.h>

namespace Seg3D {


//TODO -- implement slider handling for non integer values

//  --- Start constructors ---  //
SliderSpinCombo::SliderSpinCombo( QWidget *parent )
    : QWidget( parent )
{
    buildWidget();
    makeConnections();
    setRanges( 1, 100);
    setStep(1);
    setCurrentValue( 1 );
    SCI_LOG_MESSAGE("SliderSpinCombo has been created!");
}

SliderSpinCombo::SliderSpinCombo( QWidget *parent, double minRange, double maxRange, double startValue, double stepSize )
    : QWidget( parent )
{
    buildWidget();
    makeConnections();
    setRanges( minRange, maxRange);
    setStep(stepSize);
    setCurrentValue( startValue );
  SCI_LOG_MESSAGE("SliderSpinCombo has been created!");
}

SliderSpinCombo::SliderSpinCombo( QWidget *parent, double minRange, double maxRange, double stepSize )
    : QWidget( parent )
{
    buildWidget();
    makeConnections();
    setRanges( minRange, maxRange);
    setStep(stepSize);
    setCurrentValue( minRange );
    SCI_LOG_MESSAGE("SliderSpinCombo has been created!");
}
//  --- End Constructors ---  //


//  --- function for visually building the widget ---  //
void SliderSpinCombo::buildWidget()
{
    vLayout = new QVBoxLayout( this );
    vLayout->setSpacing( 0 );
    vLayout->setContentsMargins( 0, 0, 0, 0 );

    hTopLayout = new QHBoxLayout();
    hTopLayout->setSpacing( 0 );
    hTopLayout->setContentsMargins( 0, 0, 0, 0 );

    slider = new QSlider;
    slider->setOrientation( Qt::Horizontal );
    slider->setFocusPolicy( Qt::StrongFocus );
    slider->setTickPosition( QSlider::TicksBelow );

    hTopLayout->addWidget( slider );

    spinner = new QDoubleSpinBox;
    spinner->setDecimals( 0 );
    spinner->setFocusPolicy( Qt::StrongFocus );
    hTopLayout->addWidget( spinner );

    vLayout->addLayout( hTopLayout );

    hBottomLayout = new QHBoxLayout();
    hBottomLayout->setSpacing( 0 );
    hBottomLayout->setContentsMargins( 0,0,0,0 );

    minValueLabel = new QLabel( this );
    QFont labelFont;
    labelFont.setPointSize( 10) ;
    minValueLabel->setFont( labelFont );
    minValueLabel->setStyleSheet( QString::fromUtf8("padding-left: -2px; margin-left: 0px;") );

    hBottomLayout->addWidget(minValueLabel);

    maxValueLabel = new QLabel( this );
    maxValueLabel->setFont( labelFont );
    maxValueLabel->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );

    hBottomLayout->addWidget( maxValueLabel );

    spacer = new QSpacerItem( 62, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    hBottomLayout->addItem( spacer );

    vLayout->addLayout( hBottomLayout );

}


//  --- Private slots --- //
void SliderSpinCombo::setSliderValue( int value )
{
    setCurrentValue( value );
    Q_EMIT valueAdjusted( value );
}
void SliderSpinCombo::setSpinnerValue( double value )
{
    setCurrentValue( value );
    Q_EMIT valueAdjusted( value );
}


//  --- function for setting up signals and slots ---  //
void SliderSpinCombo::makeConnections()
{
    connect( slider,  SIGNAL( valueChanged( int )),    this, SLOT( setSliderValue( int )));
    connect( spinner, SIGNAL( valueChanged( double )), this, SLOT( setSpinnerValue( double )));

}

//  --- setters ---  //
void SliderSpinCombo::setRanges( double lower, double upper)
{

    slider->setRange(lower, upper);
    spinner->setRange(lower, upper);

    if ((upper - lower) > 10)
    {
        slider->setPageStep((upper - lower)/10);
    }

    valueString = QString().sprintf("%4.0f", lower);
    minValueLabel->setText(valueString);

    valueString = QString().sprintf("%4.0f", upper);
    maxValueLabel->setText(valueString);

}

void SliderSpinCombo::setStep(double stepSize)
{
    slider->setSingleStep(stepSize);
}

void SliderSpinCombo::setCurrentValue(double currentValue)
{
    slider->setValue(currentValue);
    spinner->setValue(currentValue);
}
  
  SliderSpinCombo::~SliderSpinCombo()
  {}
  
} // namespace Seg3D
