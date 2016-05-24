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

#include <Core/State/Actions/ActionOffset.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/RenderResources/RenderResources.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/CropTool.h>
#include <Application/Filters/Actions/ActionCrop.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ViewerManager/ViewerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, CropTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class CropToolPrivate
//////////////////////////////////////////////////////////////////////////

class CropToolPrivate
{
public:
  CropToolPrivate() : hit_pos_( Core::HitPosition::NONE_E ) {}

  void handle_target_group_changed();
  void handle_cropbox_origin_changed( int index, double value, Core::ActionSource source );
  void handle_cropbox_size_changed( int index, double value );
  void handle_cropbox_changed( Core::ActionSource source );

  void handle_cropbox_origin_index_changed( int index, int value );
  void handle_cropbox_size_index_changed( int index, int value );

  void hit_test( ViewerHandle viewer, int x, int y );
  void update_cursor( ViewerHandle viewer );
  void resize( ViewerHandle viewer, int x0, int y0, int x1, int y1 );

  size_t signal_block_count_;
  CropTool* tool_;
  int hit_pos_;
  int ver_index_; // Index of the vertical position state
  int hor_index_; // Index of the horizontal position state
  bool resizing_;
};

void CropToolPrivate::handle_target_group_changed()
{
  const std::string& group_id = this->tool_->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  // NOTE: Lock the state engine because the following changes need to be atomic
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  LayerGroupHandle layer_group = LayerManager::Instance()->find_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  int nx = static_cast< int >( grid_trans.get_nx() );
  int ny = static_cast< int >( grid_trans.get_ny() );
  int nz = static_cast< int >( grid_trans.get_nz() );
  this->tool_->input_dimensions_state_[ 0 ]->set( nx );
  this->tool_->input_dimensions_state_[ 1 ]->set( ny );
  this->tool_->input_dimensions_state_[ 2 ]->set( nz );

  // Compute the bounding box of the layer group in world space.
  // NOTE: The bounding box is extended by half a voxel in each direction
  Core::Point origin( -0.5, -0.5, -0.5 );
  origin = grid_trans * origin;
  Core::Point end( nx - 0.5, ny - 0.5, nz - 0.5 );
  end = grid_trans * end;

  this->tool_->cropbox_origin_state_[ 0 ]->set_range( origin[ 0 ], end[ 0 ] );
  this->tool_->cropbox_origin_state_[ 1 ]->set_range( origin[ 1 ], end[ 1 ] );
  this->tool_->cropbox_origin_state_[ 2 ]->set_range( origin[ 2 ], end[ 2 ] );
  this->tool_->cropbox_size_state_[ 0 ]->set_range( 0, 
    end[ 0 ] - this->tool_->cropbox_origin_state_[ 0 ]->get() );
  this->tool_->cropbox_size_state_[ 1 ]->set_range( 0, 
    end[ 1 ] - this->tool_->cropbox_origin_state_[ 1 ]->get() );
  this->tool_->cropbox_size_state_[ 2 ]->set_range( 0, 
    end[ 2 ] - this->tool_->cropbox_origin_state_[ 2 ]->get() );

  this->tool_->cropbox_origin_index_state_[ 0 ]->set_range( 0, nx - 1 );
  this->tool_->cropbox_origin_index_state_[ 1 ]->set_range( 0, ny - 1 );
  this->tool_->cropbox_origin_index_state_[ 2 ]->set_range( 0, nz - 1 );
  this->tool_->cropbox_size_index_state_[ 0 ]->set_range( 1,
    nx - this->tool_->cropbox_origin_index_state_[ 0 ]->get() );
  this->tool_->cropbox_size_index_state_[ 1 ]->set_range( 1,
    ny - this->tool_->cropbox_origin_index_state_[ 1 ]->get() );
  this->tool_->cropbox_size_index_state_[ 2 ]->set_range( 1,
    nz - this->tool_->cropbox_origin_index_state_[ 2 ]->get() );

  state_lock.unlock();
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void CropToolPrivate::handle_cropbox_changed( Core::ActionSource source )
{
  if ( source != Core::ActionSource::INTERFACE_MOUSE_E &&
    this->signal_block_count_ == 0 )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  }
}

