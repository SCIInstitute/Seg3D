#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

#include "sliderspincombo.h"


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
}

SliderSpinCombo::SliderSpinCombo( QWidget *parent, double minRange, double maxRange, double startValue, double stepSize )
    : QWidget( parent )
{
    buildWidget();
    makeConnections();
    setRanges( minRange, maxRange);
    setStep(stepSize);
    setCurrentValue( startValue );
}

SliderSpinCombo::SliderSpinCombo( QWidget *parent, double minRange, double maxRange, double stepSize )
    : QWidget( parent )
{
    buildWidget();
    makeConnections();
    setRanges( minRange, maxRange);
    setStep(stepSize);
    setCurrentValue( minRange );
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
    emit valueAdjusted( value );
}
void SliderSpinCombo::setSpinnerValue( double value )
{
    setCurrentValue( value );
    emit valueAdjusted( value );
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
