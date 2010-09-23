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
#include <QtGui>

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

// -- constructor/destructor --
public:
  LayerWidget( QFrame* parent, LayerHandle layer );
  virtual ~LayerWidget();

// -- update functions --
public:
  // ENABLE_BUTTONS:
  // Specify which buttons need to be enabled for the user
  void enable_buttons( bool lock_button, bool compute_isosurface_button,
    bool other_buttons, bool initialize = false);

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

public:
  // GET_VOLUME_TYPE:
  // this function returns the type volume that the layerwidget represents
  int get_volume_type() const;

  // GET_LAYER_ID:
  // this function returns the id of the layer that the layerwidget represents
  std::string get_layer_id() const;

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
  void mousePressEvent( QMouseEvent* event );

  // DROPEVENT:
  // this function is called when the user drops another layer onto this one
  void dropEvent( QDropEvent* event );

  // DRAGENTEREVENT:
  // this function is called when the user drags another layer over the top of this one
  void dragEnterEvent( QDragEnterEvent* event );

  // DRAGLEAVEEVENT:
  // this function is called when the user drags another layer off of the top of this one
  void dragLeaveEvent( QDragLeaveEvent* event );
    
private Q_SLOTS:

  // SET_MASK_BACKGROUND_COLOR:
  // this function is for setting the background of the mask to match the active color
  void set_mask_background_color( int color_index );

  // SET_MASK_BACKGROUND_COLOR_FROM_PREFERENCE_CHANGE:
  // this seperate function is need to change the color of the mask backgroun in the case that
  // the active color for the layer has been changed by the preferences manager
  void set_mask_background_color_from_preference_change( int color_index );
  
  // UNCHECK_SHOW_ISO_BUTTON:
  // this function unchecks the show iso button if it is on when the iso surface button is clicked
  // and the iso surface is deleted
  void uncheck_show_iso_button();

  // COMPUTE_ISOSURFACE
  // this function computes the isosurface using the quality stored in the state variable
  void compute_isosurface();

  // DELETE_ISOSURFACE
  void delete_isosurface();

  // TRIGGER_ABORT:
  // Trigger the abort signal of the layer
  void trigger_abort();

protected:
  void resizeEvent( QResizeEvent *event );
  
public:
  typedef QPointer< LayerWidget > qpointer_type;

  // UPDATESTATE:
  // Entry point for the state engine to notify state has changed
  static void UpdateState( qpointer_type qpointer );

  // UPDATEPROGRESS:
  // Update the progress bar 
  static void UpdateProgress( qpointer_type qpointer, double progress );
};

} //end namespace Seg3D

#endif
