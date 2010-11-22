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

#ifndef INTERFACE_APPINTERFACE_LAYERWIDGET_H
#define INTERFACE_APPINTERFACE_LAYERWIDGET_H

// QT Includes
#include <QtGui/QWidget>
#include <QtGui/QFrame>
#include <QtGui/QMouseEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QPointer>


// Application includes
#include <Application/Layer/Layer.h>


namespace Seg3D
{
  
class LayerGroupWidget;
typedef QSharedPointer< LayerGroupWidget > LayerGroupWidgetQHandle;
  
class LayerWidget;
typedef QSharedPointer< LayerWidget > LayerWidgetQHandle;
typedef QWeakPointer< LayerWidget > LayerWidgetQWeakHandle;


class LayerWidgetPrivate;
  
class LayerWidget : public QWidget
{
Q_OBJECT


Q_SIGNALS:
  // PREP_FOR_DRAG_AND_DROP:
  // this signal tells the group that it is time to prep the layers for drag and drop
  void prep_for_drag_and_drop( bool );
  
  // LAYER_SIZE_SIGNAL:
  // this function lets the LayerManagerWidget know what size the picked up layer is so that it
  // can notify the layers
  void layer_size_signal_( int );
  
  // SELECTION_BOX_CHANGED:
  // this signal tells the group that the selection box's status has been changed
  void selection_box_changed();

// -- constructor/destructor --
public:
  LayerWidget( QFrame* parent, LayerHandle layer );
  virtual ~LayerWidget();

// -- update functions --
public:

  // UPDATE_APPEARANCE:
  // Update the appearance of the widget to reflect its state
  void update_appearance( bool locked, bool active, bool in_use, bool initialize = false);

  // UPDATE_WIDGET_STATE:
  // Update the button state, the open menus and color of the widget
  void update_widget_state( bool initialize = false );

public:
  
  // UPDATE_PROGRESS_BAR:
  // Update the progress in the widget
  void update_progress_bar( double progress );
  
  // SHOW_SELECTION_CHECKBOX:
  // this function is called when the user opens a group menu so that the layer's selection
  // checkbox can be accessed
  void show_selection_checkbox( bool hideshow );

  // SET_ACTIVE:
  // this function is called when the user activates or deactivates the layer and changes the
  // stylesheet of the layer appropriately
  void set_active( bool active );

  // ENABLE_DROP_SPACE:
  // this function is called when a drag enter or leave event is triggered and it calls hide
  // or show on the dropspace to make it look like there is a space for the user to drop the
  // dragged layer
  void enable_drop_space( bool drop );
  
  // INSTANT_HIDE_DROP_SPACE:
  // this funtion instantly hides the drop space for when a move has been made
  void instant_hide_drop_space();
  
  // HIDE_OVERLAY:
  // this hides the invsible overlay that we put over the widget when dragging and dropping so
  // that we dont have multiple enter and exit events
  void hide_overlay();

  // SEETHROUGH:
  // this function is called when the user clicks on a layer to drag it.  It sets the stylesheet
  // on the layer to make it look like it has been picked up
  void seethrough( bool see );

  // SET_GROUP_MENU_STATUS:
  // this function is called by the group to let the layers know that a group menu has been opened
  void set_group_menu_status( bool status );

  // SET_PICKED_UP:
  // this function is called to set or unset the state of picked_up_
  void set_picked_up( bool up );

  // PREP_FOR_ANIMATION:
  // this function hides the actual widgets and substitutes them for images to make drag and drop
  // faster
  void prep_for_animation( bool move_time );
  
  // SET_PICKED_UP_LAYER_SIZE:
  // this function sets the picked up layer size on all the layers so that the drop space widget
  // knows how big to become when it opens
  void set_picked_up_layer_size( int size );
  
