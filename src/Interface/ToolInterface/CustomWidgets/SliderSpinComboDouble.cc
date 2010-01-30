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

// Qt Interface Includes
#include <QtGui>
#include <Utils/Core/Log.h>

#include <Interface/ToolInterface/CustomWidgets/SliderSpinComboDouble.h>

namespace Seg3D {


//TODO -- implement slider handling for non integer values

//  --- Start constructor ---  //
SliderSpinComboDouble::SliderSpinComboDouble( QWidget *parent )
  : QWidget( parent )
{
  buildWidget();
  makeConnections();
  setRanges( 1, 100);
  setStep(1);
  setCurrentValue( 1 );   
}
//  --- End Constructors ---  //


  //  --- function for visually building the widget ---  //
  void SliderSpinComboDouble::buildWidget()
  {
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(2);
    mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
    mainLayout->setContentsMargins(0,0,2,0);

    sliderSideLayout = new QVBoxLayout();
    sliderSideLayout->setSpacing(0);
    sliderSideLayout->setObjectName(QString::fromUtf8("sliderSideLayout"));
    sliderSideLayout->setContentsMargins(0,0,0,0);

    slider = new QSlider(this);
    slider->setObjectName(QString::fromUtf8("slider"));
    slider->setOrientation(Qt::Horizontal);
    slider->setTickPosition(QSlider::TicksBelow);

    sliderSideLayout->addWidget(slider);

    labelLayout = new QHBoxLayout();
    labelLayout->setSpacing(0);
    labelLayout->setObjectName(QString::fromUtf8("labelLayout"));
    labelLayout->setContentsMargins(2,0,6,0);

    minValueLabel = new QLabel(this);
    minValueLabel->setObjectName(QString::fromUtf8("minValueLabel"));
    minValueLabel->setMinimumSize(QSize(0,12));
    minValueLabel->setMaximumSize(QSize(16777215,12));

    labelLayout->addWidget(minValueLabel);

    maxValueLabel = new QLabel(this);
    maxValueLabel->setObjectName(QString::fromUtf8("maxValueLabel"));
    maxValueLabel->setMinimumSize(QSize(0,12));
    maxValueLabel->setMaximumSize(QSize(16777215,12));
    maxValueLabel->setLayoutDirection(Qt::RightToLeft);

    labelLayout->addWidget(maxValueLabel);

    sliderSideLayout->addLayout(labelLayout);

    mainLayout->addLayout(sliderSideLayout);

    spinnerSideLayout = new QVBoxLayout();
    spinnerSideLayout->setSpacing(0);
    spinnerSideLayout->setObjectName(QString::fromUtf8("spinnerSideLayout"));
    spinnerSideLayout->setContentsMargins(0,0,0,0);
    spinner = new QDoubleSpinBox(this);
    spinner->setObjectName(QString::fromUtf8("spinner"));

    spinnerSideLayout->addWidget(spinner);

    verticalSpacer = new QSpacerItem(10, 15, QSizePolicy::Fixed, QSizePolicy::Fixed);

    spinnerSideLayout->addItem(verticalSpacer);

    mainLayout->addLayout(spinnerSideLayout);

  } // end buildWidget


  //  --- Private slots --- //
  void SliderSpinComboDouble::setSliderValue( int value )
  { 
    //TODO - need to do conversions because sliders are int only
    double new_value;
    new_value = value;
    setCurrentValue( value );
    Q_EMIT valueAdjusted( new_value );
  }
  void SliderSpinComboDouble::setSpinnerValue( double value )
  {
    setCurrentValue( value );
  }


  //  --- function for setting up signals and slots ---  //
  void SliderSpinComboDouble::makeConnections()
  {
    connect( slider,  SIGNAL( valueChanged( int )),    this, SLOT( setSliderValue( int )));
    connect( spinner, SIGNAL( valueChanged( double )), this, SLOT( setSpinnerValue( double )));
  } // end makeConnections



  //  --- setters ---  //
  void SliderSpinComboDouble::setRanges( double lower, double upper)
  {
    slider->setRange(lower, upper);
    spinner->setRange(lower, upper);

    if ((upper - lower) > 10)
    {
        slider->setPageStep((upper - lower)/10);
    }

    QString valueString = QString().sprintf("%4.0f", lower);
    minValueLabel->setText(valueString);

    valueString = QString().sprintf("%4.0f", upper);
    maxValueLabel->setText(valueString);
    
  } // end setRanges

  void SliderSpinComboDouble::setStep(double stepSize)
  {
    slider->setSingleStep(stepSize);
  } // end setStep

  void SliderSpinComboDouble::setCurrentValue(double currentValue)
  {
    slider->setValue(currentValue);
    spinner->setValue(currentValue);
  } // end setCurrentValue

  //virtual destructor
  SliderSpinComboDouble::~SliderSpinComboDouble()
  {}
  
} // namespace Seg3D
