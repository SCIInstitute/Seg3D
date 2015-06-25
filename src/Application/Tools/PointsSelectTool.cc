/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>

#include <Core/Utils/Exception.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Geometry/Point.h>

#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PointsSelectTool.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ViewerManager/ViewerManager.h>

SCI_REGISTER_TOOL( Seg3D, PointsSelectTool )

namespace Seg3D
{

class PointsSelectToolPrivate : public Core::Lockable
{
public:
  typedef std::vector< Core::Point > PointVector;

  void handle_target_layer_changed();
  void handle_units_selection_changed( std::string units );
  void update_unit_point_list();
  void update_viewers();

  PointsSelectTool* tool_;
  ViewerHandle viewer_;
};

// Generate seed points in index coordinates
void
PointsSelectToolPrivate::update_unit_point_list()
{
  lock_type lock( this->get_mutex() );

  if ( ! this->viewer_ )
  {
    return;
  }

  if ( this->viewer_->is_volume_view() )
  {
    return;
  }

  bool convert_units = true;

  // We need access to the use_world_units_state_, and several functions we call also lock
  // the state engine.  
  Core::StateEngine::lock_type stateLock( Core::StateEngine::GetMutex() );
  convert_units = ! this->tool_->use_world_units_state_->get();

  Core::VolumeSliceHandle active_slice = this->viewer_->get_active_volume_slice();
  // seed points are in world coordinates by default
  if ( convert_units && active_slice )
  {
    std::string view_mode = this->viewer_->view_mode_state_->get();

    this->tool_->seed_points_index_state_->clear();

    PointVector::const_iterator iter = this->tool_->seed_points_state_->get().begin();
    PointVector::const_iterator end = this->tool_->seed_points_state_->get().end();
    while (iter != end)
    {
      int i = -1, j = -1;

      if ( view_mode == Viewer::AXIAL_C )
      {
        active_slice->world_to_index(iter->x(), iter->y(), i, j);
      }
      else if ( view_mode == Viewer::CORONAL_C )
      {
        active_slice->world_to_index(iter->x(), iter->z(), i, j);
      }
      else if ( view_mode == Viewer::SAGITTAL_C )
      {
        active_slice->world_to_index(iter->y(), iter->z(), i, j);
      }
      else
      {
        // this should not happen!
        CORE_LOG_ERROR("Viewer is not volume, axial, coronal or sagittal.");
      }

      Core::Point indexPoint;
      active_slice->to_index(i, j, indexPoint);
      this->tool_->seed_points_index_state_->add(indexPoint);
      ++iter;
    }
  }
}

void
PointsSelectToolPrivate::handle_units_selection_changed( std::string units )
{
  // Don't need to lock state engine because we're running on app thread
  ASSERT_IS_APPLICATION_THREAD();
  
  bool old_show_world_units_state = this->tool_->use_world_units_state_->get();
  
  if ( units == PointsSelectTool::WORLD_UNITS_C )
  {
    this->tool_->use_world_units_state_->set( true );
  }
  else
  {
    this->tool_->use_world_units_state_->set( false );
  }
  
  // If units have changed, emit signal
  if ( old_show_world_units_state != this->tool_->use_world_units_state_->get() )
  {
    this->update_unit_point_list();
    this->tool_->units_changed_signal_();
  }
}

void
PointsSelectToolPrivate::update_viewers()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void
PointsSelectToolPrivate::handle_target_layer_changed()
{
  std::string target_layer_id = this->tool_->target_layer_state_->get();
  if ( target_layer_id != Tool::NONE_OPTION_C )
  {
    if ( this->tool_->seed_points_state_->get().size() > 0 )
    {
      this->tool_->handle_seed_points_changed();
    }
  }

  this->update_viewers();
}

const std::string PointsSelectTool::INDEX_UNITS_C( "index_units" );
const std::string PointsSelectTool::WORLD_UNITS_C( "world_units" );

PointsSelectTool::PointsSelectTool( const std::string& toolid ) :
  SeedPointsTool( Core::VolumeType::ALL_REGULAR_E, toolid ),
  private_( new PointsSelectToolPrivate )
{
  this->private_->tool_ = this;

  this->add_state( "units_selection", this->units_selection_state_, WORLD_UNITS_C, 
                  INDEX_UNITS_C + "=Pixel|" +
                  WORLD_UNITS_C + "=Actual" );
  this->add_state( "show_world_units", this->use_world_units_state_, true );
  this->add_state( "seed_points_index", this->seed_points_index_state_ );
  
  this->private_->handle_target_layer_changed();

  this->add_connection( this->target_layer_state_->state_changed_signal_.connect(
    boost::bind( &PointsSelectToolPrivate::handle_target_layer_changed, this->private_ ) ) );
  this->add_connection( this->units_selection_state_->value_changed_signal_.connect(
    boost::bind( &PointsSelectToolPrivate::handle_units_selection_changed, this->private_, _2 ) ) );
}
  
PointsSelectTool::~PointsSelectTool()
{
  this->disconnect_all();
}

bool
PointsSelectTool::handle_mouse_press( ViewerHandle viewer, 
                                      const Core::MouseHistory& mouse_history, 
                                      int button, int buttons, int modifiers )
{
  this->private_->viewer_ = viewer;
  return SeedPointsTool::handle_mouse_press( viewer, mouse_history, button, buttons, modifiers );
}

void
PointsSelectTool::handle_seed_points_changed()
{
  std::string target_layer_id = this->target_layer_state_->get();
  if ( target_layer_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  this->private_->update_unit_point_list();
  this->private_->update_viewers();
}

void
PointsSelectTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat, int viewer_width, int viewer_height )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  std::string target_layer_id;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    target_layer_id = this->target_layer_state_->get();
  }
  
  if ( target_layer_id == Tool::NONE_OPTION_C )
  {
    return;
  }
  
  SeedPointsTool::redraw( viewer_id, proj_mat, viewer_width, viewer_height );
}    

}