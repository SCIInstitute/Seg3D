#include "MedianFilter.h"
#include "ui_MedianFilter.h"

MedianFilter::MedianFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MedianFilter)
{
    ui->setupUi(this);
}

MedianFilter::~MedianFilter()
{
    delete ui;
}

void MedianFilter::changeEvent(QEvent *e)
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
