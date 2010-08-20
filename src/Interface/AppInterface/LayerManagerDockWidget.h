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
#include <Core/Utils/ConnectionHandler.h>

// Interface includes
#include <Interface/AppInterface/LayerWidget.h>
#include <Interface/AppInterface/LayerManagerWidget.h>

namespace Seg3D
{

class LayerManagerDockWidget : public QDockWidget, public Core::ConnectionHandler
{
Q_OBJECT

// -- constructor/destructor --
public:
  LayerManagerDockWidget( QWidget *parent = 0 );
  virtual ~LayerManagerDockWidget();

  // -- functions that control the layer manager --
public:
  
  //Set the active layer
  void activate_layer_ui( LayerHandle &layer );
  
  // GROUPS_CHANGED_UI:
  // function that handles sending notification to the ui that one of the groups has been changed
  void groups_changed_ui();
  
  // GROUP_INTERNALS_CHANGED_UI:
  // function that handles sending notification to the ui that the contents of one of the groups 
  // has been changed
  void group_internals_changed_ui( LayerGroupHandle &group );

  // -- static functions for callbacks into this widget --
public:
  typedef QPointer< LayerManagerDockWidget > qpointer_type;
  
  static void HandleGroupInternalChanged( qpointer_type qpointer, LayerGroupHandle &group );
  static void HandleGroupsChanged( qpointer_type qpointer );
  static void HandleActivateLayer( qpointer_type qpointer, LayerHandle &layer );
  
  
private:
  QSharedPointer< LayerManagerWidget > layer_manager_widget_;
  
};

} // end namespace Seg3D
#endif // LAYERMANAGERDOCKWIDGET_H