void CropToolPrivate::handle_cropbox_origin_changed( int index, double value,
                          Core::ActionSource source )
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }
  
  // NOTE: Lock the state engine because the following changes need to be atomic
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  const std::string& group_id = this->tool_->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  LayerGroupHandle layer_group = LayerManager::Instance()->find_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  Core::Transform inv_trans = grid_trans.get_inverse();
  Core::Point origin( 0, 0, 0 );
  origin[ index ] = value;
  Core::Point origin_index = inv_trans * origin;
  int crop_index = Core::Round( origin_index[ index ] );

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  double min_val, max_val;
  this->tool_->cropbox_origin_state_[ index ]->get_range( min_val, max_val );
  double min_size, max_size;
  this->tool_->cropbox_size_state_[ index ]->get_range( min_size, max_size );
  this->tool_->cropbox_size_state_[ index ]->set_range( 0, max_val - value );
  if ( source == Core::ActionSource::INTERFACE_MOUSE_E )
  {
    double size = this->tool_->cropbox_size_state_[ index ]->get();
    this->tool_->cropbox_size_state_[ index ]->set( size + max_val - value - max_size );  
  }

  this->tool_->cropbox_origin_index_state_[ index ]->set( crop_index );
  this->tool_->cropbox_size_index_state_[ index ]->set_range( 1,
    this->tool_->input_dimensions_state_[ index ]->get() -
    this->tool_->cropbox_origin_index_state_[ index ]->get() );
  Core::Point end( 0, 0, 0 );
  end[ index ] = value + this->tool_->cropbox_size_state_[ index ]->get();
  end = inv_trans * end;
  this->tool_->cropbox_size_index_state_[ index ]->set( Core::Round( end[ index ] ) - crop_index + 1 );
}

void CropToolPrivate::handle_cropbox_size_changed( int index, double value )
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }
  
  const std::string& group_id = this->tool_->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  LayerGroupHandle layer_group = LayerManager::Instance()->find_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  Core::Transform inv_trans = grid_trans.get_inverse();
  Core::Point end( 0, 0, 0 );
  end[ index ] = this->tool_->cropbox_origin_state_[ index ]->get() + value;
  end = inv_trans * end;

  Core::ScopedCounter signal_block( this->signal_block_count_ );
  this->tool_->cropbox_size_index_state_[ index ]->set( Core::Round( end[ index ] ) - 
    this->tool_->cropbox_origin_index_state_[ index ]->get() + 1 );
}

void CropToolPrivate::handle_cropbox_origin_index_changed( int index, int value )
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }
  
  const std::string& group_id = this->tool_->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  LayerGroupHandle layer_group = LayerManager::Instance()->find_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  Core::Point origin( 0, 0, 0 );
  origin[ index ] = value;
  origin = grid_trans * origin;

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  this->tool_->cropbox_origin_state_[ index ]->set( origin[ index ] );
  this->tool_->cropbox_size_index_state_[ index ]->set_range( 1,
    this->tool_->input_dimensions_state_[ index ]->get() - value );

  Core::Point end( 0, 0, 0 );
  end[ index ] = this->tool_->input_dimensions_state_[ index ]->get() - 1;
  end = grid_trans * end;
  this->tool_->cropbox_size_state_[ index ]->set_range( 0, end[ index ] - origin[ index ] );
}

void CropToolPrivate::handle_cropbox_size_index_changed( int index, int value )
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }

  const std::string& group_id = this->tool_->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  LayerGroupHandle layer_group = LayerManager::Instance()->find_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  Core::Point end( 0, 0, 0 );
  end[ index ] = this->tool_->cropbox_origin_index_state_[ index ]->get() + value - 1;
  end = grid_trans * end;
  Core::ScopedCounter signal_block( this->signal_block_count_ );
  this->tool_->cropbox_size_state_[ index ]->set( end[ index ] -
    this->tool_->cropbox_origin_state_[ index ]->get() );
}

