#include "FlipToolInterface.h"
#include "ui_FlipToolInterface.h"

//  --- constructor ---  //
FlipToolInterface::FlipToolInterface( QWidget *parent ) :
  QWidget( parent ), ui( new Ui::FlipToolInterface )
{
  ui->setupUi( this );

}

//  --- destructor ---  //
FlipToolInterface::~FlipToolInterface()
{
  delete ui;
}
