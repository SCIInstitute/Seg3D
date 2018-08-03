#include <qapplication.h>
#include <qpushbutton.h>

#include <Main/Seg3DGui.h>

#include "Lib/philips_lib.h"

using namespace Seg3D;

void showSeg3DWidget()
{
  Seg3DLibrary::makeSeg3DWidget()->show();
}

int main(int argc, char *argv[])
{
  Seg3DGui app;
  Seg3DLibrary::setupSeg3DQApp(app);

  // dummy application code
  QPushButton runSeg3DasWidget("Click to run Seg3D as a widget");
  runSeg3DasWidget.resize(200, 30);
  QObject::connect(&runSeg3DasWidget, &QPushButton::clicked, showSeg3DWidget);
  runSeg3DasWidget.show();
  // end dummy application

  return Seg3DLibrary::runSeg3DQAppEventLoop();
}
