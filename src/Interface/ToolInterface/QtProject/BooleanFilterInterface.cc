#include "BooleanFilterInterface.h"
#include "ui_BooleanFilterInterface.h"

BooleanFilterInterface::BooleanFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BooleanFilterInterface)
{
    ui->setupUi(this);
}

BooleanFilterInterface::~BooleanFilterInterface()
{
    delete ui;
}

void BooleanFilterInterface::changeEvent(QEvent *e)
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
