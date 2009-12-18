#include "PolyLineToolInterface.h"
#include "ui_PolyLineToolInterface.h"

//  --- constructor ---  //
PolyLineToolInterface::PolyLineToolInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PolyLineToolInterface)
{
    ui->setupUi(this);
    makeConnections();

    // sample data for displaying in layer box
    QStringList layers;
    layers << "Mouse Layer 01" << "Mouse Layer 02"
           << "Mouse Layer 03" << "Mouse Layer 04"
           << "Mouse Layer 05" << "Mouse Layer 06"
           << "Mouse Layer 07" << "Mouse Layer 08";
    addToActive(layers);
}

//  --- Function for making signal slots connections ---  //
void PolyLineToolInterface::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));

    connect(ui->resetPLButton, SIGNAL(clicked()), this, SLOT(senseResetPolyLineTool()));
    connect(ui->insideFillButton, SIGNAL(clicked()), this, SLOT(senseFillPolyLine()));
    connect(ui->insideEraseButton, SIGNAL(clicked()), this, SLOT(senseErasePolyLine()));
}


//  --- Private slots for custom signals ---  //
void PolyLineToolInterface::senseActiveChanged(int active)
{
    emit activeChanged(active);
}

void PolyLineToolInterface::senseResetPolyLineTool()
{
    emit resetPolyLineTool();
}

void PolyLineToolInterface::senseFillPolyLine()
{
    emit fillPolyLine();
}

void PolyLineToolInterface::senseErasePolyLine()
{
    emit erasePolyLine();
}

//  --- Public slots for setting widget values ---  //
void PolyLineToolInterface::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void PolyLineToolInterface::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}


//  --- destructor ---  //
PolyLineToolInterface::~PolyLineToolInterface()
{
    delete ui;
}

