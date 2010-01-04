#include "CannyEdgeDetectionFilterInterface.h"
#include "ui_CannyEdgeDetectionFilterInterface.h"

CannyEdgeDetectionFilterInterface::CannyEdgeDetectionFilterInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CannyEdgeDetectionFilterInterface)
{
    ui->setupUi(this);
}

CannyEdgeDetectionFilterInterface::~CannyEdgeDetectionFilterInterface()
{
    delete ui;
}

void CannyEdgeDetectionFilterInterface::changeEvent(QEvent *e)
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
