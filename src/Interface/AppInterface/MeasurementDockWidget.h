#ifndef MEASUREMENTDOCKWIDGET_H
#define MEASUREMENTDOCKWIDGET_H

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
  
class MeasurementDockWidgetPrivate;

class MeasurementDockWidget : public QDockWidget {

    Q_OBJECT

public:
    MeasurementDockWidget(QWidget *parent = 0);
    ~MeasurementDockWidget();

private:
  
  boost::shared_ptr<MeasurementDockWidgetPrivate> private_;
   
};
  
} // end namespace

#endif // MEASUREMENTDOCKWIDGET_H
