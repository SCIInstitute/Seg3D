#ifndef TOOLSDOCKWIDGET_H
#define TOOLSDOCKWIDGET_H

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
  
class ToolsDockWidgetPrivate;
  

class ToolsDockWidget : public QDockWidget {

    Q_OBJECT

public:
    ToolsDockWidget(QWidget *parent = 0);
    ~ToolsDockWidget();


private:
  
    boost::shared_ptr<ToolsDockWidgetPrivate> private_;

};
  
} // end namespace

#endif // TOOLSDOCKWIDGET_H
