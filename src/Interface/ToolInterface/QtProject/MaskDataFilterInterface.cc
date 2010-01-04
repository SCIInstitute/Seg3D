#include "MaskDataFilterInterface.h"
#include "ui_MaskDataFilterInterface.h"

MaskDataFilterInterface::MaskDataFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MaskDataFilterInterface)
{
    ui->setupUi(this);
}

MaskDataFilterInterface::~MaskDataFilterInterface()
{
    delete ui;
}

void MaskDataFilterInterface::changeEvent(QEvent *e)
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
