#include "ArithmeticFilterInterface.h"
#include "ui_ArithmeticFilterInterface.h"

ArithmeticFilterInterface::ArithmeticFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ArithmeticFilterInterface)
{
    ui->setupUi(this);

}

ArithmeticFilterInterface::~ArithmeticFilterInterface()
{
    delete ui;
}

void ArithmeticFilterInterface::changeEvent(QEvent *e)
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
