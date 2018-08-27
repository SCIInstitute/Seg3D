#ifndef SEG3D_LIB_H__
#define SEG3D_LIB_H__

#include <memory>
#include <QString>

class QWidget;
class QApplication;

namespace Seg3DLibrary
{
	class ContextImpl;

	class Context
	{
	public:
		~Context();
		static std::unique_ptr<Context> makeContext(QApplication* app);

	private:
		ContextImpl* impl_;
		explicit Context(QApplication* app);
		Context(const Context&);
		Context& operator=(const Context&);
	};

	QWidget* makeSeg3DWidget();
  void importDataFile(const QString& data_filename);
  void importSegFile(const QString& seg_filename);
  void exportSegFile(const QString& export_path);
}
#endif