void CropToolPrivate::hit_test( ViewerHandle viewer, int x, int y )
{
  this->hit_pos_ = Core::HitPosition::NONE_E;

  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( viewer->is_volume_view() ||
    this->tool_->target_group_state_->get() == "" )
  {
    return;
  }
  
  // Compute the size of a pixel in world space
  double x0, y0, x1, y1;
  viewer->window_to_world( 0, 0, x0, y0 );
  viewer->window_to_world( 1, 1, x1, y1 );
  double pixel_width = Core::Abs( x1 - x0 );
  double pixel_height = Core::Abs( y1 - y0 );

  // Compute the mouse position in world space
  double world_x, world_y;
  viewer->window_to_world( x, y, world_x, world_y );

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  this->hor_index_ = 0;
  this->ver_index_ = 1;
  if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
    this->hor_index_ = 0;
    this->ver_index_ = 2;
  }
  else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
  {
    slice_type = Core::VolumeSliceType::SAGITTAL_E;
    this->hor_index_ = 1;
    this->ver_index_ = 2;
  }

  // Compute the boundary of the crop box in 2D space
  double left, right, bottom, top;
  Core::Point origin( this->tool_->cropbox_origin_state_[ 0 ]->get(),
    this->tool_->cropbox_origin_state_[ 1 ]->get(), 
    this->tool_->cropbox_origin_state_[ 2 ]->get() );
  Core::Vector size( this->tool_->cropbox_size_state_[ 0 ]->get(),
    this->tool_->cropbox_size_state_[ 1 ]->get(),
    this->tool_->cropbox_size_state_[ 2 ]->get() );
  Core::Point end = origin + size;
  Core::VolumeSlice::ProjectOntoSlice( slice_type, origin, left, bottom );
  Core::VolumeSlice::ProjectOntoSlice( slice_type, end, right, top );

  // Test where the mouse hits the crop box boundary
  // The threshold is 2 pixels.
  double range_x = pixel_width * 2;
  double range_y = pixel_height * 2;

  // Test left boundary
  if ( Core::Abs( world_x - left ) <= range_x &&
    world_y + range_y >= bottom && 
    world_y - range_y <= top )
  {
    this->hit_pos_ |= Core::HitPosition::LEFT_E;
  }

  // Test bottom boundary
  if ( Core::Abs( world_y - bottom ) <= range_y && 
    world_x + range_x >= left && 
    world_x - range_x <= right )
  {
    this->hit_pos_ |= Core::HitPosition::BOTTOM_E;
  }

  // If the mouse hits a corner, no further test is needed
  if ( this->hit_pos_ == Core::HitPosition::BOTTOM_LEFT_E )
  {
    return;
  }
  
  // Test top boundary
  if ( !( this->hit_pos_ & Core::HitPosition::BOTTOM_E ) &&
    Core::Abs( world_y - top ) <= range_y &&
    world_x + range_x >= left && 
    world_x - range_x <= right )
  {
    this->hit_pos_ |= Core::HitPosition::TOP_E;
  }
  
  if ( this->hit_pos_ == Core::HitPosition::TOP_LEFT_E )
  {
    return;
  }
  
  // Test right boundary
  if ( !( this->hit_pos_ & Core::HitPosition::LEFT_E ) &&
    Core::Abs( world_x - right ) <= range_x &&
    world_y + range_y >= bottom && 
    world_y - range_y <= top )
  {
    this->hit_pos_ |= Core::HitPosition::RIGHT_E;
  }

  if ( this->hit_pos_ == Core::HitPosition::NONE_E &&
    world_x > left && world_x < right &&
    world_y > bottom && world_y < top )
  {
    this->hit_pos_ = Core::HitPosition::INSIDE_E;
  }
}

