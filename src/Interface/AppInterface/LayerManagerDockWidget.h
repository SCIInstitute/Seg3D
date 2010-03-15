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

  //Create new group
  void new_group();

  //Close group
  void close_group();

  //Open new data layer from file
  void layer_from_file();

  //Clone layer
  void clone_layer( LayerHandle& layer );

  //New mask layer
  void new_mask_layer();

  //Remove layer
  void remove_layer( LayerHandle& layer );
  
  //Move Layer Above
  void insert_above_layer( LayerHandle& below_layer, LayerHandle &above_layer );
  
  //Insert Layer default
  void insert_layer( LayerHandle &layer );
  void process_group_ui( LayerGroupHandle &group );
  
  //Flip or Rotate Layer
//  void flip_layer( layer_widget_list_type layers_to_crop_list );
//  void crop_layer( layer_widget_list_type layers_to_crop_list );
//  void resample_layer( layer_widget_list_type layers_to_crop_list );
//  void transform_layer( 

private:

  boost::signals2::connection new_group_connection_;
  boost::signals2::connection close_group_connection_;
  boost::signals2::connection layer_from_file_connection_;
  boost::signals2::connection clone_layer_connection_;
  boost::signals2::connection new_mask_layer_connection_;
  boost::signals2::connection remove_layer_connection_;

  //typedef std::map< std::string, LayerWidget* > layer_widget_list_type;
  //layer_widget_list_type layer_widget_list_;

  LayerManagerWidget* layer_manager_widget_;

  // -- static functions for callbacks into this widget --
public:
  typedef QPointer< LayerManagerDockWidget > qpointer_type;
  
  static void HandleInsertLayer( qpointer_type qpointer, LayerGroupHandle group );
  static void HandleNewGroup( qpointer_type qpointer, LayerHandle layer );
  static void HandleCloseGroup( qpointer_type qpointer, LayerHandle layer );
  static void HandleDataFromFile( qpointer_type qpointer, LayerHandle layer );
  static void HandleCloneLayer( qpointer_type qpointer, LayerHandle layer );
  static void HandleNewMaskLayer( qpointer_type qpointer, LayerHandle layer );
  static void HandleRemoveLayer( qpointer_type qpointer, LayerHandle layer );

};

} // end namespace Seg3D
#endif // LAYERMANAGERDOCKWIDGET_H
