#include "ConfidenceConnectedFilterInterface.h"
#include "ui_ConfidenceConnectedFilterInterface.h"

ConfidenceConnectedFilterInterface::ConfidenceConnectedFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfidenceConnectedFilterInterface)
{
    ui->setupUi(this);
}

ConfidenceConnectedFilterInterface::~ConfidenceConnectedFilterInterface()
{
    delete ui;
}

void ConfidenceConnectedFilterInterface::changeEvent(QEvent *e)
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
