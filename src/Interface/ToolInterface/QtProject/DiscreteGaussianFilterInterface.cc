#include "DiscreteGaussianFilterInterface.h"
#include "ui_DiscreteGaussianFilterInterface.h"

DiscreteGaussianFilterInterface::DiscreteGaussianFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiscreteGaussianFilterInterface)
{
    ui->setupUi(this);
}

DiscreteGaussianFilterInterface::~DiscreteGaussianFilterInterface()
{
    delete ui;
}

void DiscreteGaussianFilterInterface::changeEvent(QEvent *e)
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
