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

#ifndef APPLICATION_VIEWER_VIEWER_H
#define APPLICATION_VIEWER_VIEWER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Boost includes 
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Viewer/AbstractViewer.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/State/State.h>
#include <Core/Renderer/PickPoint.h>

// Application includes
#include <Application/Layer/LayerFWD.h>

namespace Seg3D
{

// Forward declarations
class Viewer;
typedef boost::shared_ptr< Viewer > ViewerHandle;
typedef boost::weak_ptr< Viewer > ViewerWeakHandle;

class ViewerPrivate;
typedef boost::shared_ptr< ViewerPrivate > ViewerPrivateHandle;

// Class definition
class Viewer : public Core::AbstractViewer, public boost::enable_shared_from_this< Viewer >
{

  // -- Constructor/Destructor --
public:
  friend class ViewManipulator;
  friend class ViewerPrivate;
  friend class ViewerManager;
  friend class ViewerManagerPrivate;

  Viewer( size_t viewer_id, bool visible = true, const std::string& mode = Viewer::AXIAL_C );
  virtual ~Viewer();

  // -- Mouse and keyboard events handling --
public:

  typedef boost::function< bool( ViewerHandle, const Core::MouseHistory&, int, int, int ) > 
    mouse_event_handler_type;
  typedef boost::function< bool( ViewerHandle, int, int ) > enter_event_handler_type;
  typedef boost::function< bool( ViewerHandle ) > leave_event_handler_type;
  typedef boost::function< bool( ViewerHandle, int, int, int, int, int ) > wheel_event_handler_type;
  typedef boost::function< bool( ViewerHandle, int, int ) > key_press_event_handler_type;
  typedef boost::function< bool( ViewerHandle, int, int ) > key_release_event_handler_type;
  typedef boost::function< bool( ViewerHandle ) > cursor_handler_type;