void CropToolPrivate::resize( ViewerHandle viewer, int x0, int y0, int x1, int y1 )
{
  double world_x0, world_y0, world_x1, world_y1;
  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    viewer->window_to_world( x0, y0, world_x0, world_y0 );
    viewer->window_to_world( x1, y1, world_x1, world_y1 );
  }
  double dx = world_x1 - world_x0;
  double dy = world_y1 - world_y0;
  bool size_changed = false;
  const double epsilon = 1e-6;

  if ( ( this->hit_pos_ & Core::HitPosition::LEFT_E ) && Core::Abs( dx ) > epsilon )
  {
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      this->tool_->cropbox_origin_state_[ this->hor_index_ ], dx );
    size_changed = true;
  }
  
  if ( ( this->hit_pos_ & Core::HitPosition::BOTTOM_E ) && Core::Abs( dy ) > epsilon )
  {
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      this->tool_->cropbox_origin_state_[ this->ver_index_ ], dy );
    size_changed = true;
  }
  
  if ( ( this->hit_pos_ & Core::HitPosition::RIGHT_E ) && Core::Abs( dx ) > epsilon )
  {
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      this->tool_->cropbox_size_state_[ this->hor_index_ ], dx );
    size_changed = true;
  }
  
  if ( ( this->hit_pos_ & Core::HitPosition::TOP_E ) && Core::Abs( dy ) > epsilon )
  {
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      this->tool_->cropbox_size_state_[ this->ver_index_ ], dy );
    size_changed = true;
  }

  if ( this->hit_pos_ == Core::HitPosition::INSIDE_E )
  {
    // NOTE: Here it uses keyboard action context instead of the actual mouse action
    // context so that the change of origin won't cause the size to change
    if ( Core::Abs( dx ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetKeyboardActionContext(),
        this->tool_->cropbox_origin_state_[ this->hor_index_ ], dx );
      size_changed = true;
    }
    if ( Core::Abs( dy ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetKeyboardActionContext(),
        this->tool_->cropbox_origin_state_[ this->ver_index_ ], dy );
      size_changed = true;
    }
  }

  if ( size_changed )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  }
}

void CropToolPrivate::update_cursor( ViewerHandle viewer )
{
  switch ( this->hit_pos_ )
  {
  case Core::HitPosition::INSIDE_E:
    viewer->set_cursor( Core::CursorShape::SIZE_ALL_E );
    break;
  case Core::HitPosition::TOP_LEFT_E:
  case Core::HitPosition::BOTTOM_RIGHT_E:
    viewer->set_cursor( Core::CursorShape::SIZE_FDIAG_E );
    break;
  case Core::HitPosition::BOTTOM_LEFT_E:
  case Core::HitPosition::TOP_RIGHT_E:
    viewer->set_cursor( Core::CursorShape::SIZE_BDIAG_E );
    break;
  case Core::HitPosition::LEFT_E:
  case Core::HitPosition::RIGHT_E:
    viewer->set_cursor( Core::CursorShape::SIZE_HOR_E );
    break;
  case Core::HitPosition::BOTTOM_E:
  case Core::HitPosition::TOP_E:
    viewer->set_cursor( Core::CursorShape::SIZE_VER_E );
    break;
  default:
    viewer->set_cursor( Core::CursorShape::CROSS_E );
    break;
  }
}

//////////////////////////////////////////////////////////////////////////
// Class CropTool
//////////////////////////////////////////////////////////////////////////

