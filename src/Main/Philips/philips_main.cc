#include <qapplication.h>
#include <qpushbutton.h>

#include <Interface/Application/ApplicationInterface.h>
#include <Core/Application/Application.h>
#include <QtUtils/Utils/QtApplication.h>
#include <Main/Seg3DGui.h>

using namespace Seg3D;

void setupSeg3DQApp(Seg3DGui& app)
{
  const char* cmdline[] = {""};
  int argc = 0;

  Core::Application::Instance()->parse_command_line_parameters( argc, const_cast<char**>(cmdline) );

  QtUtils::QtApplication::Instance()->setup( argc, const_cast<char**>(cmdline));

  app.initialize();
}

QWidget* makeSeg3DWidget()
{
  return new ApplicationInterface();
}

void showSeg3DWidget()
{
  makeSeg3DWidget()->show();
}

int runSeg3DQAppEventLoop()
{
  return QtUtils::QtApplication::Instance()->exec();
}

int main(int argc, char *argv[])
{
  Seg3DGui app;
  setupSeg3DQApp(app);

  // dummy application code
  QPushButton runSeg3DasWidget("Click to run Seg3D as a widget");
  runSeg3DasWidget.resize(200, 30);
  QObject::connect(&runSeg3DasWidget, &QPushButton::clicked, showSeg3DWidget);
  runSeg3DasWidget.show();
  // end dummy application

  return runSeg3DQAppEventLoop();
}
