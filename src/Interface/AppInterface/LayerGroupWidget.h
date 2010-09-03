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

#ifndef INTERFACE_APPINTERFACE_LAYERGROUPWIDGET_H
#define INTERFACE_APPINTERFACE_LAYERGROUPWIDGET_H

// QT Includes
#include <QtGui>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Interface/AppInterface/LayerWidget.h>

namespace Seg3D
{

class LayerGroupWidgetPrivate;

class LayerGroupWidget : public QWidget
{
  Q_OBJECT
  
Q_SIGNALS:
  // PREP_GROUPS_FOR_DRAG_AND_DROP_SIGNAL_:
  // this signal tells the LayerManagerWidget that it needs to prep all the groups for drag and 
  // drop.
  void prep_layers_for_drag_and_drop_signal_( bool );
  
  // PREP_LAYERS_FOR_DRAG_AND_DROP_SIGNAL_:
  // this signal tells the LayerManagerWidget that it needs to tell all the groups to prepare their
  // layers for for drag and drop.
  void prep_groups_for_drag_and_drop_signal_( bool );
  
  // PICKED_UP_GROUP_SIZE_SIGNAL_:
  // this signal is sent to the LayerManagerWidget to notify the other groups what size this
  // group is so that the proper size slot is opened up
  void picked_up_group_size_signal_( int );
  
  // -- constructor/destructor --
public:
  LayerGroupWidget( QWidget* parent, LayerGroupHandle group );
  virtual ~LayerGroupWidget();
  
public:
  typedef QPointer< LayerGroupWidget > qpointer_type;
  
public Q_SLOTS:
  // SHOW_LAYERS:
  // function that shows or hides the layers
  void show_layers( bool show );

  // SHOW_SELECTION_CHECKBOXES:
  // function that hides or shows selection checkboxes
  void show_selection_checkboxes( bool show );

  // ENABLE_DELETE_BUTTON:
  // function that enables the delete button
  void enable_delete_button( bool enable );
  
  // NOTIFY_LAYER_MANAGER_WIDGET:
  // this function triggers the prep_groups_for_drag_and_drop signal.
  void notify_layer_manager_widget( bool move_time );
  
  // PREP_FOR_ANIMATION:
  // this function replaces the widget with a screenshot of the widget for speed
  void prep_for_animation( bool move_time );
  
  
public:
  // SET_DROP:
  // this function give the user the impression that a group is available for dropping onto by
  // opening up a space for dropping
    void set_drop( bool drop );

  // GET_GROUP_ID:
  // function that returns a string containing the groups id
  const std::string &get_group_id();

  // SET_ACTIVE_LAYER:
  // function that sets the active layer
  LayerWidgetQWeakHandle set_active_layer( LayerHandle layer );

  // SEETHROUGH:
  // function that puts the group widget into a state that makes it look "picked up"
  void seethrough( bool see );
  
  // PREP_LAYERS_FOR_DRAG_AND_DROP:
  // this function tells each layer to pepare for drag and drop by replacing the actual widgets
  // with images of themselves.
  void prep_layers_for_drag_and_drop( bool move_time );
  
  // SET_PICKED_UP_GROUP_SIZE:
  // function that sets the size of the currently picked up group
  void set_picked_up_group_size( int group_height );
  
  // UPDATESTATE:
  // Entry point for the state engine to notify state has changed
  static void UpdateState( qpointer_type qpointer );
  
  void handle_change();

protected:
  // RESIZEEVENT:
  // this is an overloaded function to keep the size of the overlay widget in sync with the 
  // size of the LayerGroupWidget
  void resizeEvent( QResizeEvent *event );
  
private Q_SLOTS:
  // ADJUST_NEW_SIZE_LABELS:
  // this function adjusts the size of the labels based on the scale
/*    void adjust_new_size_labels( double scale_factor );*/

  // UNCHECK_DELETE_CONFIRM:
  // this is a simple helper function that unchecks the delete confirmation checkbox
    void uncheck_delete_confirm();