CropTool::CropTool( const std::string& toolid ) :
  GroupTargetTool( Core::VolumeType::ALL_E, toolid ),
  private_( new CropToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->signal_block_count_ = 0;
  this->private_->resizing_ = false;

  double inf = std::numeric_limits< double >::infinity();
  int max_int = std::numeric_limits< int >::max();
  this->add_state( "input_x", this->input_dimensions_state_[ 0 ], 0 );
  this->add_state( "input_y", this->input_dimensions_state_[ 1 ], 0 );
  this->add_state( "input_z", this->input_dimensions_state_[ 2 ], 0 );
  this->add_state( "crop_origin_x", this->cropbox_origin_state_[ 0 ], -inf, -inf, inf, 0.01 );
  this->add_state( "crop_origin_y", this->cropbox_origin_state_[ 1 ], -inf, -inf, inf, 0.01 );
  this->add_state( "crop_origin_z", this->cropbox_origin_state_[ 2 ], -inf, -inf, inf, 0.01 );
  this->add_state( "crop_width", this->cropbox_size_state_[ 0 ], inf, 0.0, inf, 0.01 );
  this->add_state( "crop_height", this->cropbox_size_state_[ 1 ], inf, 0.0, inf, 0.01 );
  this->add_state( "crop_depth", this->cropbox_size_state_[ 2 ], inf, 0.0, inf, 0.01 );
  this->add_state( "crop_origin_index_x", this->cropbox_origin_index_state_[ 0 ], 0, 0, max_int, 1 );
  this->add_state( "crop_origin_index_y", this->cropbox_origin_index_state_[ 1 ], 0, 0, max_int, 1 );
  this->add_state( "crop_origin_index_z", this->cropbox_origin_index_state_[ 2 ], 0, 0, max_int, 1 );
  this->add_state( "crop_width_index", this->cropbox_size_index_state_[ 0 ], max_int, 1, max_int, 1 );
  this->add_state( "crop_height_index", this->cropbox_size_index_state_[ 1 ], max_int, 1, max_int, 1 );
  this->add_state( "crop_depth_index", this->cropbox_size_index_state_[ 2 ], max_int, 1, max_int, 1 );
  this->add_state( "crop_in_index", this->crop_in_index_space_state_, true );

  this->add_state( "replace", this->replace_state_, false );

  this->add_connection( this->target_group_state_->state_changed_signal_.connect(
    boost::bind( &CropToolPrivate::handle_target_group_changed, this->private_ ) ) );

  for ( int i = 0; i < 3; ++i )
  {
    this->add_connection( this->cropbox_origin_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_origin_changed, this->private_, i, _1, _2 ) ) );
    this->add_connection( this->cropbox_origin_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_changed, this->private_, _2 ) ) );
    this->add_connection( this->cropbox_size_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_size_changed, this->private_, i, _1 ) ) );
    this->add_connection( this->cropbox_size_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_changed, this->private_, _2 ) ) );
    this->add_connection( this->cropbox_origin_index_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_origin_index_changed, this->private_, i, _1 ) ) );
    this->add_connection( this->cropbox_origin_index_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_changed, this->private_, _2 ) ) );
    this->add_connection( this->cropbox_size_index_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_size_index_changed, this->private_, i, _1 ) ) );
    this->add_connection( this->cropbox_size_index_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_changed, this->private_, _2 ) ) );
  }
  
  this->private_->handle_target_group_changed();
}

CropTool::~CropTool()
{
  this->disconnect_all();
}

void CropTool::execute( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  Core::Point origin( this->cropbox_origin_state_[ 0 ]->get(),
    this->cropbox_origin_state_[ 1 ]->get(), this->cropbox_origin_state_[ 2 ]->get() );
  Core::Vector size( this->cropbox_size_state_[ 0 ]->get(),
    this->cropbox_size_state_[ 1 ]->get(), this->cropbox_size_state_[ 2 ]->get() );
  // Reverse the order of target layers
  std::vector< std::string > target_layers = this->target_layers_state_->get();
  std::reverse( target_layers.begin(), target_layers.end() );

  ActionCrop::Dispatch( context, target_layers, origin, size, this->replace_state_->get() );
}

void CropTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat,
  int viewer_width, int viewer_height )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  Core::Point crop_origin, crop_end;
  double depth;

  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    std::string target_group_id = this->target_group_state_->get();
    if ( viewer->is_volume_view() || 
      target_group_id == "" || 
      target_group_id == Tool::NONE_OPTION_C )
    {
      return;
    }

    Core::StateView2D* state_view2d = static_cast< Core::StateView2D* >( 
      viewer->get_active_view_state().get() );
    depth = state_view2d->get().center().z();

    if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      slice_type = Core::VolumeSliceType::SAGITTAL_E;
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      slice_type = Core::VolumeSliceType::CORONAL_E;
    }

    for ( int i = 0; i < 3; ++i )
    {
      crop_origin[ i ] = this->cropbox_origin_state_[ i ]->get();
      crop_end[ i ] = crop_origin[ i ] + this->cropbox_size_state_[ i ]->get();
    }
  }

  double start_x, start_y, end_x, end_y, depth_min, depth_max;
  Core::VolumeSlice::ProjectOntoSlice( slice_type, crop_origin, start_x, start_y, depth_min );
  Core::VolumeSlice::ProjectOntoSlice( slice_type, crop_end, end_x, end_y, depth_max );
  bool in_range = depth >= depth_min && depth <= depth_max;

  glPushAttrib( GL_LINE_BIT | GL_TRANSFORM_BIT );
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glLineWidth( 1.0f );
  glColor4f( 1.0f, 0.0f, 0.0f, in_range ? 1.0f : 0.5f );

  glBegin( GL_LINE_LOOP );
  glVertex2d( start_x, start_y );
  glVertex2d( end_x, start_y );
  glVertex2d( end_x, end_y );
  glVertex2d( start_x, end_y );
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

