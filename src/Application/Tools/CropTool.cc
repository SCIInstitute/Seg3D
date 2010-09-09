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

#include <Core/State/Actions/ActionOffset.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/RenderResources/RenderResources.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/CropTool.h>
#include <Application/Tools/Actions/ActionCrop.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/ViewerManager/ViewerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, CropTool )

namespace Seg3D
{

CORE_ENUM_CLASS
(
  HitPosition,
  NONE_E = 0x00,
  LEFT_E = 0x01,
  RIGHT_E = 0x02,
  BOTTOM_E = 0x04,
  TOP_E = 0x08,
  INSIDE_E = 0x10,
  BOTTOM_LEFT_E = LEFT_E | BOTTOM_E,
  BOTTOM_RIGHT_E = RIGHT_E | BOTTOM_E,
  TOP_LEFT_E = LEFT_E | TOP_E,
  TOP_RIGHT_E = RIGHT_E | TOP_E
)

//////////////////////////////////////////////////////////////////////////
// Class CropToolPrivate
//////////////////////////////////////////////////////////////////////////

class CropToolPrivate
{
public:
  CropToolPrivate() : hit_pos_( HitPosition::NONE_E ) {}

  void handle_target_group_changed();
  void handle_cropbox_origin_changed( int index, double value, Core::ActionSource source );
  void handle_cropbox_changed( Core::ActionSource source );

  void hit_test( int x, int y );
  void update_cursor();
  void resize( int x0, int y0, int x1, int y1 );

  size_t signal_block_count_;
  CropTool* tool_;
  ViewerHandle viewer_;
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

  LayerGroupHandle layer_group = LayerManager::Instance()->get_layer_group( group_id );
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

  state_lock.unlock();
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void CropToolPrivate::handle_cropbox_changed(  Core::ActionSource source )
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

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  double min_val, max_val;
  this->tool_->cropbox_origin_state_[ index ]->get_range( min_val, max_val );
  double min_size, max_size, size;
  this->tool_->cropbox_size_state_[ index ]->get_range( min_size, max_size );
  size = this->tool_->cropbox_size_state_[ index ]->get();
  this->tool_->cropbox_size_state_[ index ]->set_range( 0, max_val - value );
  if ( source != Core::ActionSource::INTERFACE_MOUSE_E )
  {
    this->tool_->cropbox_size_state_[ index ]->set( size + max_val - value - max_size );  
  }
}

void CropToolPrivate::hit_test( int x, int y )
{
  this->hit_pos_ = HitPosition::NONE_E;

  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( this->viewer_->is_volume_view() ||
    this->tool_->target_group_state_->get() == "" )
  {
    return;
  }
  
  // Compute the size of a pixel in world space
  double x0, y0, x1, y1;
  this->viewer_->window_to_world( 0, 0, x0, y0 );
  this->viewer_->window_to_world( 1, 1, x1, y1 );
  double pixel_width = Core::Abs( x1 - x0 );
  double pixel_height = Core::Abs( y1 - y0 );

  // Compute the mouse position in world space
  double world_x, world_y;
  this->viewer_->window_to_world( x, y, world_x, world_y );

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  this->hor_index_ = 0;
  this->ver_index_ = 1;
  if ( this->viewer_->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
    this->hor_index_ = 0;
    this->ver_index_ = 2;
  }
  else if ( this->viewer_->view_mode_state_->get() == Viewer::SAGITTAL_C )
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
    this->hit_pos_ |= HitPosition::LEFT_E;
  }

  // Test bottom boundary
  if ( Core::Abs( world_y - bottom ) <= range_y && 
    world_x + range_x >= left && 
    world_x - range_x <= right )
  {
    this->hit_pos_ |= HitPosition::BOTTOM_E;
  }

  // If the mouse hits a corner, no further test is needed
  if ( this->hit_pos_ == HitPosition::BOTTOM_LEFT_E )
  {
    return;
  }
  
  // Test top boundary
  if ( !( this->hit_pos_ & HitPosition::BOTTOM_E ) &&
    Core::Abs( world_y - top ) <= range_y &&
    world_x + range_x >= left && 
    world_x - range_x <= right )
  {
    this->hit_pos_ |= HitPosition::TOP_E;
  }
  
  if ( this->hit_pos_ == HitPosition::TOP_LEFT_E )
  {
    return;
  }
  
  // Test right boundary
  if ( !( this->hit_pos_ & HitPosition::LEFT_E ) &&
    Core::Abs( world_x - right ) <= range_x &&
    world_y + range_y >= bottom && 
    world_y - range_y <= top )
  {
    this->hit_pos_ |= HitPosition::RIGHT_E;
  }

  if ( this->hit_pos_ == HitPosition::NONE_E &&
    world_x > left && world_x < right &&
    world_y > bottom && world_y < top )
  {
    this->hit_pos_ = HitPosition::INSIDE_E;
  }
}