  // SET_PICKED_UP:
  // this sets the local member variable picked_up_ to indicate whether the current LayerWidget 
  // has been picked up.  This is useful when we are representing drag and drop visually
    void set_picked_up( bool up ){ this->picked_up_ = up; }

  // SET_DROP_TARGET:
  // this function stores a local copy of the widget that is going to be dropped onto
  // in the LayerGroupWidget that is being dropped
  void set_drop_target( LayerGroupWidget* target_group );

  // MOUSEPRESSEVENT:
  // Overloaded function that is triggered when a user clicks on the group
  void mousePressEvent( QMouseEvent* event );

  // DROPEVENT:
  // Overloaded function that is triggered when a drop occurs on the group
  void dropEvent( QDropEvent* event );

  // DRAGENTEREVENT:
  // Overloaded function that is triggered when a drag even enters the group
  void dragEnterEvent( QDragEnterEvent* event );
  
  // DRAGLEAVEEVENT:
  // Overloaded function that is triggered when a drag even leaves the group
  void dragLeaveEvent( QDragLeaveEvent* event );

  // SHOW_GROUP:
  // helper function for the show_layers function that shows the group
  void show_group();

  // HIDE_GROUP:
  // helper function for the show_layers function
  void hide_group();
  
  // SELECT_CROP_MENU:
  // slot that dispatches an action to set the active menu to the crop menu
/*  void select_crop_menu( bool show );*/
  
  // SELECT_FLIP_ROTATE_MENU:
  // slot that dispatches an action to set the active menu to the flip rotate menu
/*  void select_flip_rotate_menu( bool show );*/
  
  // SELECT_RESAMPLE_MENU:
  // slot that dispatches an action to set the active menu to the resample menu
/*  void select_resample_menu( bool show );*/
  
  // SELECT_TRANSFORM_MENU:
  // slot that dispatches an action to set the active menu to the transform menu
/*  void select_transform_menu( bool show );*/
  
  // SELECT_ISO_MENU:
  // slot that dispatches an action to set the active menu to the iso menu
  void select_iso_menu( bool show );
  
  // SELECT_DELETE_MENU:
  // slot that dispatches an action to set the active menu to the delete menu
  void select_delete_menu( bool show );
  
private:
  // UPDATE_WIDGET_STATE:
  // function that updates which menu is shown based on which menu is the active one
  void update_widget_state();
  
  // SET_CROP_VISIBILITY:
  // function that hides or shows the crop menu
/*  void set_crop_visibility( bool show );*/
  
  // SET_FLIP_ROTATE_VISIBILITY:
  // function that hides or shows the flip rotate menu
/*  void set_flip_rotate_visibility( bool show );*/
  
  // SET_RESAMPLE_VISIBILITY:
  // function that hides or shows the resample menu
/*  void set_resample_visibility( bool show );*/
  
  // SET_TRANSFORM_VISIBILITY:
  // function that hides or shows the transform menu
/*  void set_transform_visibility( bool show );*/
  
  // SET_ISO_VISIBILITY:
  // function that hides or shows the iso menu
  void set_iso_visibility( bool show );
  
  // SET_DELETE_VISIBILITY:
  // function that hides or shows the delete menu
  void set_delete_visibility( bool show );
  
  
  // CLEANUP_REMOVED_WIDGETS:
  void cleanup_removed_widgets();
  

  // -- widget internals --
private:
    boost::shared_ptr< LayerGroupWidgetPrivate > private_;
    QVector< LayerWidgetQHandle > layer_list_;
  std::map< std::string, LayerWidgetQHandle > layer_map_;
    
private:
  std::string group_id_;
  int picked_up_group_height_;
  int current_height_;
  int current_width_;
  int current_depth_;
  bool group_menus_open_;
  bool picked_up_;
  bool drop_group_set_;
  LayerGroupWidget* drop_group_;
    
};
    


} //end namespace Seg3D

#endif