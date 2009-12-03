#ifndef PROJECTDOCKWIDGET_H
#define PROJECTDOCKWIDGET_H

// QT includes
#include <QtGui>
#include <QDockWidget>

// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D{
  
class ProjectDockWidgetPrivate;
  
class ProjectDockWidget : public QDockWidget {
  
    Q_OBJECT
  
public:
    ProjectDockWidget(QWidget *parent = 0);
    ~ProjectDockWidget();


private:
    boost::shared_ptr<ProjectDockWidgetPrivate> private_;
};
  
} // end namespace

#endif // PROJECTDOCKWIDGET_H
