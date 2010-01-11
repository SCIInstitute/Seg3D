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

// Interface includes
#include <Interface/AppInterface/LayerWidget.h>
#include <Interface/AppInterface/LayerManagerWidget.h>

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
  
  //Open new data layer from file
  void layer_from_file();
  
  //Clone layer
  void clone_layer(LayerHandle layer);
  
  //New mask layer
  void new_mask_layer();
  
  //Remove layer
  void remove_layer(LayerHandle layer);

//  -- slots --
  public Q_SLOTS:
    
    void layer_changed(int index);
  
  private:

    boost::signals2::connection new_group_connection_;
    boost::signals2::connection close_group_connection_;
    boost::signals2::connection layer_from_file_connection_;
    boost::signals2::connection clone_layer_connection_;
    boost::signals2::connection new_mask_layer_connection_;
    boost::signals2::connection remove_layer_connection_;
    
    typedef std::map<std::string, LayerWidget*> layer_widget_list_type;
    layer_widget_list_type layer_widget_list_;
    
    LayerManagerWidget* layer_manager_;  
  
public:
  static void HandleNewGroup(QPointer<LayerManagerDockWidget> widget, LayerHandle layer);
  static void HandleCloseGroup(QPointer<LayerManagerDockWidget> widget, LayerHandle layer);
  static void HandleDataFromFile(QPointer<LayerManagerDockWidget> widget, LayerHandle layer);
  static void HandleCloneLayer(QPointer<LayerManagerDockWidget> widget, LayerHandle layer);
  static void HandleNewMaskLayer(QPointer<LayerManagerDockWidget> widget, LayerHandle layer);
  static void HandleRemoveLayer(QPointer<LayerManagerDockWidget> widget, LayerHandle layer);

};

} // end namespace
#endif // LAYERMANAGERDOCKWIDGET_H
