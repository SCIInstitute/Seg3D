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
  void prep_for_drag_and_drop( bool );

// -- constructor/destructor --
public:
  LayerWidget( QFrame* parent, LayerHandle layer );
  virtual ~LayerWidget();
    
// -- widget internals --
public Q_SLOTS:
  // SHOW_OPACITY_BAR:
  // this function is called when the opacity button is clicked, it hides any bars that might be
  // open and hides or shows the opacity bar
  void show_opacity_bar( bool show );

  // SHOW_BRIGHTNESS_CONTRAST_BAR:
  // this function is called when the brightness contrast button is clicked, it hides any bars that might be
  // open and hides or shows the brightness contrast bar
  void show_brightness_contrast_bar( bool show );

  // SHOW_BORDER_FILL_BAR:
  // this function is called when the border fill button is clicked, it hides any bars that might be
  // open and hides or shows the border fill bar
  void show_border_fill_bar( bool show );

  // SHOW_COLOR_BAR:
  // this function is called when the color button is clicked, it hides any bars that might be
  // open and hides or shows the color bar
  void show_color_bar( bool show );

  // SHOW_PROGRESS_BAR:
  // this function is called when work is being done on the layer, it hides any bars that might be
  // open and hides or shows the progress bar
  void show_progress_bar( bool show );

  // VISUAL_LOCK:
  // this function is called when the layer is locked, it locks the gui representation of the layer
  // and also makes it so that you cannot access it for making changes
  void visual_lock( bool lock );
  
public:
  // SHOW_SELECTION_CHECKBOX:
  // this function is called when the user opens a group menu so that the layer's selection
  // checkbox can be accessed
  void show_selection_checkbox( bool hideshow );

  // SET_ACTIVE:
  // this function is called when the user activates or deactivates the layer and changes the
  // stylesheet of the layer appropriately
  void set_active( bool active );

  // SET_DROP:
  // this function is called when a drag enter or leave event is triggered and it calls hide
  // or show on the dropspace to make it look like there is a space for the user to drop the
  // dragged layer
  void set_drop( bool drop );

  // SEETHROUGH:
  // this function is called when the user clicks on a layer to drag it.  It sets the stylesheet
  // on the layer to make it look like it has been picked up
  void seethrough( bool see );

  // SET_GROUP_MENU_STATUS:
  // this function is called by the group to let the layers know that a group menu has been opened
  void set_group_menu_status( bool status );

  // SET_PICKED_UP:
  // this function is called to set or unset the state of picked_up_
  void set_picked_up( bool up ){ this->picked_up_ = up; }

  // PREP_FOR_ANIMATION:
  // this function hides the actual widgets and substitutes them for images to make drag and drop
  // faster
  void prep_for_animation( bool move_time );

public:
  // GET_VOLUME_TYPE:
  // this function returns the type volume that the layerwidget represents
  int get_volume_type(){ return this->volume_type_; }

  // GET_LAYER_ID:
  // this function returns the id of the layer that the layerwidget represents
  std::string& get_layer_id(){ return this->layer_id_; }


  
private:
    boost::shared_ptr< LayerWidgetPrivate > private_;

private:
  // icons to represent the layer types
  QIcon data_layer_icon_;
  QIcon label_layer_icon_;
  QIcon mask_layer_icon_;
  
  // these member variables are for keeping track locally of the layer settings
  std::string layer_id_;
  Core::GridTransform grid_transform_;
  Core::VolumeType volume_type_;

  // these member variables are for keeping track of the states of the layers so that they can
  // be represented properly in the gui
  bool active_;
  bool picked_up_;
  bool layer_menus_open_;
  bool group_menus_open_;
  
  
  LayerWidget* drop_layer_;

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

protected:
  void resizeEvent( QResizeEvent *event );

};

} //end namespace Seg3D

#endif
