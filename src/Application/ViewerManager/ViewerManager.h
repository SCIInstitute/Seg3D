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

#ifndef APPLICATION_VIEWER_VIEWERMANAGER_H
#define APPLICATION_VIEWER_VIEWERMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <string>
#include <vector>

// Boost includes 
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/State/StateHandler.h>
#include <Core/VolumeRenderer/TransferFunction.h>

// Application includes
#include <Application/Viewer/Viewer.h>

namespace Seg3D
{

// Forward declarations
class ViewerManager;
class ViewerManagerPrivate;
class ViewerInfo;
typedef boost::shared_ptr< ViewerInfo > ViewerInfoHandle;
typedef std::vector< ViewerInfoHandle > ViewerInfoList;
typedef boost::shared_ptr< ViewerManagerPrivate > ViewerManagerPrivateHandle;

class ViewerInfo
{
public:
  size_t viewer_id_;
  int view_mode_;
  double depth_;
  bool is_picking_target_;
};

// typedefs
class ViewerManager : public Core::StateHandler
{
  CORE_SINGLETON( ViewerManager );

  // -- Constructor/Destructor --
private:
  ViewerManager();
  virtual ~ViewerManager();

  // -- Getting information from manager --

public:

  /// NUMBER_OF_VIEWERS
  /// The number of viewers that is available in the application
  /// This is a preset number to simplify the dependency scheme
  size_t number_of_viewers()
  {
    return 6;
  }

  /// GET_VIEWER:
  /// Returns the specified viewer.
  ViewerHandle get_viewer( size_t idx );
  ViewerHandle get_viewer( const std::string viewer_name );

  /// GET_ACTIVE_VIEWER:
  /// Returns the active viewer.
  ViewerHandle get_active_viewer();

  /// GET_2D_VIEWERS_INFO:
  /// Get a snapshot of all the 2D viewers.
  void get_2d_viewers_info( ViewerInfoList viewers[ 3 ] );

  /// GET_LOCKED_VIEWERS:
  /// Returns the IDs of locked viewers in the specified view mode.
  std::vector< size_t > get_locked_viewers( int mode_index );

  /// UPDATE_VIEWERS:
  /// Cause all the visible viewers in the specified mode to redraw overlay.
  void update_viewers_overlay( const std::string& view_mode );

  /// UPDATE_2D_VIEWERS:
  /// Cause all the visible 2D viewers to redraw overlay.
  void update_2d_viewers_overlay();

  /// UPDATE_VOLUME_VIEWERS:
  /// Cause all the visible volume viewers to redraw.
  void update_volume_viewers();

  /// RESET_CURSOR:
  /// Reset the cursors of all the viewers to default.
  void reset_cursor();

  /// IS_BUSY:
  /// Returns true if the mouse is pressed in any viewer, otherwise false.
  bool is_busy();

  /// GET_TRANSFER_FUNCTION:
  /// Returns a const handle to the transfer function.
  Core::TransferFunctionHandle get_transfer_function();

private:
  friend class ActionPickPoint;
  friend class ActionNewFeature;
  friend class ActionDeleteFeature;

  /// PICK_POINT:
  /// Move the 2D viewers that are currently set as picking targets to 
  /// the specified position in world space.  Exclude the source viewer.
  void pick_point( size_t source_viewer, const Core::Point& pt );

  /// PICK_POINT:
  /// Move the 2D viewers that are currently set as picking targets to 
  /// the specified position in world space.
  void pick_point( const Core::Point& pt );

  /// ADD_NEW_FEATURE:
  /// Add a new transfer function feature.
  void add_new_feature();

  /// DELETE_FEATURE:
  /// Delete the specified transfer function feature.
  void delete_feature( const std::string& feature_id );

  // -- State information --
public:

  /// Which layout is used to display the data (single, 1and1, 1and2, 1and3, 2and2, 3and3 )
  Core::StateOptionHandle layout_state_;
  
  /// Number of the viewer that is the active viewer
  Core::StateIntHandle active_viewer_state_;
  
  /// The density of the fog in 3D view
  Core::StateRangedDoubleHandle fog_density_state_;

  /// The target data layer for volume rendering
  Core::StateLabeledOptionHandle volume_rendering_target_state_;

  /// Which volume renderer to use.
  Core::StateLabeledOptionHandle volume_renderer_state_;

  /// The sampling rate of volume rendering
  Core::StateRangedDoubleHandle volume_sample_rate_state_;

  /// The aperture angle of the cone for occlusion volume rendering
  Core::StateRangedDoubleHandle vr_occlusion_angle_state_;

  /// The grid resolution for sampling the cone base
  Core::StateRangedIntHandle vr_occlusion_grid_resolution_state_;

  /// Clipping planes enabled state
  Core::StateBoolHandle enable_clip_plane_state_[ 6 ];

  /// X component of clipping plane normals
  Core::StateRangedDoubleHandle clip_plane_x_state_[ 6 ];

  /// Y component of clipping plane normals
  Core::StateRangedDoubleHandle clip_plane_y_state_[ 6 ];

  /// Z component of clipping plane normals
  Core::StateRangedDoubleHandle clip_plane_z_state_[ 6 ];

  /// Distance of the clipping planes
  Core::StateRangedDoubleHandle clip_plane_distance_state_[ 6 ];

  /// Whether to reverse the normal of clipping planes
  Core::StateBoolHandle clip_plane_reverse_norm_state_[ 6 ];

  /// Whether to show controls for fog
  Core::StateBoolHandle show_fog_control_state_;

  /// Whether to show controls for clipping
  Core::StateBoolHandle show_clipping_control_state_;

  /// Whether to show controls for volume rendering
  Core::StateBoolHandle show_volume_rendering_control_state_;

  /// TODO: The next ones should not be state variables
  /// --JGS
  /// Number of the viewer that is the active axial viewer ( for picking )
  Core::StateIntHandle active_axial_viewer_;
  
  /// Number of the viewer that is the active coronal viewer ( for picking )
  Core::StateIntHandle active_coronal_viewer_;
  
  /// Number of the viewer that is the active  sagittal viewer ( for picking )
  Core::StateIntHandle active_sagittal_viewer_;

public:
  typedef boost::signals2::signal< void ( size_t ) > picking_target_changed_signal_type;
  picking_target_changed_signal_type picking_target_changed_signal_;

protected:
  /// POST_SAVE_STATES:
  /// This function is called by the statehandler once the initial state saving has finished and
  /// will then save the states of each of the viewers.
  virtual bool post_save_states( Core::StateIO& state_io );

  /// PRE_LOAD_STATES:
  /// This virtual function is called by StateHandler::load_states before loading any states.
  virtual bool pre_load_states( const Core::StateIO& state_io );

  /// POST_LOAD_STATES:
  /// This function is called by the statehandler once the initial state loading has finished and
  /// will then load the states of each of the viewers.
  virtual bool post_load_states( const Core::StateIO& state_io );

public:
  virtual int get_session_priority();

  // -- Viewer information --
private:
  ViewerManagerPrivateHandle private_;

  // -- Layout options --
public:
  static const std::string VIEW_SINGLE_C;
  static const std::string VIEW_1AND1_C;
  static const std::string VIEW_1AND2_C;
  static const std::string VIEW_1AND3_C;
  static const std::string VIEW_2AND2_C;
  static const std::string VIEW_2AND3_C;
  static const std::string VIEW_3AND3_C;

}; // class ViewerManager

} // end namespace Seg3D

#endif
