#include "polylinetool.h"
#include "ui_polylinetool.h"

//  --- constructor ---  //
PolyLineTool::PolyLineTool(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PolyLineTool)
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
void PolyLineTool::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));

    connect(ui->resetPLButton, SIGNAL(clicked()), this, SLOT(senseResetPolyLineTool()));
    connect(ui->insideFillButton, SIGNAL(clicked()), this, SLOT(senseFillPolyLine()));
    connect(ui->insideEraseButton, SIGNAL(clicked()), this, SLOT(senseErasePolyLine()));
}


//  --- Private slots for custom signals ---  //
void PolyLineTool::senseActiveChanged(int active)
{
    emit activeChanged(active);
}

void PolyLineTool::senseResetPolyLineTool()
{
    emit resetPolyLineTool();
}

void PolyLineTool::senseFillPolyLine()
{
    emit fillPolyLine();
}

void PolyLineTool::senseErasePolyLine()
{
    emit erasePolyLine();
}

//  --- Public slots for setting widget values ---  //
void PolyLineTool::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void PolyLineTool::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}


//  --- destructor ---  //
PolyLineTool::~PolyLineTool()
{
    delete ui;
}

