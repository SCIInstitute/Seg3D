#include "fliprotatetool.h"
#include "ui_fliprotatetool.h"

//  --- constructor ---  //
FlipRotateTool::FlipRotateTool(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::FlipRotateTool)
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
void FlipRotateTool::makeConnections()
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
void FlipRotateTool::senseActiveChanged(int active)
{
    emit activeChanged(active);
}

void FlipRotateTool::senseFlipAxial()
{
    emit flipAxial();
}

void FlipRotateTool::senseFlipCoronal()
{
    emit flipCoronal();
}

void FlipRotateTool::senseFlipSagittal()
{
    emit flipSagittal();
}

void FlipRotateTool::senseRotateAxialCoronal()
{
    emit rotateAxialCoronal();
}

void FlipRotateTool::senseRotateAxialSagital()
{
     emit rotateAxialSagittal();
}

void FlipRotateTool::senseRotateSagittalAxial()
{
    emit rotateSagittalAxial();
}

//  --- Public slots for setting widget values ---  //
void FlipRotateTool::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void FlipRotateTool::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

//  --- destructor ---  //
FlipRotateTool::~FlipRotateTool()
{
    delete ui;
}
