#include "inverttool.h"
#include "ui_inverttool.h"


//  --- constructor ---  //
InvertTool::InvertTool(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InvertTool)
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
void InvertTool::makeConnections()
{
    connect(ui->invertButton, SIGNAL(clicked()), this, SLOT(senseInverted()));
    connect(ui->activeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(senseActiveChanged(int)));
}


//  --- Private slots for custom signals ---  //
void InvertTool::senseActiveChanged(int active)
{
    emit activeChanged(active);
}

void InvertTool::senseInverted()
{
    if(ui->replaceCheckBox->isChecked())
    {
        emit invert(true);
    }
    else
    {
        emit invert(false);
    }
}

//  --- Public slots for setting widget values ---  //
void InvertTool::setActive(int active)
{
    ui->activeComboBox->setCurrentIndex(active);
}

void InvertTool::addToActive(QStringList &items)
{
    ui->activeComboBox->addItems(items);
}

//  --- destructor ---  //
InvertTool::~InvertTool()
{
    delete ui;
}
