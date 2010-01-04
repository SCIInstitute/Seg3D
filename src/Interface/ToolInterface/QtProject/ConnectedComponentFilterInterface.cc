#include "ConnectedComponentFilterInterface.h"
#include "ui_ConnectedComponentFilterInterface.h"

ConnectedComponentFilterInterface::ConnectedComponentFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectedComponentFilterInterface)
{
    ui->setupUi(this);
}

ConnectedComponentFilterInterface::~ConnectedComponentFilterInterface()
{
    delete ui;
}

void ConnectedComponentFilterInterface::changeEvent(QEvent *e)
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