void CropToolPrivate::resize( int x0, int y0, int x1, int y1 )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  double world_x0, world_y0, world_x1, world_y1;
  this->viewer_->window_to_world( x0, y0, world_x0, world_y0 );
  this->viewer_->window_to_world( x1, y1, world_x1, world_y1 );
  double dx = world_x1 - world_x0;
  double dy = world_y1 - world_y0;
  bool size_changed = false;
  const double epsilon = 1e-6;

  if ( this->hit_pos_ & HitPosition::LEFT_E )
  {
    double min_x, max_x;
    double old_origin_hor = this->tool_->cropbox_origin_state_[ this->hor_index_ ]->get();
    this->tool_->cropbox_origin_state_[ this->hor_index_ ]->get_range( min_x, max_x );
    double origin_hor = old_origin_hor + dx;
    origin_hor = Core::Clamp( origin_hor, min_x, max_x );
    double offset = origin_hor - old_origin_hor;
    if ( Core::Abs( offset ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_origin_state_[ this->hor_index_ ], offset );
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_size_state_[ this->hor_index_ ], -offset );
      size_changed = true;
    }
  }
  
  if ( this->hit_pos_ & HitPosition::BOTTOM_E )
  {
    double min_y, max_y;
    double old_origin_ver = this->tool_->cropbox_origin_state_[ this->ver_index_ ]->get();
    this->tool_->cropbox_origin_state_[ this->ver_index_ ]->get_range( min_y, max_y );
    double origin_ver = old_origin_ver + dy;
    origin_ver = Core::Clamp( origin_ver, min_y, max_y );
    double offset = origin_ver - old_origin_ver;
    if ( Core::Abs( offset ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_origin_state_[ this->ver_index_ ], offset );
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_size_state_[ this->ver_index_ ], -offset );
      size_changed = true;
    }
  }
  
  if ( this->hit_pos_ & HitPosition::RIGHT_E )
  {
    double min_size, max_size, old_size;
    this->tool_->cropbox_size_state_[ this->hor_index_ ]->get_range( min_size, max_size );
    old_size = this->tool_->cropbox_size_state_[ this->hor_index_ ]->get();
    double size = old_size + dx;
    size = Core::Clamp( size, min_size, max_size );
    double offset = size - old_size;
    if ( Core::Abs( offset ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_size_state_[ this->hor_index_ ], offset );
      size_changed = true;
    }
  }
  
  if ( this->hit_pos_ & HitPosition::TOP_E )
  {
    double min_size, max_size, old_size;
    this->tool_->cropbox_size_state_[ this->ver_index_ ]->get_range( min_size, max_size );
    old_size = this->tool_->cropbox_size_state_[ this->ver_index_ ]->get();
    double size = old_size + dy;
    size = Core::Clamp( size, min_size, max_size );
    double offset = size - old_size;
    if ( Core::Abs( offset ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_size_state_[ this->ver_index_ ], offset );
      size_changed = true;
    }
  }

  if ( this->hit_pos_ == HitPosition::INSIDE_E )
  {
    double size_ver = this->tool_->cropbox_size_state_[ this->ver_index_ ]->get();
    double size_hor = this->tool_->cropbox_size_state_[ this->hor_index_ ]->get();
    double min_ver, max_ver, min_hor, max_hor;
    this->tool_->cropbox_origin_state_[ this->ver_index_ ]->get_range( min_ver, max_ver );
    this->tool_->cropbox_origin_state_[ this->hor_index_ ]->get_range( min_hor, max_hor );
    double old_origin_ver = this->tool_->cropbox_origin_state_[ this->ver_index_ ]->get();
    double old_origin_hor = this->tool_->cropbox_origin_state_[ this->hor_index_ ]->get();
    double origin_ver = Core::Clamp( old_origin_ver + dy, min_ver, max_ver - size_ver );
    double origin_hor = Core::Clamp( old_origin_hor + dx, min_hor, max_hor - size_hor );
    double hor_offset = origin_hor - old_origin_hor;
    double ver_offset = origin_ver - old_origin_ver;
    if ( Core::Abs( hor_offset ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_origin_state_[ this->hor_index_ ], hor_offset );
      size_changed = true;
    }
    if ( Core::Abs( ver_offset ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->cropbox_origin_state_[ this->ver_index_ ], ver_offset );
      size_changed = true;
    } 
  }

  state_lock.unlock();
  if ( size_changed )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  }
}

