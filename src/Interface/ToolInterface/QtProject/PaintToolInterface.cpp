#include "PaintToolInterface.h"
#include "ui_PaintToolInterface.h"
#include "sliderspincombo.h"


//  --- constructor ---  //
PaintToolInterface::PaintToolInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaintToolInterface)
{
    ui->setupUi(this);

    paintBrushAdjuster = new SliderSpinCombo(parent, 1, 100, 1);
    ui->verticalLayout->addWidget(paintBrushAdjuster);

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

    // sample data for displaying in layer box
    QStringList masks;
    masks  << "not selected"
           << "Mouse Layer 01" << "Mouse Layer 02"
           << "Mouse Layer 03" << "Mouse Layer 04"
           << "Mouse Layer 05" << "Mouse Layer 06"
           << "Mouse Layer 07" << "Mouse Layer 08";
    addToMask(masks);


}


//  --- Function for making signal slots connections ---  //
void PaintToolInterface::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
    connect(ui->maskComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseMaskChanged(int)));
    connect(ui->eraseCheckBox, SIGNAL(toggled(bool)), this, SLOT(senseEraseModeChanged(bool)));


    connect(paintBrushAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(sensePaintBrushSizeChanged(double)));
    connect(upperThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseUpperThresholdChanged(double)));
    connect(lowerThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senselowerThresholdChanged(double)));
}


//  --- Private slots for custom signals ---  //
void PaintToolInterface::senseActiveChanged(int active)
{
    ui->maskComboBox->setCurrentIndex(0);
    emit activeChanged( active );
}

void PaintToolInterface::senseMaskChanged(int mask)
{
    emit maskChanged(mask);
}

void PaintToolInterface::senseEraseModeChanged(bool mode)
{
    emit eraseModeChanged(mode);
}

void PaintToolInterface::sensePaintBrushSizeChanged(double size)
{
    emit paintBrushSizeChanged(size);
}

void PaintToolInterface::senseUpperThresholdChanged(double upper)
{
    emit  upperThresholdChanged(upper);
}

void PaintToolInterface::senselowerThresholdChanged(double lower)
{
    emit lowerThresholdChanged(lower);
}

//  --- Public slots for setting widget values ---  //
void PaintToolInterface::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void PaintToolInterface::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

void PaintToolInterface::setMask(int mask)
{
    ui->maskComboBox->setCurrentIndex(mask);
}

void PaintToolInterface::addToMask(QStringList &items)
{
    ui->maskComboBox->addItems(items);
}

void PaintToolInterface::setPaintBrushSize(int size)
{
    paintBrushAdjuster->setCurrentValue(size);
}

void PaintToolInterface::setLowerThreshold(double lower, double upper)
{
    lowerThresholdAdjuster->setRanges(lower, upper);
}

void PaintToolInterface::setLowerThresholdStep(double step)
{
    lowerThresholdAdjuster->setStep(step);
}

void PaintToolInterface::setUpperThreshold(double lower, double upper)
{
    upperThresholdAdjuster->setRanges(lower, upper);
}

void PaintToolInterface::setUpperThresholdStep(double step)
{
    upperThresholdAdjuster->setStep(step);
}


//  --- destructor ---  //
PaintToolInterface::~PaintToolInterface()
{
    delete ui;
}


