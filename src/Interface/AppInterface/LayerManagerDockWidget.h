/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2009 Scientific Computing and Imaging Institute,
 University of Utah.
 
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

#ifndef INTERFACE_APPINTERFACE_LAYERMANAGERDOCKWIDGET_H
#define INTERFACE_APPINTERFACE_LAYERMANAGERDOCKWIDGET_H

// QT includes
#include <QtGui>

// STL includes
#include <string>

// Boost includes
#include <boost/signals2/signal.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Utils/Core/ConnectionHandler.h>

// Interface includes
#include <Interface/AppInterface/LayerWidget.h>
#include <Interface/AppInterface/LayerManagerWidget.h>

namespace Seg3D
{

class LayerManagerDockWidget : public QDockWidget, public Utils::ConnectionHandler
{
Q_OBJECT

// -- constructor/destructor --
public:
  LayerManagerDockWidget( QWidget *parent = 0 );
  virtual ~LayerManagerDockWidget();

  // -- functions that control the layer manager --
public:

  //Insert Layer 
  void insert_layer_ui( LayerHandle &layer );
  
  //Insert Layer at
  void insert_layer_at_ui( LayerHandle &layer, int index );
  
  //Set the active layer
  void activate_layer_ui( LayerHandle &layer );
  
  //Delete a layer
  void delete_layer_ui( LayerHandle &layer );
  
  //Delete a layer
  void delete_layers_ui( std::vector< LayerHandle > layers );
  
  //Delete an empty group
    void delete_group_ui( LayerGroupHandle &group );

  //Move a group
  void move_group_ui( std::string &group_id, int index );

  // -- static functions for callbacks into this widget --
public:
  typedef QPointer< LayerManagerDockWidget > qpointer_type;
  
  static void HandleActivateLayer( qpointer_type qpointer, LayerHandle &layer );
  static void HandleInsertLayer( qpointer_type qpointer, LayerHandle &layer );
  static void HandleInsertLayerAt( qpointer_type qpointer, LayerHandle &layer, int index );
  static void HandleDeleteLayer( qpointer_type qpointer, LayerHandle &layer );
  static void HandleDeleteLayers( qpointer_type qpointer, std::vector< LayerHandle > layers );
  static void HandleGroupDeleted( qpointer_type qpointer, LayerGroupHandle &group );
  static void HandleGroupMoved( qpointer_type qpointer, std::string &group_id, int index );
  
  
private:
  QSharedPointer< LayerManagerWidget > layer_manager_widget_;
  
};

} // end namespace Seg3D
#endif // LAYERMANAGERDOCKWIDGET_H
