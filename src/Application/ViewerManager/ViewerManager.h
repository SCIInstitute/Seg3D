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

#ifndef APPLICATION_VIEWER_VIEWERMANAGER_H
#define APPLICATION_VIEWER_VIEWERMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <string>
#include <vector>

// Boost includes 
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

// Application includes
#include <Application/Viewer/Viewer.h>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/State/State.h>
#include <Core/State/StateSet.h>

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

  // NUMBER_OF_VIEWERS
  // The number of viewers that is available in the application
  // This is a preset number to simplify the dependency scheme
  size_t number_of_viewers()
  {
    return 6;
  }

  ViewerHandle get_viewer( size_t idx );
  ViewerHandle get_viewer( const std::string viewer_name );
  ViewerHandle get_active_viewer();

  void get_2d_viewers_info( ViewerInfoList viewers[ 3 ] );
  void pick_point( size_t source_viewer, const Core::Point& pt );
  std::vector< size_t > get_locked_viewers( int mode_index );

  // UPDATE_VIEWERS:
  // Cause all the visible viewers in the specified mode to redraw overlay.
  void update_viewers_overlay( const std::string& view_mode );

  // UPDATE_2D_VIEWERS:
  // Cause all the visible 2D viewers to redraw overlay.
  void update_2d_viewers_overlay();

  // UPDATE_VOLUME_VIEWERS:
  // Cause all the visible volume viewers to redraw.
  void update_volume_viewers();

  // RESET_CURSOR:
  // Reset the cursors of all the viewers to default.
  void reset_cursor();

  // IS_BUSY:
  // Returns true if the mouse is pressed in any viewer, otherwise false.
  bool is_busy();

  // -- State information --
public:

  // Which layout is used to display the data (single, 1and1, 1and2, 1and3, 2and2, 3and3 )
  Core::StateOptionHandle layout_state_;
  
  // Number of the viewer that is the active viewer
  Core::StateIntHandle active_viewer_state_;
  
  // The density of the fog in 3D view
  Core::StateRangedDoubleHandle fog_density_state_;

  // Clipping planes enabled state
  Core::StateBoolHandle enable_clip_plane_state_[ 6 ];

  // X component of clipping plane normals
  Core::StateRangedDoubleHandle clip_plane_x_state_[ 6 ];

  // Y component of clipping plane normals
  Core::StateRangedDoubleHandle clip_plane_y_state_[ 6 ];

  // Z component of clipping plane normals
  Core::StateRangedDoubleHandle clip_plane_z_state_[ 6 ];

  // Distance of the clipping planes
  Core::StateRangedDoubleHandle clip_plane_distance_state_[ 6 ];

  // Whether to reverse the normal of clipping planes
  Core::StateBoolHandle clip_plane_reverse_norm_state_[ 6 ];

  // Whether to show controls for fog
  Core::StateBoolHandle show_fog_control_state_;

  // Whether to show controls for clipping
  Core::StateBoolHandle show_clipping_control_state_;

  // TODO: The next ones should not be state variables
  // --JGS
  // Number of the viewer that is the active axial viewer ( for picking )
  Core::StateIntHandle active_axial_viewer_;
  
  // Number of the viewer that is the active coronal viewer ( for picking )
  Core::StateIntHandle active_coronal_viewer_;
  
  // Number of the viewer that is the active  sagittal viewer ( for picking )
  Core::StateIntHandle active_sagittal_viewer_;

public:
  typedef boost::signals2::signal< void ( size_t ) > picking_target_changed_signal_type;
  picking_target_changed_signal_type picking_target_changed_signal_;

protected:
  // POST_SAVE_STATES:
  // This function is called by the statehandler once the initial state saving has finished and
  // will then save the states of each of the viewers.
  virtual bool post_save_states( Core::StateIO& state_io );

  // PRE_LOAD_STATES:
  // This virtual function is called by StateHandler::load_states before loading any states.
  virtual bool pre_load_states( const Core::StateIO& state_io );

  // POST_LOAD_STATES:
  // This function is called by the statehandler once the initial state loading has finished and
  // will then load the states of each of the viewers.
  virtual bool post_load_states( const Core::StateIO& state_io );

public:
  virtual int get_session_priority();

  // -- Viewer information --
private:
  ViewerManagerPrivateHandle private_;

  // -- Layout options --
public:
  static const std::string SINGLE_C;
  static const std::string _1AND1_C;
  static const std::string _1AND2_C;
  static const std::string _1AND3_C;
  static const std::string _2AND2_C;
  static const std::string _2AND3_C;
  static const std::string _3AND3_C;

}; // class ViewerManager

} // end namespace Seg3D

#endif
