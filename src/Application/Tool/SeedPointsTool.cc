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

#include <Core/Geometry/Color.h>
#include <Core/State/StateEngine.h>
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionClear.h>
#include <Core/State/Actions/ActionRemove.h>
#include <Core/Viewer/Mouse.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/LayerManager.h>
#include <Application/Tool/SeedPointsTool.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

namespace Seg3D
{

const Core::Color SeedPointsTool::yellow( 1.0f, 1.0f, 0.0f );
const Core::Color SeedPointsTool::dark_yellow( 0.6f, 0.6f, 0.0f );

//////////////////////////////////////////////////////////////////////////
// Implementation of class  SeedPointsToolPrivate
//////////////////////////////////////////////////////////////////////////
  
class SeedPointsToolPrivate
{
public:
  Core::VolumeSliceHandle get_target_slice( ViewerHandle viewer );
  SeedPointsTool* tool_;
};

Core::VolumeSliceHandle SeedPointsToolPrivate::get_target_slice( ViewerHandle viewer )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  Core::VolumeSliceHandle vol_slice;
  
  std::string target_layer_id = this->tool_->target_layer_state_->get();
  if ( target_layer_id == Tool::NONE_OPTION_C )
  {
    return vol_slice;
  }
  vol_slice = viewer->get_volume_slice( target_layer_id );
  
  return vol_slice;
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class SeedPointsTool
//////////////////////////////////////////////////////////////////////////

SeedPointsTool::SeedPointsTool( Core::VolumeType target_volume_type, const std::string& toolid ) :
  SingleTargetTool( target_volume_type, toolid ),
  private_( new SeedPointsToolPrivate )
{
  this->private_->tool_ = this;

  this->add_state( "seed_points", this->seed_points_state_ );

  this->add_connection( this->seed_points_state_->state_changed_signal_.connect( 
    boost::bind( &SeedPointsTool::handle_seed_points_changed, this ) ) );
}

SeedPointsTool::~SeedPointsTool()
{
  this->disconnect_all();
}

bool SeedPointsTool::handle_mouse_press( ViewerHandle viewer, 
                    const Core::MouseHistory& mouse_history, 
                    int button, int buttons, int modifiers )
{
  Core::VolumeSliceHandle target_slice;
  double world_x, world_y;

  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

    if ( viewer->is_volume_view() )
    {
      return false;
    }
    
    target_slice = this->private_->get_target_slice( viewer );
    if ( !target_slice || target_slice->out_of_boundary() )
    {
      return false;
    }

    viewer->window_to_world( mouse_history.current_.x_, 
      mouse_history.current_.y_, world_x, world_y );
  }

  if ( button == Core::MouseButton::LEFT_BUTTON_E && 
    ( modifiers == Core::KeyModifier::NO_MODIFIER_E || 
    modifiers & ( Core::KeyModifier::ALT_MODIFIER_E) ) )
  {
    int u, v;
    target_slice->world_to_index( world_x, world_y, u, v );
    if ( u >= 0 && u < static_cast< int >( target_slice->nx() ) &&
      v >= 0 && v < static_cast< int >( target_slice->ny() ) )
    {
      Core::Point pos;
      target_slice->get_world_coord( world_x, world_y, pos );
      Core::ActionAdd::Dispatch( Core::Interface::GetMouseActionContext(),
        this->seed_points_state_, pos );

      return true;
    }
  }
  else if ( button == Core::MouseButton::RIGHT_BUTTON_E && 
    ( modifiers == Core::KeyModifier::NO_MODIFIER_E || 
    modifiers & (Core::KeyModifier::ALT_MODIFIER_E) )  )
  {
    Core::Point pt;
    if ( this->find_point( viewer, world_x, world_y, target_slice, pt ) )
    {
      Core::ActionRemove::Dispatch( Core::Interface::GetMouseActionContext(),
        this->seed_points_state_, pt );
    }
    return true;
  }
  
  return false;
}