  // SET_CHECK_SELECTED:
  // funtion that sets whether all the layer checkboxes are selected or not when you press the 
  // select all button
  void set_check_selected( bool selected );

public:
  // GET_VOLUME_TYPE:
  // this function returns the type volume that the layerwidget represents
  int get_volume_type() const;

  // GET_LAYER_ID:
  // this function returns the id of the layer that the layerwidget represents
  std::string get_layer_id() const;
  
  // GET_LAYER_NAME:
  // this function returns the id of the layer that the layerwidget represents
  std::string get_layer_name() const;
  
  // GET_SELECTED:
  // funtion that returns whether or not the layer is selected.
  bool get_selected() const;

private:
    boost::shared_ptr< LayerWidgetPrivate > private_;

// FUNCTIONS FOR HANDLING DRAG AND DROP
private:
  // these member variables are for keeping track of the states of the layers so that they can
  // be represented properly in the gui
  bool layer_menus_open_;
  bool group_menus_open_;

  // SET_DROP_TARGET:
  // this function is for keeping track of which layer the drop is going to happen on
  void set_drop_target( LayerWidget* target_layer );

  // MOUSEPRESSEVENT:
  // this function is overloaded to enable drag and drop
  virtual void mousePressEvent( QMouseEvent* event );

  // DROPEVENT:
  // this function is called when the user drops another layer onto this one
  virtual void dropEvent( QDropEvent* event );

  // DRAGENTEREVENT:
  // this function is called when the user drags another layer over the top of this one
  virtual void dragEnterEvent( QDragEnterEvent* event );

  // DRAGLEAVEEVENT:
  // this function is called when the user drags another layer off of the top of this one
  virtual void dragLeaveEvent( QDragLeaveEvent* event );
  
  virtual void contextMenuEvent( QContextMenuEvent * event );
  
private Q_SLOTS:

  // SET_MASK_BACKGROUND_COLOR:
  // this function is for setting the background of the mask to match the active color
  void set_mask_background_color( int color_index );

  // SET_MASK_BACKGROUND_COLOR_FROM_PREFERENCE_CHANGE:
  // this seperate function is need to change the color of the mask backgroun in the case that
  // the active color for the layer has been changed by the preferences manager
  void set_mask_background_color_from_preference_change( int color_index );

  // COMPUTE_ISOSURFACE
  // this function computes the isosurface using the quality stored in the state variable
  void compute_isosurface();

  // DELETE_ISOSURFACE
  void delete_isosurface();

  // TRIGGER_ABORT:
  // Trigger the abort signal of the layer
  void trigger_abort();
  
  // SET_BRIGHTNESS_CONTRAST_TO_DEFAULT:
  // dispatches actions that set the values of the brightness and contrast back to defaults
  void set_brightness_contrast_to_default();
  
  // ACTIVATE_FROM_LINEEDIT_FOCUS:
  // dispatches an action that will set the layer to be active when the user edits the name of the 
  // layer
  void activate_from_lineedit_focus();
  
  // DELETE_LAYER_FROM_CONTEXT_MENU:
  // dispatches an action that deletes the layer
  void delete_layer_from_context_menu();
  
  // EXPORT_DATA:
  // dispatches an action that exports the data
  void export_data();

protected:
  void resizeEvent( QResizeEvent *event );
  
public:
  typedef QPointer< LayerWidget > qpointer_type;

  // UPDATESTATE:
  // Entry point for the state engine to notify state has changed
  static void UpdateState( qpointer_type qpointer );

  // UPDATEACTIVESTATE:
  // Entry point for the state engine to notify active layer has changed
  static void UpdateActiveState( qpointer_type qpointer, LayerHandle layer );
  
  // UPDATEVIEWERBUTTONS:
  // update the layout of the viewer buttons
  static void UpdateViewerButtons( qpointer_type qpointer, std::string layout );

  // UPDATEPROGRESS:
  // Update the progress bar 
  static void UpdateProgress( qpointer_type qpointer, double progress );
  
};

} //end namespace Seg3D

#endif
