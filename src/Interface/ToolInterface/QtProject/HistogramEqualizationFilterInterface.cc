#include "HistogramEqualizationFilterInterface.h"
#include "ui_HistogramEqualizationFilterInterface.h"

HistogramEqualizationFilterInterface::HistogramEqualizationFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HistogramEqualizationFilterInterface)
{
    ui->setupUi(this);
}

HistogramEqualizationFilterInterface::~HistogramEqualizationFilterInterface()
{
    delete ui;
}

void HistogramEqualizationFilterInterface::changeEvent(QEvent *e)
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
