#include "anisotropicdiffusionfilter.h"
#include "ui_anisotropicdiffusionfilter.h"

AnisotropicDiffusionFilter::AnisotropicDiffusionFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnisotropicDiffusionFilter)
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
void AnisotropicDiffusionFilter::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));

}

//  --- Private slots for custom signals ---  //
void AnisotropicDiffusionFilter::senseActiveChanged(int active)
{
    emit activeChanged( active );
}

void AnisotropicDiffusionFilter::senseIterationsChanged(int iterations)
{
    emit iterationsChanged(iterations);
}

void AnisotropicDiffusionFilter::senseStepChanged(double step)
{
    emit stepChanged(step);
}

void AnisotropicDiffusionFilter::senseConductanceChanged(int conductance)
{
    emit conductanceChanged(conductance);
}

void AnisotropicDiffusionFilter::senseFilterRun(bool)
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
void AnisotropicDiffusionFilter::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void AnisotropicDiffusionFilter::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

void AnisotropicDiffusionFilter::setIterations(int iterations)
{
    iterationsAdjuster->setCurrentValue(iterations);
}

void AnisotropicDiffusionFilter::setIterationRange(int lower, int upper)
{
    iterationsAdjuster->setRanges(lower, upper);
}

void AnisotropicDiffusionFilter::setStep(double iterations)
{
    stepAdjuster->setCurrentValue(iterations);
}

void AnisotropicDiffusionFilter::setStepRange(double lower, double upper)
{
    stepAdjuster->setRanges(lower, upper);
}

void AnisotropicDiffusionFilter::setConductance(int iterations)
{
    conductanceAdjuster->setCurrentValue(iterations);
}

void AnisotropicDiffusionFilter::setConductanceRange(int lower, int upper)
{
    conductanceAdjuster->setRanges(lower, upper);
}


AnisotropicDiffusionFilter::~AnisotropicDiffusionFilter()
{
    delete ui;
}
