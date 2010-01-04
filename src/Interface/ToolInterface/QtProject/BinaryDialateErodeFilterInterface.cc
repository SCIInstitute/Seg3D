#include "BinaryDialateErodeFilterInterface.h"
#include "ui_BinaryDialateErodeFilterInterface.h"

BinaryDialateErodeFilterInterface::BinaryDialateErodeFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BinaryDialateErodeFilterInterface)
{
    ui->setupUi(this);
}

BinaryDialateErodeFilterInterface::~BinaryDialateErodeFilterInterface()
{
    delete ui;
}

void BinaryDialateErodeFilterInterface::changeEvent(QEvent *e)
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
