#include "Seg3D_lib.h"
#include <Interface/Application/ApplicationInterface.h>
#include <Core/Application/Application.h>
#include <QtUtils/Utils/QtApplication.h>
#include <Main/Seg3DGui.h>

using namespace Seg3D;

class Seg3DLibrary::ContextImpl
{
public:
	Seg3DGui gui;
};

void setupSeg3DQApp(Seg3DGui& gui, QApplication* app)
{
  const char* cmdline[] = {""};
  int argc = 0;

  Core::Application::Instance()->parse_command_line_parameters( argc, const_cast<char**>(cmdline) );

  QtUtils::QtApplication::Instance()->setExternalInstance(app);

  QtUtils::QtApplication::Instance()->setup( argc, const_cast<char**>(cmdline));

  gui.initialize();
}

Seg3DLibrary::Context::Context(QApplication* app) : impl_(new ContextImpl)
{
	setupSeg3DQApp(impl_->gui, app);
}

Seg3DLibrary::Context::~Context()
{
  if (impl_)
  {
    impl_->gui.close();
	delete impl_;
	impl_ = nullptr;
  }
}

std::unique_ptr<Seg3DLibrary::Context> Seg3DLibrary::Context::makeContext(QApplication* app)
{
	return std::unique_ptr<Seg3DLibrary::Context>(new Context(app));
}

QWidget* Seg3DLibrary::makeSeg3DWidget()
{
  return new ApplicationInterface();
}