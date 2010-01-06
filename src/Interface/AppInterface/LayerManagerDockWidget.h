#ifndef LAYERMANAGERDOCKWIDGET_H
#define LAYERMANAGERDOCKWIDGET_H

// QT includes
#include <QtGui>
#include <QDockWidget>

// STL includes
#include <string>

// Boost includes
#include <boost/signals2/signal.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D {

class LayerManagerDockWidget : public QDockWidget {
    Q_OBJECT

// -- constructor/destructor --
public:
  LayerManagerDockWidget(QWidget *parent = 0);
  virtual ~LayerManagerDockWidget();
  
// -- functions that control the layer manager --
public:
  
  //Create new group
  void new_group();
  
  //Close group
  void close_group();
  
  //Open new data layer
  void open_data_layer();
  
  //Create new mask layer
  void new_mask_layer();
  
private:
  QWidget* layer_manager_;  

};

} // end namespace
#endif // LAYERMANAGERDOCKWIDGET_H
