#include "NeighborhoodConnectedFilterInterface.h"
#include "ui_NeighborhoodConnectedFilterInterface.h"

NeighborhoodConnectedFilterInterface::NeighborhoodConnectedFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NeighborhoodConnectedFilterInterface)
{
    ui->setupUi(this);
}

NeighborhoodConnectedFilterInterface::~NeighborhoodConnectedFilterInterface()
{
    delete ui;
}

void NeighborhoodConnectedFilterInterface::changeEvent(QEvent *e)
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
