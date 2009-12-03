#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H


// QT includes
#include <QtGui>
#include <QDockWidget>

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
    ~ViewerInterface();
//    void writeSizeSettings();
//    void readSizeSettings();

public Q_SLOTS:
    void setViews(int, int);
    void setFocusedView(int);

private:
    
  boost::shared_ptr<ViewerInterfacePrivate> private_;



};

} // end namespace  
#endif // VIEWERINTERFACE_H