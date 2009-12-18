#include "ThresholdToolInterface.h"
#include "sliderspincombo.h"
#include "ui_ThresholdToolInterface.h"

//  --- constructor ---  //
ThresholdToolInterface::ThresholdToolInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThresholdToolInterface)
{
    ui->setupUi(this);

    upperThresholdAdjuster = new SliderSpinCombo();
    ui->upperHLayout_bottom->addWidget(upperThresholdAdjuster);

    lowerThresholdAdjuster = new SliderSpinCombo();
    ui->lowerHLayout_bottom->addWidget(lowerThresholdAdjuster);

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
void ThresholdToolInterface::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));

    connect(upperThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseUpperThresholdChanged(double)));
    connect(lowerThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senselowerThresholdChanged(double)));

    connect(ui->createThresholdLayerButton, SIGNAL(clicked()), this, SLOT(senseCreateThresholdLayer()));
    connect(ui->clearSeedsButton, SIGNAL(clicked()), this, SLOT(senseClearSeeds()));
}

//  --- Private slots for custom signals ---  //
void ThresholdToolInterface::senseActiveChanged(int active)
{
    emit activeChanged(active);
}

void ThresholdToolInterface::senseCreateThresholdLayer()
{
    emit createThresholdLayer();
}

void ThresholdToolInterface::senseClearSeeds()
{
    emit clearSeeds();
}

void ThresholdToolInterface::senseUpperThresholdChanged(double upper)
{
    emit upperThresholdChanged(upper);
}

void ThresholdToolInterface::senselowerThresholdChanged(double lower)
{
    emit lowerThresholdChanged(lower);
}

//  --- Public slots for setting widget values ---  //
void ThresholdToolInterface::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void ThresholdToolInterface::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

void ThresholdToolInterface::setLowerThreshold(double lower, double upper)
{
    lowerThresholdAdjuster->setRanges(lower, upper);
}

void ThresholdToolInterface::setLowerThresholdStep(double step)
{
    lowerThresholdAdjuster->setStep(step);
}

void ThresholdToolInterface::setUpperThreshold(double lower, double upper)
{
    upperThresholdAdjuster->setRanges(lower, upper);
}

void ThresholdToolInterface::setUpperThresholdStep(double step)
{
    upperThresholdAdjuster->setStep(step);
}

void ThresholdToolInterface::setHistogram()
{
    //TODO - implement histogram display
}



//  --- destructor ---  //
ThresholdToolInterface::~ThresholdToolInterface()
{
    delete ui;
}


