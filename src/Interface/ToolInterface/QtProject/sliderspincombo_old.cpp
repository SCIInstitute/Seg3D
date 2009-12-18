#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QHBoxLayout>
#include "sliderspincombo.h"

SliderSpinCombo::SliderSpinCombo(QWidget *parent) :
    QWidget(parent)
{

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);


    slider = new QSlider;
    slider->setOrientation(Qt::Horizontal);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setSingleStep(1);
    layout->addWidget( slider );


    spinner = new QSpinBox;
    spinner->setFocusPolicy(Qt::StrongFocus);
    layout->addWidget( spinner );
    createConnections();

}


void SliderSpinCombo::createConnections()
{
    connect(slider, SIGNAL(valueChanged(int)), spinner, SLOT(setValue(int)));
    connect(spinner, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

    emit valueAdjusted(slider->value());
}

void SliderSpinCombo::setRanges(int upper, int lower)
{
    slider->setRange(upper, lower);
    spinner->setRange(upper, lower);
}

void SliderSpinCombo::setCurrentValue(int currentValue)
{
    slider->setValue(currentValue);
    spinner->setValue(currentValue);
}
