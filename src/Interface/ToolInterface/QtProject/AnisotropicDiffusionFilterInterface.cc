#include "AnisotropicDiffusionFilterInterface.h"
#include "ui_AnisotropicDiffusionFilterInterface.h"

AnisotropicDiffusionFilterInterface::AnisotropicDiffusionFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnisotropicDiffusionFilterInterface)
{
    ui->setupUi(this);

    iterationsAdjuster = new SliderSpinCombo();
    ui->iterationsHLayout_bottom->addWidget(iterationsAdjuster);

    stepAdjuster = new SliderSpinCombo();
    ui->integrationHLayout_bottom->addWidget(stepAdjuster);

    conductanceAdjuster = new SliderSpinCombo();
    ui->conductanceHLayout_bottom->addWidget(conductanceAdjuster);

    makeConnections();

    // sample data for displaying in layer box
    QStringList layers;
    layers << "Mouse Layer 01" << "Mouse Layer 02"
           << "Mouse Layer 03" << "Mouse Layer 04"
           << "Mouse Layer 05" << "Mouse Layer 06"
           << "Mouse Layer 07" << "Mouse Layer 08";
    addToActive(layers);
}

//  --- Function for making signal slots connections ---  //
void AnisotropicDiffusionFilterInterface::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));

}

//  --- Private slots for custom signals ---  //
void AnisotropicDiffusionFilterInterface::senseActiveChanged(int active)
{
    emit activeChanged( active );
}

void AnisotropicDiffusionFilterInterface::senseIterationsChanged(int iterations)
{
    emit iterationsChanged(iterations);
}

void AnisotropicDiffusionFilterInterface::senseStepChanged(double step)
{
    emit stepChanged(step);
}

void AnisotropicDiffusionFilterInterface::senseConductanceChanged(int conductance)
{
    emit conductanceChanged(conductance);
}

void AnisotropicDiffusionFilterInterface::senseFilterRun(bool)
{
    if(ui->replaceCheckBox->isChecked())
    {
        emit filterRun(true);
    }
    else
    {
        emit filterRun(false);
    }
}


//  --- Public slots for setting widget values ---  //
void AnisotropicDiffusionFilterInterface::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void AnisotropicDiffusionFilterInterface::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

void AnisotropicDiffusionFilterInterface::setIterations(int iterations)
{
    iterationsAdjuster->setCurrentValue(iterations);
}

void AnisotropicDiffusionFilterInterface::setIterationRange(int lower, int upper)
{
    iterationsAdjuster->setRanges(lower, upper);
}

void AnisotropicDiffusionFilterInterface::setStep(double iterations)
{
    stepAdjuster->setCurrentValue(iterations);
}

void AnisotropicDiffusionFilterInterface::setStepRange(double lower, double upper)
{
    stepAdjuster->setRanges(lower, upper);
}

void AnisotropicDiffusionFilterInterface::setConductance(int iterations)
{
    conductanceAdjuster->setCurrentValue(iterations);
}

void AnisotropicDiffusionFilterInterface::setConductanceRange(int lower, int upper)
{
    conductanceAdjuster->setRanges(lower, upper);
}


AnisotropicDiffusionFilterInterface::~AnisotropicDiffusionFilterInterface()
{
    delete ui;
}
