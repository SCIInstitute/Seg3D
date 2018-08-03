#ifndef PHILIPS_LIB_H__
#define PHILIPS_LIB_H__

namespace Seg3D
{
  class Seg3DGui;
}

class QWidget;

namespace Seg3DLibrary
{
  void setupSeg3DQApp(Seg3D::Seg3DGui& app);

  QWidget* makeSeg3DWidget();

  int runSeg3DQAppEventLoop();
}
#endif
