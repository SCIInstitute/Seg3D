#include "IntensityCorrectionFilterInterface.h"
#include "ui_IntensityCorrectionFilterInterface.h"

IntensityCorrectionFilterInterface::IntensityCorrectionFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IntensityCorrectionFilterInterface)
{
    ui->setupUi(this);
}

IntensityCorrectionFilterInterface::~IntensityCorrectionFilterInterface()
{
    delete ui;
}

void IntensityCorrectionFilterInterface::changeEvent(QEvent *e)
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
