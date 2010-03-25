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

#ifndef APPLICATION_VIEWER_VIEWER_H
#define APPLICATION_VIEWER_VIEWER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <map>
#include <vector>

// Boost includes 
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/State/State.h>
#include <Application/Viewer/ViewerRenderer.h>
#include <Application/Viewer/ViewManipulator.h>

#include <Utils/Core/EnumClass.h>
#include <Utils/Volume/DataVolumeSlice.h>
#include <Utils/Volume/MaskVolumeSlice.h>

namespace Seg3D
{

// Enums for mouse buttons 
// they have the same values as corresponding Qt ones
SCI_ENUM_CLASS
(
  MouseButton,
  NO_BUTTON_E = 0x00000000,
  LEFT_BUTTON_E = 0x00000001,
  RIGHT_BUTTON_E = 0x00000002,
  MID_BUTTON_E = 0x00000004
)

// Enums for key modifiers
// they have the same values as corresponding Qt ones
SCI_ENUM_CLASS
(
  KeyModifier,
  NO_MODIFIER_E = 0x00000000,
  SHIFT_MODIFIER_E = 0x02000000,
  CONTROL_MODIFIER_E = 0x04000000,
  ALT_MODIFIER_E = 0x08000000
)

class MousePosition
{
public:
  MousePosition() :
    x( 0 ), y( 0 )
  {
  }

  int x;
  int y;
};

class MouseHistory
{
public:
  MousePosition left_start;
  MousePosition right_start;
  MousePosition mid_start;
  MousePosition previous;
  MousePosition current;
};

// Forward declarations
class ViewManipulator;
class Viewer;
typedef boost::shared_ptr< Viewer > ViewerHandle;
typedef boost::weak_ptr< Viewer > ViewerWeakHandle;

// Class definition
class Viewer : public StateHandler
{

  // -- constructor/destructor --
public:
  Viewer( const std::string& key );
  virtual ~Viewer();

  // -- mouse events handling --
public:

  typedef boost::function< bool( const MouseHistory&, int, int, int ) > mouse_event_handler_type;

  void mouse_move_event( const MouseHistory& mouse_history, int button, int buttons,
      int modifiers );
  void mouse_press_event( const MouseHistory& mouse_history, int button, int buttons,
      int modifiers );
  void mouse_release_event( const MouseHistory& mouse_history, int button, int buttons,
      int modifiers );
  bool wheel_event( int delta, int x, int y, int buttons, int modifiers );

  void set_mouse_move_handler( mouse_event_handler_type func );
  void set_mouse_press_handler( mouse_event_handler_type func );
  void set_mouse_release_handler( mouse_event_handler_type func );
  void reset_mouse_handlers();

private:
  void update_status_bar( int x, int y );

  mouse_event_handler_type mouse_move_handler_;
  mouse_event_handler_type mouse_press_handler_;
  mouse_event_handler_type mouse_release_handler_;
  ViewManipulatorHandle view_manipulator_;

public:
  void resize( int width, int height );
  bool is_volume_view() const;
  StateViewBaseHandle get_active_view_state();

protected:
  virtual void state_changed();

  // -- Signals and Slots --
public:

  typedef boost::signals2::signal< void() > redraw_signal_type;
  redraw_signal_type redraw_signal_;

private:
  void change_view_mode( std::string mode, ActionSource source );
  void set_slice_number( int num, ActionSource source = ActionSource::NONE_E );

  // -- Data structures for keeping track of slices of layers --
private:
  typedef std::map< std::string, Utils::MaskVolumeSliceHandle > mask_slices_map_type;
  typedef std::map< std::string, Utils::DataVolumeSliceHandle > data_slices_map_type;

  mask_slices_map_type mask_slices_;
  data_slices_map_type data_slices_;
  Utils::VolumeSliceHandle active_layer_slice_;

  void insert_layer( LayerHandle layer );
  void delete_layer( LayerHandle layer );
  void set_active_layer( LayerHandle layer );

public:
  Utils::MaskVolumeSliceHandle get_mask_volume_slice( const std::string& layer_id );
  Utils::DataVolumeSliceHandle get_data_volume_slice( const std::string& layer_id );

  // -- Mutex and lock --
public: 
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  mutex_type& get_mutex()
  {
    return this->layer_map_mutex_;
  }

private:
  mutex_type layer_map_mutex_;

  // -- Other functions and variables --
public:
  // Auto adjust the view for the active layer
  void auto_view();

  // Get the stateid prefix of the viewer
  const std::string& get_stateid() const
  {
    return this->stateid();
  }

private:
  
  // Auto adjust the view states so the slices are fully visible
  void adjust_view();

  // Move the active slices to the center of the volume
  void adjust_depth();

  void push_ignoring_status() 
  { 
    this->ignoring_status_stack_.push_back( this->ignore_state_changes_ ); 
  }

  void pop_ignoring_status()
  {
    this->ignore_state_changes_ = this->ignoring_status_stack_.back();
    this->ignoring_status_stack_.pop_back();
  }

private:
  int width_;
  int height_;
  bool ignore_state_changes_;
  std::vector< bool > ignoring_status_stack_;
  bool updating_states_; // Indicates if it's in the middle of state changes

  // -- State information --
public:

  StateOptionHandle view_mode_state_;

  StateView2DHandle axial_view_state_;
  StateView2DHandle coronal_view_state_;
  StateView2DHandle sagittal_view_state_;
  StateView3DHandle volume_view_state_;

  StateRangedIntHandle slice_number_state_;

  StateBoolHandle slice_lock_state_;
  StateBoolHandle slice_grid_state_;
  StateBoolHandle slice_visible_state_;

  StateBoolHandle volume_lock_state_;
  StateBoolHandle volume_slices_visible_state_;
  StateBoolHandle volume_isosurfaces_visible_state_;
  StateBoolHandle volume_volume_rendering_visible_state_;

private:
  // Indexed view state variables for quick access
  StateViewBaseHandle view_states_[ 4 ];

public:
  const static std::string AXIAL_C;
  const static std::string SAGITTAL_C;
  const static std::string CORONAL_C;
  const static std::string VOLUME_C;

};

} // end namespace Seg3D

#endif
