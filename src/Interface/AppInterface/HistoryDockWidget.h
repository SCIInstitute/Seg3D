#ifndef HISTORYDOCKWIDGET_H
#define HISTORYDOCKWIDGET_H

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

class HistoryDockWidgetPrivate;

class HistoryDockWidget : public QDockWidget {
   
  Q_OBJECT

public:
    HistoryDockWidget(QWidget *parent = 0);
    virtual ~HistoryDockWidget();

private:
  
  boost::shared_ptr<HistoryDockWidgetPrivate> private_;
  

};
  
} // end namespace

#endif // HISTORYDOCKWIDGET_H
