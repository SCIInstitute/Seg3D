#include "philips_lib.h"
#include <Interface/Application/ApplicationInterface.h>
#include <Core/Application/Application.h>
#include <QtUtils/Utils/QtApplication.h>
#include <Main/Seg3DGui.h>

using namespace Seg3D;

void Seg3DLibrary::setupSeg3DQApp(Seg3DGui& app)
{
  const char* cmdline[] = {""};
  int argc = 0;

  Core::Application::Instance()->parse_command_line_parameters( argc, const_cast<char**>(cmdline) );

  QtUtils::QtApplication::Instance()->setExternalInstance(qApp);

  QtUtils::QtApplication::Instance()->setup( argc, const_cast<char**>(cmdline));

  app.initialize();
}

QWidget* Seg3DLibrary::makeSeg3DWidget()
{
  return new ApplicationInterface();
}

int Seg3DLibrary::runSeg3DQAppEventLoop()
{
  return QtUtils::QtApplication::Instance()->exec();
}
