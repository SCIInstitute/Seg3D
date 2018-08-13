#ifndef PHILIPS_LIB_H__
#define PHILIPS_LIB_H__

class QWidget;
class QApplication;

namespace Seg3DLibrary
{
  class ContextImpl;

  class Context
  {
  public:
    explicit Context(QApplication* app);
	~Context();

  private:
    ContextImpl* impl_;
  };

  Context* makeContext(QApplication* app);
  //void setupSeg3DQApp(Seg3D::Seg3DGui& app);

  QWidget* makeSeg3DWidget();

  //Keep for testing - remove before giving to Philips
  int runSeg3DQAppEventLoop();

}
#endif
