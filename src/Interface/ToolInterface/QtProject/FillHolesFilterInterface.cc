#include "FillHolesFilterInterface.h"
#include "ui_FillHolesFilterInterface.h"

FillHolesFilterInterface::FillHolesFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FillHolesFilterInterface)
{
    ui->setupUi(this);
}

FillHolesFilterInterface::~FillHolesFilterInterface()
{
    delete ui;
}

void FillHolesFilterInterface::changeEvent(QEvent *e)
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
