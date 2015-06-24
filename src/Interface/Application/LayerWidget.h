/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_APPLICATION_LAYERWIDGET_H
#define INTERFACE_APPLICATION_LAYERWIDGET_H

#ifndef Q_MOC_RUN

// QT Includes
#include <QWidget>
#include <QFrame>

// Application includes
#include <Application/Layer/LayerFWD.h>

// Interface includes
#include <Interface/Application/GroupButtonMenu.h>

#endif

namespace Seg3D
{
  
class LayerWidget;
class LayerWidgetPrivate;
  
class LayerWidget : public QWidget
{
  Q_OBJECT

  // -- constructor/destructor --
public:
  LayerWidget( QFrame* parent, LayerHandle layer );
  virtual ~LayerWidget();

public:
  /// SHOW_SELECTION_CHECKBOX:
  /// this function is called when the user opens a group menu so that the layer's selection
  /// checkbox can be accessed
  void show_selection_checkbox( bool hideshow );

  /// INSTANT_HIDE_DROP_SPACE:
  /// this function instantly hides the drop space for when a move has been made
  void instant_hide_drop_space();

  /// PREP_FOR_ANIMATION:
  /// this function hides the actual widgets and substitutes them for images to make drag and drop
  /// faster
  void prep_for_animation( bool move_time );
  
  /// SET_PICKED_UP_LAYER_SIZE:
  /// this function sets the picked up layer size on all the layers so that the drop space widget
  /// knows how big to become when it opens
  void set_picked_up_layer_size( int size );
  
  /// SET_CHECK_SELECTED:
  /// function that sets whether all the layer checkboxes are selected or not when you press the 
  /// select all button
  void set_selected( bool selected );

  /// GET_LAYER_ID:
  /// this function returns the id of the layer that the layerwidget represents
  std::string get_layer_id() const;
  
  /// GET_LAYER_NAME:
  /// this function returns the id of the layer that the layerwidget represents
  std::string get_layer_name() const;
  
  /// IS_SELECTED:
  /// function that returns whether or not the layer is selected.
  bool is_selected() const;

  /// SET_DROP_GROUP:
  /// this function is for keeping track of which group the drop is going to happen on
  void set_drop_group( GroupButtonMenu* target_group );

  // -- Virtual functions defined by QWidget --
protected:
  /// MOUSEPRESSEVENT:
  /// this function is overloaded to enable drag and drop
  virtual void mousePressEvent( QMouseEvent* event );

  /// DROPEVENT:
  /// this function is called when the user drops another layer onto this one
  virtual void dropEvent( QDropEvent* event );

  /// DRAGENTEREVENT:
  /// this function is called when the user drags another layer over the top of this one
  virtual void dragEnterEvent( QDragEnterEvent* event );

  /// DRAGLEAVEEVENT:
  /// this function is called when the user drags another layer off of the top of this one
  virtual void dragLeaveEvent( QDragLeaveEvent* event );
  
  /// CONTEXTMENUEVENT:
  /// this function is the overloaded qt function that creates and connects actions to a context event
  virtual void contextMenuEvent( QContextMenuEvent * event );

  /// RESIZEEVENT:
  /// Called when the widget has been resized.
  virtual void resizeEvent( QResizeEvent *event );

Q_SIGNALS:
  /// PREP_FOR_DRAG_AND_DROP:
  /// this signal tells the group that it is time to prep the layers for drag and drop
  void prep_for_drag_and_drop( bool );

  /// LAYER_SIZE_SIGNAL:
  /// this function lets the LayerManagerWidget know what size the picked up layer is so that it
  /// can notify the layers
  void layer_size_signal_( int );

  /// SELECTION_BOX_CHANGED:
  /// this signal tells the group that the selection box's status has been changed
  void selection_box_changed();
  
private Q_SLOTS:

  /// SET_MASK_BACKGROUND_COLOR:
  /// this function is for setting the background of the mask to match the active color
  void set_mask_background_color( int color_index );

  /// SET_MASK_BACKGROUND_COLOR_FROM_PREFERENCE_CHANGE:
  /// this separate function is need to change the color of the mask background in the case that
  /// the active color for the layer has been changed by the preferences manager
  void set_mask_background_color_from_preference_change( int color_index );

  /// COMPUTE_ISOSURFACE
  /// this function computes the isosurface using the quality stored in the state variable
  void compute_isosurface();

  /// DELETE_ISOSURFACE
  void delete_isosurface();

  /// TRIGGER_ABORT:
  /// Trigger the abort signal of the layer
  void trigger_abort();

  /// TRIGGER_STOP:
  /// Trigger the stop signal of the layer
  void trigger_stop();
  
  /// SET_BRIGHTNESS_CONTRAST_TO_DEFAULT:
  /// dispatches actions that set the values of the brightness and contrast back to defaults
  void set_brightness_contrast_to_default();
  
  /// ACTIVATE_FROM_LINEEDIT_FOCUS:
  /// dispatches an action that will set the layer to be active when the user edits the name of the 
  /// layer
  void activate_from_lineedit_focus();
  
  /// DELETE_LAYER_FROM_CONTEXT_MENU:
  /// dispatches an action that deletes the layer
  void delete_layer_from_context_menu();
  
  /// EXPORT_NRRD:
  /// dispatches an action that exports the layer as a nrrd
  void export_nrrd();

  /// EXPORT_MATLAB:
  /// dispatches an action that exports the layer as a matlab
  void export_matlab();
  
  /// EXPORT_MRC:
  /// dispatches an action that exports the layer as a mrc
  void export_mrc();
  
  /// EXPORT_DICOM:
  /// dispatches an action that exports the layer as a dicom
  void export_dicom();
  
  /// EXPORT_TIFF
  /// dispatches an action that exports the layer as a tiff
  void export_tiff();
  
  /// EXPORT_BITMAP:
  /// dispatches an action that exports the layer as a bitmap
  void export_bitmap();
  
  /// EXPORT_PNG:
  /// dispatches an action that exports the layer as a png
  void export_png();

  /// EXPORT_ISOSURFACE:
  /// dispatches an action that exports the isosurface for this layer
  void export_isosurface();

private:
  friend class LayerWidgetPrivate;
  LayerWidgetPrivate* private_; 
};

} //end namespace Seg3D

#endif
