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

namespace Seg3D
{

// Forward declarations
class ViewerManager;
class ViewerInfo;
typedef boost::shared_ptr< ViewerInfo > ViewerInfoHandle;
typedef std::vector< ViewerInfoHandle > ViewerInfoList;

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

  void get_2d_viewers_info( ViewerInfoList viewers[ 3 ] );
  void pick_point( size_t source_viewer, const Core::Point& pt );
  std::vector< size_t > get_locked_viewers( int mode_index );

  // -- State information --
public:
  Core::StateOptionHandle layout_state_;
  Core::StateIntHandle active_viewer_state_;

  // -- Signals and slots --
private:
  void viewer_mode_changed( size_t viewer_id );
  void viewer_visibility_changed( size_t viewer_id );
  void viewer_became_picking_target( size_t viewer_id );
  void viewer_lock_state_changed( size_t viewer_id );
  void update_picking_targets();

public:
  typedef boost::signals2::signal< void ( size_t ) > picking_target_changed_signal_type;
  picking_target_changed_signal_type picking_target_changed_signal_;

  // -- Viewer information --
private:

  std::vector< ViewerHandle > viewers_;
  std::vector< size_t > locked_viewers_[ 4 ];

  int active_axial_viewer_;
  int active_coronal_viewer_;
  int active_sagittal_viewer_;

  size_t signal_block_count_;

}; // class ViewerManager

} // end namespace Seg3D

#endif
