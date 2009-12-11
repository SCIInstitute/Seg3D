#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H


// QT includes
#include <QtGui>


// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D {

class ViewerInterfacePrivate;

class ViewerInterface : public QWidget {
    
  Q_OBJECT

public:
  ViewerInterface(QWidget *parent = 0);
  virtual ~ViewerInterface();
  
  void writeSizeSettings();
  void readSizeSettings();
  

public Q_SLOTS:
  void set_views(int, int);
  void set_focused_view(int);

    

private:
    
  boost::shared_ptr<ViewerInterfacePrivate> private_;



};

} // end namespace  
#endif // VIEWERINTERFACE_H