#include "paintbrushtool.h"
#include "ui_paintbrushtool.h"
#include "sliderspincombo.h"


//  --- constructor ---  //
PaintBrushTool::PaintBrushTool(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaintBrushTool)
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
void PaintBrushTool::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
    connect(ui->maskComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseMaskChanged(int)));
    connect(ui->eraseCheckBox, SIGNAL(toggled(bool)), this, SLOT(senseEraseModeChanged(bool)));


    connect(paintBrushAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(sensePaintBrushSizeChanged(double)));
    connect(upperThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senseUpperThresholdChanged(double)));
    connect(lowerThresholdAdjuster, SIGNAL(valueAdjusted(double)), this, SLOT(senselowerThresholdChanged(double)));
}


//  --- Private slots for custom signals ---  //
void PaintBrushTool::senseActiveChanged(int active)
{
    ui->maskComboBox->setCurrentIndex(0);
    emit activeChanged( active );
}

void PaintBrushTool::senseMaskChanged(int mask)
{
    emit maskChanged(mask);
}

void PaintBrushTool::senseEraseModeChanged(bool mode)
{
    emit eraseModeChanged(mode);
}

void PaintBrushTool::sensePaintBrushSizeChanged(double size)
{
    emit paintBrushSizeChanged(size);
}

void PaintBrushTool::senseUpperThresholdChanged(double upper)
{
    emit  upperThresholdChanged(upper);
}

void PaintBrushTool::senselowerThresholdChanged(double lower)
{
    emit lowerThresholdChanged(lower);
}

//  --- Public slots for setting widget values ---  //
void PaintBrushTool::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void PaintBrushTool::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

void PaintBrushTool::setMask(int mask)
{
    ui->maskComboBox->setCurrentIndex(mask);
}

void PaintBrushTool::addToMask(QStringList &items)
{
    ui->maskComboBox->addItems(items);
}

void PaintBrushTool::setPaintBrushSize(int size)
{
    paintBrushAdjuster->setCurrentValue(size);
}

void PaintBrushTool::setLowerThreshold(double lower, double upper)
{
    lowerThresholdAdjuster->setRanges(lower, upper);
}

void PaintBrushTool::setLowerThresholdStep(double step)
{
    lowerThresholdAdjuster->setStep(step);
}

void PaintBrushTool::setUpperThreshold(double lower, double upper)
{
    upperThresholdAdjuster->setRanges(lower, upper);
}

void PaintBrushTool::setUpperThresholdStep(double step)
{
    upperThresholdAdjuster->setStep(step);
}


//  --- destructor ---  //
PaintBrushTool::~PaintBrushTool()
{
    delete ui;
}