bool CropTool::has_2d_visual()
{
  return true;
}

bool CropTool::handle_mouse_leave( ViewerHandle viewer )
{
  this->private_->resizing_ = false;
  return true;
}

bool CropTool::handle_mouse_move( ViewerHandle viewer, 
                 const Core::MouseHistory& mouse_history, 
                 int button, int buttons, int modifiers )
{ 
  if ( viewer->is_volume_view() )
  {
    this->private_->hit_pos_ = Core::HitPosition::NONE_E;
    return false;
  }
  
  if ( buttons == Core::MouseButton::NO_BUTTON_E &&
    modifiers == Core::KeyModifier::NO_MODIFIER_E )
  {
    this->private_->hit_test( viewer, mouse_history.current_.x_, mouse_history.current_.y_ );
    this->private_->update_cursor( viewer );
    return true;
  }
  
  if ( this->private_->resizing_ )
  {
    this->private_->resize( viewer, mouse_history.previous_.x_, mouse_history.previous_.y_,
      mouse_history.current_.x_, mouse_history.current_.y_ );
    return true;
  }

  return false;
}

bool CropTool::handle_mouse_press( ViewerHandle viewer, 
                  const Core::MouseHistory& mouse_history, 
                  int button, int buttons, int modifiers )
{
  if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    button == Core::MouseButton::LEFT_BUTTON_E &&
    this->private_->hit_pos_ != Core::HitPosition::NONE_E )
  {
    this->private_->resizing_ = true;
    return true;
  }
  
  return false;
}

bool CropTool::handle_mouse_release( ViewerHandle viewer, 
                  const Core::MouseHistory& mouse_history, 
                  int button, int buttons, int modifiers )
{
  if ( button == Core::MouseButton::LEFT_BUTTON_E &&
    this->private_->resizing_ )
  {
    this->private_->resizing_ = false;
    this->private_->hit_test( viewer, mouse_history.current_.x_, mouse_history.current_.y_ );
    this->private_->update_cursor( viewer );
    return true;
  }
  
  return false;
}

void CropTool::deactivate()
{
  ViewerManager::Instance()->reset_cursor();
}

void CropTool::reset()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &CropTool::reset, this ) );
    return;
  }
  
  const std::string& group_id = this->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  // NOTE: Lock the state engine because the following changes need to be atomic
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  Core::ScopedCounter signal_block( this->private_->signal_block_count_ );

  for ( int i = 0; i < 3; ++i )
  {
    double min_val, max_val;
    this->cropbox_origin_state_[ i ]->get_range( min_val, max_val );
    this->cropbox_origin_state_[ i ]->set( min_val );
    this->cropbox_size_state_[ i ]->set_range( 0, max_val - min_val );
    this->cropbox_size_state_[ i ]->set( max_val - min_val );
    this->cropbox_origin_index_state_[ i ]->set( 0 );
    this->cropbox_size_index_state_[ i ]->set_range( 1, this->input_dimensions_state_[ i ]->get() );
    this->cropbox_size_index_state_[ i ]->set( this->input_dimensions_state_[ i ]->get() );
  }

  state_lock.unlock();
  ViewerManager::Instance()->update_2d_viewers_overlay();
}


} // end namespace Seg3D
