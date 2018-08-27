#include "Seg3D_lib.h"
#include <Interface/Application/ApplicationInterface.h>
#include <Core/Application/Application.h>
#include <QtUtils/Utils/QtApplication.h>
#include <Main/Seg3DGui.h>
#include <Application/LayerIO/Actions/ActionImportLayer.h>
#include <Application/LayerIO/Actions/ActionExportSegmentation.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/Layer/LayerManager.h>

using namespace Seg3D;

class Seg3DLibrary::ContextImpl
{
public:
	Seg3DGui gui;
};

void Seg3DLibrary::importDataFile(const QString& data_filename)
{
  const std::string filename = data_filename.toStdString();
  const std::string importer = "Teem Importer";

  ActionImportLayer::Dispatch(Core::Interface::GetWidgetActionContext(),
    filename, LayerIO::DATA_MODE_C, importer);
}

void Seg3DLibrary::importSegFile(const QString& seg_filename)
{
  const std::string filename = seg_filename.toStdString();
  const std::string importer = "Teem Importer";

  ActionImportLayer::Dispatch(Core::Interface::GetWidgetActionContext(),
    filename, LayerIO::LABEL_MASK_MODE_C, importer);
}

void Seg3DLibrary::exportSegFile(const QString& export_path)
{
  const std::string path = export_path.toStdString();
  const std::string extension = ".nrrd";
  std::vector< LayerHandle > layers;
  Seg3D::LayerManager::Instance()->get_layers(layers);

  for(int i = 0; i < layers.size(); i++)
  {
    if (layers[i]->get_type() == Core::VolumeType::MASK_E)
    {
      std::string temp_filename = layers[i]->get_layer_name();
      std::string temp_layer = layers[i]->get_layer_id();

      std::string temp_complete_path = path + temp_filename;

      ActionExportSegmentation::Dispatch(Core::Interface::GetWidgetActionContext(),
        temp_layer, LayerIO::LABEL_MASK_MODE_C, temp_complete_path, extension);
    }
  }
}

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