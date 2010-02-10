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

}


AnisotropicDiffusionFilterInterface::~AnisotropicDiffusionFilterInterface()
{
    delete ui;
}
