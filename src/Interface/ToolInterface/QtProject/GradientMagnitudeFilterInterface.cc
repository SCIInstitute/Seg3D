#include "GradientMagnitudeFilterInterface.h"
#include "ui_GradientMagnitudeFilterInterface.h"

GradientMagnitudeFilterInterface::GradientMagnitudeFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GradientMagnitudeFilterInterface)
{
    ui->setupUi(this);
}

GradientMagnitudeFilterInterface::~GradientMagnitudeFilterInterface()
{
    delete ui;
}

void GradientMagnitudeFilterInterface::changeEvent(QEvent *e)
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
