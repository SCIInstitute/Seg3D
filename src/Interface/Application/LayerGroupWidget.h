/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_APPLICATION_LAYERGROUPWIDGET_H
#define INTERFACE_APPLICATION_LAYERGROUPWIDGET_H

#ifndef Q_MOC_RUN

// QT Includes
#include <QWidget>

// Application includes
#include <Application/Layer/LayerGroup.h>

#endif

namespace Seg3D
{

class LayerGroupWidgetPrivate;

class LayerGroupWidget : public QWidget
{
  Q_OBJECT

  // -- constructor/destructor --
public:
  LayerGroupWidget( QWidget* parent, LayerGroupHandle group );
  virtual ~LayerGroupWidget();

public:    
  /// PREP_FOR_ANIMATION:
  /// this function replaces the widget with a screenshot of the widget for speed
  void prep_for_animation( bool move_time );
  
    /// INSTANT_HIDE_DROP_SPACE:
    /// this function instantly hides the drop space for when a move has been made
    void instant_hide_drop_space();
  
  /// PREP_LAYERS_FOR_DRAG_AND_DROP:
  /// this function tells each layer to prepare for drag and drop by replacing the actual widgets
  /// with images of themselves.
  void prep_layers_for_drag_and_drop( bool move_time );
  
  /// SET_PICKED_UP_GROUP_SIZE:
  /// function that sets the size of the currently picked up group
  void set_picked_up_group_size( int group_height );
    
  /// HANDLE_CHANGE:
  /// this function handles changes from the layer manager.
  void handle_change();
  
  /// NOTIFY_PICKED_UP_LAYER_SIZE:
  /// this function sets the layer size of all
  void notify_picked_up_layer_size( int layer_size );

protected:
  /// RESIZEEVENT:
  /// this is an overloaded function to keep the size of the overlay widget in sync with the 
  /// size of the LayerGroupWidget
  virtual void resizeEvent( QResizeEvent *event );

  /// MOUSEPRESSEVENT:
  /// Overloaded function that is triggered when a user clicks on the group
  virtual void mousePressEvent( QMouseEvent* event );

  /// DROPEVENT:
  /// Overloaded function that is triggered when a drop occurs on the group
  virtual void dropEvent( QDropEvent* event );

  /// DRAGENTEREVENT:
  /// Overloaded function that is triggered when a drag even enters the group
  virtual void dragEnterEvent( QDragEnterEvent* event );
  
  /// DRAGLEAVEEVENT:
  /// Overloaded function that is triggered when a drag even leaves the group
  virtual void dragLeaveEvent( QDragLeaveEvent* event );

Q_SIGNALS:
  /// PREP_GROUPS_FOR_DRAG_AND_DROP_SIGNAL_:
  /// this signal tells the LayerManagerWidget that it needs to prep all the groups for drag and 
  /// drop.
  void prep_layers_for_drag_and_drop_signal_( bool );

  /// PREP_LAYERS_FOR_DRAG_AND_DROP_SIGNAL_:
  /// this signal tells the LayerManagerWidget that it needs to tell all the groups to prepare their
  /// layers for for drag and drop.
  void prep_groups_for_drag_and_drop_signal_( bool );

  /// PICKED_UP_GROUP_SIZE_SIGNAL_:
  /// this signal is sent to the LayerManagerWidget to notify the other groups what size this
  /// group is so that the proper size slot is opened up
  void picked_up_group_size_signal_( int );

  /// PICKED_UP_LAYER_SIZE_SIGNAL_:
  /// this signal is sent to the LayerManagerWidget to notify the other layers what size this
  /// layer is so that the proper size slot is opened up
  void picked_up_layer_size_signal_( int );
  
private Q_SLOTS:
  /// SHOW_LAYERS:
  /// function that shows or hides the layers
  void show_layers( bool show );

  /// UNCHECK_DELETE_CONFIRM:
  /// this is a simple helper function that unchecks the delete confirmation checkbox
    void verify_delete();

  /// SHOW_GROUP:
  /// helper function for the show_layers function that shows the group
  void show_group();
  
  /// HIDE_SHOW_CHECKBOXES:
  /// check or uncheck all the checkboxes on all the layers
  void check_uncheck_for_delete( bool checked );
  
  /// CHECK_UNCHECK_FOR_DUPLICATE:
  /// checks or unchecks all the checkboxes on all the layers
  void check_uncheck_for_duplicate( bool checked );
  
  /// ENABLE_DISABLE_DELETE_BUTTON:
  /// enables or disables the delete button based on whether or not there are any boxes selected
  void enable_disable_delete_button();
  
  /// ENABLE_DISABLE_DUPLICATE_BUTTON:
  /// enables or disables the duplicate button based on whether or not there are any boxes selected
  void enable_disable_duplicate_button();
  
  /// DUPLICATE_CHECKED_LAYERS:
  /// dispatches actions for duplicating checked layers.
  void duplicate_checked_layers();

  /// -- widget internals --
private:
    LayerGroupWidgetPrivate* private_;
};
    
} //end namespace Seg3D

#endif
