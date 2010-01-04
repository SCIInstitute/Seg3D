#include "ThresholdSegmentationLSFilterInterface.h"
#include "ui_ThresholdSegmentationLSFilterInterface.h"

ThresholdSegmentationLSFilterInterface::ThresholdSegmentationLSFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThresholdSegmentationLSFilterInterface)
{
    ui->setupUi(this);
}

ThresholdSegmentationLSFilterInterface::~ThresholdSegmentationLSFilterInterface()
{
    delete ui;
}

void ThresholdSegmentationLSFilterInterface::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