  /// MOUSE_MOVE_EVENT:
  /// This function is called by the render widget when a mouse move event has occurred.
  /// This function needs to be overloaded to record mouse movements.
  virtual void mouse_move_event( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// MOUSE_PRESS_EVENT:
  /// This function is called by the render widget when a mouse button press event has occurred.
  /// This function needs to be overloaded to record mouse buttons being pressed.
  virtual void mouse_press_event( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// MOUSE_RELEASE_EVENT:
  /// This function is called by the render widget when a mouse button release event has occurred.
  /// This function needs to be overloaded to record mouse buttons being released.
  virtual void mouse_release_event( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// MOUSE_ENTER_EVENT:
  /// This function is called by the render widget when a mouse enter event has occurred.
  virtual void mouse_enter_event( int x, int y );

  /// MOUSE_LEAVE_EVENT:
  /// This function is called by the render widget when a mouse leave event has occurred.
  virtual void mouse_leave_event();

  /// WHEEL_EVENT:
  /// This function is called by the render widget when a mouse wheel event has occurred.
  /// This function needs to be overloaded to track mouse wheel events. 
  virtual bool wheel_event( int delta, int x, int y, int buttons, int modifiers );

  /// KEY_PRESS_EVENT:
  /// This function is called when a key is pressed while hovering over the render widget
  virtual bool key_press_event( int key, int modifiers, int x, int y );

  /// KEY_RELEASE_EVENT:
  /// This function is called when a key is released while hovering over the render widget
  virtual bool key_release_event( int key, int modifiers, int x, int y );

  void set_mouse_move_handler( mouse_event_handler_type func );
  void set_mouse_press_handler( mouse_event_handler_type func );
  void set_mouse_release_handler( mouse_event_handler_type func );
  void set_mouse_enter_handler( enter_event_handler_type func );
  void set_mouse_leave_handler( leave_event_handler_type func );
  void set_wheel_event_handler( wheel_event_handler_type func );
  void set_key_press_event_handler( key_press_event_handler_type func );
  void set_key_release_event_handler( key_press_event_handler_type func );
  void set_cursor_handler( cursor_handler_type func );
  void reset_mouse_handlers();

  /// IS_BUSY:
  /// Returns true if the mouse is pressed, otherwise false.
  bool is_busy();

  // -- Slice operations --
public:
  /// GET_VOLUME_SLICE:
  /// Returns the volume slice of the specified layer.
  Core::VolumeSliceHandle get_volume_slice( const std::string& layer_id );

  /// GET_ACTIVE_LAYER_SLICE:
  /// Returns the volume slice that corresponds to the active layer.
  Core::VolumeSliceHandle get_active_volume_slice() const;

private:
  friend class ActionOffsetSlice;

  /// OFFSET_SLICE:
  /// Offset the slice number by the given value.
  int offset_slice( int delta );

  /// MOVE_SLICE_TO:
  /// Move the slice to the given world coordinate. Used for picking.
  void move_slice_to( const Core::Point& pt );

  /// UPDATE_SLICE_VOLUME:
  /// Update the volume associated with the layer slice.
  void update_slice_volume( LayerHandle layer );

  // -- View information --
public:

  /// RESIZE:
  /// Set the new size of the viewer.
  virtual void resize( int width, int height );

  /// INSTALL_RENDERER:
  /// Install a renderer to the viewer.
  virtual void install_renderer( Core::AbstractRendererHandle renderer );

  /// AUTO_VIEW:
  /// Auto adjust the view for the active layer
  void auto_view();

  /// SNAP_TO_AXIS:
  /// Snap the eye position of the volume view to the closest axis.
  void snap_to_axis();

  /// IS_VOLUME_VIEW:
  /// Returns true if the current view mode is volume, otherwise false.
  bool is_volume_view() const;

  /// GET_ACTIVE_VIEW_STATE:
  /// Returns the view state variable associated with the current view mode.
  Core::StateViewBaseHandle get_active_view_state() const;

  /// WINDOW_TO_WORLD:
  /// Maps from window coordinates to world coordinates.
  /// NOTE: Only call this function when the viewer is in one of the 2D modes.
  /// Locks: StateEngine
  void window_to_world( int x, int y, double& world_x, double& world_y ) const;

  /// WORLD_TO_WINDOW:
  /// Maps from world coordinates to window coordinates.
  /// NOTE: Only call this function when the viewer is in one of the 2D modes.
  /// Locks: StateEngine
  void world_to_window( double world_x, double world_y, double& x, double& y ) const;

  /// GET_PROJECTION_MATRIX:
  /// Get the projection matrix of current view mode.
  /// NOTE: Only works in 2D modes.
  void get_projection_matrix( Core::Matrix& proj_mat ) const;

  /// UPDATE_STATUS_BAR:
  /// Update the status bar to show the data information of the specified layer under
  /// the mouse cursor. If no layer is specified, the active layer will be used.
  void update_status_bar( int x, int y, const std::string& layer_id = "" );

  // -- Rendering --
public:
  
  /// REDRAW_SCENE:
  /// Emits redraw_scene_signal_.
  void redraw_scene();

  /// REDRAW_OVERLAY:
  /// Emits redraw_overlay_signal_.
  void redraw_overlay();
  
  /// REDRAW_ALL:
  /// Emits both redraw_scene_signal_ and redraw_overlay_signal_
  void redraw_all();

  // -- Signals and Slots --
public:
  // Types of signals
  typedef boost::signals2::signal< void( Core::PickPointHandle ) > redraw_scene_pick_signal_type;
  typedef boost::signals2::signal< void ( size_t ) > slice_changed_signal_type;

  /// REDRAW_SCENE_PICK_SIGNAL:
  /// Signals that 3D pick point needs to be obtained from renderer.
  redraw_scene_pick_signal_type redraw_scene_pick_signal_;

  /// SLICE_CHANGED_SIGNAL_:
  /// Triggered when slice number or viewer visibility is changed.
  /// Renderer of other viewers connect to this signal to update the overlay.
  slice_changed_signal_type slice_changed_signal_;

  // -- State handling --
protected:
  /// PRE_LOAD_STATES:
  /// This virtual function is called by StateHandler::load_states before loading any states.
  virtual bool pre_load_states( const Core::StateIO& state_io );

  /// POST_LOAD_STATES:
  /// This function is called by StateHandler::load_states once the initial state 
  /// loading has finished.
  virtual bool post_load_states( const Core::StateIO& state_io );

  // -- State information --
public:

  Core::StateLabeledOptionHandle view_mode_state_;

  Core::StateView2DHandle axial_view_state_;
  Core::StateView2DHandle coronal_view_state_;
  Core::StateView2DHandle sagittal_view_state_;
  Core::StateView3DHandle volume_view_state_;
  Core::StateRangedIntHandle slice_number_state_;

  /// Whether to flip the 2D view horizontally
  Core::StateBoolHandle flip_horizontal_state_;
  /// Whether to flip the 2D view vertically
  Core::StateBoolHandle flip_vertical_state_;

  /// 2D viewer state
  Core::StateBoolHandle slice_grid_state_;
  Core::StateBoolHandle slice_visible_state_;
  Core::StateBoolHandle slice_picking_visible_state_;

  /// 3D viewer state
  Core::StateBoolHandle volume_slices_visible_state_;
  Core::StateBoolHandle volume_isosurfaces_visible_state_;
  Core::StateBoolHandle volume_volume_rendering_visible_state_;
  Core::StateBoolHandle volume_light_visible_state_;
  Core::StateBoolHandle volume_enable_fog_state_;
  Core::StateBoolHandle volume_enable_clipping_state_;
  Core::StateBoolHandle volume_show_invisible_slices_state_;
  Core::StateBoolHandle volume_show_bounding_box_state_;

  Core::StateBoolHandle lock_state_;
  Core::StateBoolHandle overlay_visible_state_;
  Core::StateBoolHandle is_picking_target_state_;

public:
  const static std::string AXIAL_C;
  const static std::string SAGITTAL_C;
  const static std::string CORONAL_C;
  const static std::string VOLUME_C;

private:
  ViewerPrivateHandle private_;

};

} // end namespace Seg3D

#endif
