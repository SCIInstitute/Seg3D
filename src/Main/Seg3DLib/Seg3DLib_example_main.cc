#include <qapplication.h>
#include <qpushbutton.h>
#include "Lib/Seg3D_lib.h"

void showSeg3DWidget()
{
  Seg3DLibrary::makeSeg3DWidget()->show();
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  auto Context = Seg3DLibrary::Context::makeContext(&app);

  // dummy application code
  QPushButton runSeg3DasWidget("Click to run Seg3D as a widget");
  runSeg3DasWidget.resize(200, 30);
  QObject::connect(&runSeg3DasWidget, &QPushButton::clicked, showSeg3DWidget);
  runSeg3DasWidget.show();
  // end dummy application

  return app.exec();
}