bool SeedPointsTool::handle_mouse_move( ViewerHandle viewer, 
                     const Core::MouseHistory& mouse_history, 
                     int button, int buttons, int modifiers )
{
  if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    buttons == Core::MouseButton::NO_BUTTON_E )
  {
    std::string target_id;
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      target_id = this->target_layer_state_->get();
    }
    if ( target_id != Tool::NONE_OPTION_C )
    {
      viewer->update_status_bar( mouse_history.current_.x_,
        mouse_history.current_.y_, target_id );
      return true;
    }
  }
  
  return false;
}

void SeedPointsTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat,
                             int viewer_width, int viewer_height )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( viewer->is_volume_view() )
  {
    return;
  }

  Core::VolumeSliceHandle vol_slice = this->private_->get_target_slice( viewer );
  if ( ! vol_slice )
  {
    return;
  }
  
  std::vector< Core::Point > seed_points;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    seed_points = this->seed_points_state_->get();
  }

  size_t num_of_pts = seed_points.size();
  if ( num_of_pts == 0 )
  {
    return;
  }
  
  glPushAttrib( GL_LINE_BIT );
  glLineWidth( 1.0f );

  for ( auto &seed_point : seed_points )
  {
    double x_pos, y_pos;
    vol_slice->project_onto_slice( seed_point, x_pos, y_pos );
    Core::Point pt( x_pos, y_pos, 0 );
    pt = proj_mat * pt;
    int x = static_cast< int >( ( pt[ 0 ] + 1.0 ) * 0.5 * ( viewer_width - 1 ) );
    int y = static_cast< int >( ( pt[ 1 ] + 1.0 ) * 0.5 * ( viewer_height - 1 ) );
    int slice_num = vol_slice->get_closest_slice( seed_point );
    bool in_slice = ( ! vol_slice->out_of_boundary() &&
                      static_cast< int >( vol_slice->get_slice_number() ) == slice_num );

    Core::Color color = in_slice ? yellow : dark_yellow;
    glColor3f( color.r(), color.g(), color.b() );
    glBegin( GL_LINES );
      glVertex2i( x - 5, y );
      glVertex2i( x + 5, y );
      glVertex2i( x, y - 5 );
      glVertex2i( x, y + 5 );
    glEnd();
  }
  
  glPopAttrib();
}

bool SeedPointsTool::find_point( ViewerHandle viewer, double world_x, double world_y,
                                 Core::VolumeSliceHandle vol_slice, Core::Point& pt )
{
  // Step 1. Compute the size of a pixel in world space
  double x0, y0, x1, y1;
  viewer->window_to_world( 0, 0, x0, y0 );
  viewer->window_to_world( 1, 1, x1, y1 );
  double pixel_width = Core::Abs( x1 - x0 );
  double pixel_height = Core::Abs( y1 - y0 );

  // Step 2: Search for the first seed point that's within 4 pixels in each direction
  // from the given search position (world_x, world_y)
  const int NEIGHBORHOOD_SIZE = 4;
  double range_x = pixel_width * NEIGHBORHOOD_SIZE;
  double range_y = pixel_height * NEIGHBORHOOD_SIZE;
  std::vector< Core::Point > seed_points;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    seed_points = this->seed_points_state_->get();
  }

  for ( auto &seed_point : seed_points )
  {
    double pt_x, pt_y;
    vol_slice->project_onto_slice( seed_point, pt_x, pt_y );
    if ( Core::Abs( pt_x - world_x ) <= range_x &&
         Core::Abs( pt_y - world_y ) <= range_y )
    {
      pt = seed_point;
      return true;
    }
  }
  
  return false;
}

void SeedPointsTool::clear( Core::ActionContextHandle context )
{
  Core::ActionClear::Dispatch( context, this->seed_points_state_ );
}

bool SeedPointsTool::has_2d_visual()
{
  return true;
}

bool SeedPointsTool::has_3d_visual()
{
  return false;
}

void SeedPointsTool::handle_seed_points_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

bool SeedPointsTool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
  if ( key == Core::Key::KEY_C_E )
  {
    Core::ActionClear::Dispatch( Core::Interface::GetKeyboardActionContext(),
      this->seed_points_state_ );
    return true;
  }
  
  return SingleTargetTool::handle_key_press( viewer, key, modifiers );
}


} // end namespace Seg3D