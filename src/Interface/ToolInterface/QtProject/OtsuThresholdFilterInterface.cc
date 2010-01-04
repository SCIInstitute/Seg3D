#include "OtsuThresholdFilterInterface.h"
#include "ui_OtsuThresholdFilterInterface.h"

OtsuThresholdFilterInterface::OtsuThresholdFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OtsuThresholdFilterInterface)
{
    ui->setupUi(this);
}

OtsuThresholdFilterInterface::~OtsuThresholdFilterInterface()
{
    delete ui;
}

void OtsuThresholdFilterInterface::changeEvent(QEvent *e)
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