void CropToolPrivate::update_cursor()
{
  switch ( this->hit_pos_ )
  {
  case HitPosition::INSIDE_E:
    this->viewer_->set_cursor( Core::CursorShape::SIZE_ALL_E );
    break;
  case HitPosition::TOP_LEFT_E:
  case HitPosition::BOTTOM_RIGHT_E:
    this->viewer_->set_cursor( Core::CursorShape::SIZE_FDIAG_E );
    break;
  case HitPosition::BOTTOM_LEFT_E:
  case HitPosition::TOP_RIGHT_E:
    this->viewer_->set_cursor( Core::CursorShape::SIZE_BDIAG_E );
    break;
  case HitPosition::LEFT_E:
  case HitPosition::RIGHT_E:
    this->viewer_->set_cursor( Core::CursorShape::SIZE_HOR_E );
    break;
  case HitPosition::BOTTOM_E:
  case HitPosition::TOP_E:
    this->viewer_->set_cursor( Core::CursorShape::SIZE_VER_E );
    break;
  default:
    this->viewer_->set_cursor( Core::CursorShape::ARROW_E );
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

  this->add_state( "input_x", this->input_dimensions_state_[ 0 ], 0 );
  this->add_state( "input_y", this->input_dimensions_state_[ 1 ], 0 );
  this->add_state( "input_z", this->input_dimensions_state_[ 2 ], 0 );
  this->add_state( "crop_origin_x", this->cropbox_origin_state_[ 0 ], -1000.0, -1000.0, 1000.0, 0.01 );
  this->add_state( "crop_origin_y", this->cropbox_origin_state_[ 1 ], -1000.0, -1000.0, 1000.0, 0.01 );
  this->add_state( "crop_origin_z", this->cropbox_origin_state_[ 2 ], -1000.0, -1000.0, 1000.0, 0.01 );
  this->add_state( "crop_width", this->cropbox_size_state_[ 0 ], 1000.0, 0.0, 1000.0, 0.01 );
  this->add_state( "crop_height", this->cropbox_size_state_[ 1 ], 1000.0, 0.0, 1000.0, 0.01 );
  this->add_state( "crop_depth", this->cropbox_size_state_[ 2 ], 1000.0, 0.0, 1000.0, 0.01 );

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
  ActionCrop::Dispatch( context, this->target_layers_state_->get(), 
    origin, size, this->replace_state_->get() );
}

void CropTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  Core::Point crop_origin, crop_end;

  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    std::string target_group_id = this->target_group_state_->get();
    if ( viewer->is_volume_view() || 
      target_group_id == "" || 
      target_group_id == Tool::NONE_OPTION_C )
    {
      return;
    }
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

  double start_x, start_y, end_x, end_y;
  Core::VolumeSlice::ProjectOntoSlice( slice_type, crop_origin, start_x, start_y );
  Core::VolumeSlice::ProjectOntoSlice( slice_type, crop_end, end_x, end_y );

  glPushAttrib( GL_LINE_BIT | GL_POINT_BIT | GL_TRANSFORM_BIT );
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glLineWidth( 2.0f );
  glPointSize( 2.0f );
  glColor3f( 1.0f, 0.0f, 0.0f );

  glBegin( GL_LINE_LOOP );
  glVertex2d( start_x, start_y );
  glVertex2d( end_x, start_y );
  glVertex2d( end_x, end_y );
  glVertex2d( start_x, end_y );
  glEnd();

  glBegin( GL_POINTS );
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

bool CropTool::handle_mouse_enter( size_t viewer_id, int x, int y )
{
  this->private_->viewer_ = ViewerManager::Instance()->get_viewer( viewer_id );
  return true;
}

bool CropTool::handle_mouse_leave( size_t viewer_id )
{
  this->private_->resizing_ = false;
  this->private_->viewer_.reset();
  return true;
}

bool CropTool::handle_mouse_move( const Core::MouseHistory& mouse_history, 
                 int button, int buttons, int modifiers )
{ 
  if ( buttons == Core::MouseButton::NO_BUTTON_E &&
    modifiers == Core::KeyModifier::NO_MODIFIER_E )
  {
    this->private_->hit_test( mouse_history.current_.x_, mouse_history.current_.y_ );
    this->private_->update_cursor();
    return true;
  }
  
  if ( this->private_->resizing_ )
  {
    this->private_->resize( mouse_history.previous_.x_, mouse_history.previous_.y_,
      mouse_history.current_.x_, mouse_history.current_.y_ );
    return true;
  }

  return false;
}

bool CropTool::handle_mouse_press( const Core::MouseHistory& mouse_history, 
                  int button, int buttons, int modifiers )
{
  if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    button == Core::MouseButton::LEFT_BUTTON_E &&
    this->private_->hit_pos_ != HitPosition::NONE_E )
  {
    this->private_->resizing_ = true;
    return true;
  }
  
  return false;
}

bool CropTool::handle_mouse_release( const Core::MouseHistory& mouse_history, 
                  int button, int buttons, int modifiers )
{
  if ( button == Core::MouseButton::LEFT_BUTTON_E &&
    this->private_->resizing_ )
  {
    this->private_->resizing_ = false;
    this->private_->hit_test( mouse_history.current_.x_, mouse_history.current_.y_ );
    this->private_->update_cursor();
    return true;
  }
  
  return false;
}

} // end namespace Seg3D
