#ifndef LAYERMANAGERDOCKWIDGET_H
#define LAYERMANAGERDOCKWIDGET_H

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
  
class LayerManagerDockWidgetPrivate;

class LayerManagerDockWidget : public QDockWidget {

    Q_OBJECT

public:
    LayerManagerDockWidget(QWidget *parent = 0);
    virtual ~LayerManagerDockWidget();


private:
    boost::shared_ptr<LayerManagerDockWidgetPrivate> private_;
};

} // end namespace
#endif // LAYERMANAGERDOCKWIDGET_H
