#include "FlipToolInterface.h"
#include "ui_FlipToolInterface.h"

//  --- constructor ---  //
FlipToolInterface::FlipToolInterface(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::FlipToolInterface)
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
void FlipToolInterface::makeConnections()
{
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));

    connect(ui->flipAxialButton, SIGNAL(clicked()), this, SLOT(senseFlipAxial()));
    connect(ui->flipCoronalButton, SIGNAL(clicked()), this, SLOT(senseFlipCoronal()));
    connect(ui->flipSagittalButton, SIGNAL(clicked()), this, SLOT(senseFlipSagittal()));

    connect(ui->rotateACButton, SIGNAL(clicked()), this, SLOT(senseRotateAxialCoronal()));
    connect(ui->rotateASButton, SIGNAL(clicked()), this, SLOT(senseRotateAxialSagital()));
    connect(ui->rotateSAButton, SIGNAL(clicked()), this, SLOT(senseRotateSagittalAxial()));
}


//  --- Private slots for custom signals ---  //
void FlipToolInterface::senseActiveChanged(int active)
{
    emit activeChanged(active);
}

void FlipToolInterface::senseFlipAxial()
{
    emit flipAxial();
}

void FlipToolInterface::senseFlipCoronal()
{
    emit flipCoronal();
}

void FlipToolInterface::senseFlipSagittal()
{
    emit flipSagittal();
}

void FlipToolInterface::senseRotateAxialCoronal()
{
    emit rotateAxialCoronal();
}

void FlipToolInterface::senseRotateAxialSagital()
{
     emit rotateAxialSagittal();
}

void FlipToolInterface::senseRotateSagittalAxial()
{
    emit rotateSagittalAxial();
}

//  --- Public slots for setting widget values ---  //
void FlipToolInterface::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void FlipToolInterface::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

//  --- destructor ---  //
FlipToolInterface::~FlipToolInterface()
{
    delete ui;
}
