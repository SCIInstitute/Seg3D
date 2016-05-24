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

// boost includes
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <Core/State/Actions/ActionOffset.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/RenderResources/RenderResources.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/TransformTool.h>
#include <Application/Tools/detail/MaskShader.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Filters/Actions/ActionTransform.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, TransformTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class TransformToolPrivate
//////////////////////////////////////////////////////////////////////////

class TransformToolPrivate : public Core::Lockable
{
public:

  void handle_target_group_changed( std::string group_id );
  void handle_target_layers_changed();

  void handle_preview_layer_changed();
  void handle_origin_changed();
  void handle_spacing_changed( int index );
  void handle_keep_aspect_ratio_changed( bool keep );
  void handle_show_border_changed( bool show );
  void handle_show_preview_changed( bool show );

  void hit_test( ViewerHandle viewer, int x, int y );
  void update_cursor( ViewerHandle viewer );
  void resize( ViewerHandle viewer, int x0, int y0, int x1, int y1 );

  void initialize_gl();
  void update_grid_transform();

  size_t signal_block_count_;

  Core::GridTransform grid_transform_;
  Core::Vector src_spacing_;
  int size_[ 3 ];
  Core::Point world_min_, world_max_;
  
  MaskShaderHandle shader_;
  TransformTool* tool_;

  int hit_pos_;
  int ver_index_; // Index of the vertical position state
  int hor_index_; // Index of the horizontal position state
  bool resizing_;
};

void TransformToolPrivate::handle_target_group_changed( std::string group_id )
{
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  LayerGroupHandle layer_group = LayerManager::Instance()->find_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  Core::Point origin = grid_trans * Core::Point( 0, 0, 0 );
  this->src_spacing_ = grid_trans * Core::Vector( 1, 1, 1 );

  {
    Core::ScopedCounter signal_block( this->signal_block_count_ );
    for ( int i = 0; i < 3; ++i )
    {
      this->tool_->origin_state_[ i ]->set( origin[ i ] );
      this->tool_->spacing_state_[ i ]->set( this->src_spacing_[ i ] );
    }
  }

  {
    TransformToolPrivate::lock_type private_lock( this->get_mutex() );
    this->grid_transform_ = grid_trans;
    this->size_[ 0 ] = static_cast< int >( grid_trans.get_nx() );
    this->size_[ 1 ] = static_cast< int >( grid_trans.get_ny() );
    this->size_[ 2 ] = static_cast< int >( grid_trans.get_nz() );
    this->world_min_ = this->grid_transform_ * Core::Point( -0.5, -0.5, -0.5 );
    this->world_max_ = this->grid_transform_ * Core::Point( this->size_[ 0 ] - 0.5, 
      this->size_[ 1 ] - 0.5, this->size_[ 2 ] - 0.5 );
  }
}

void TransformToolPrivate::handle_target_layers_changed()
{
  const std::vector< std::string >& target_layers = this->tool_->target_layers_state_->get();
  const std::vector< Core::OptionLabelPair >& option_list = this->tool_->
    target_layers_state_->get_option_list();
  std::vector< Core::OptionLabelPair > preview_layers_list;
  for ( size_t i = 0; i < target_layers.size(); ++i )
  {
    std::string layer_id = target_layers[ i ];
    for ( size_t j = 0; j < option_list.size(); j++ )
    {
      if ( option_list[ j ].first == layer_id )
      {
        preview_layers_list.push_back( option_list[ j ] );
        break;
      }
    }
  }
  
  this->tool_->preview_layer_state_->set_option_list(   preview_layers_list );
}

void TransformToolPrivate::handle_preview_layer_changed()
{
  if ( this->signal_block_count_ == 0 )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  } 
}

void TransformToolPrivate::handle_origin_changed()
{
  if ( this->signal_block_count_ == 0 &&
    this->tool_->preview_layer_state_->get() != "" )
  {
    this->update_grid_transform();
    ViewerManager::Instance()->update_2d_viewers_overlay();
  } 
}

void TransformToolPrivate::handle_spacing_changed( int index )
{
  if ( this->signal_block_count_ > 0 ||
    this->tool_->target_group_state_->get() == "" )
  {
    return;
  }
  
  if ( this->tool_->keep_aspect_ratio_state_->get() )
  {
    Core::ScopedCounter signal_block( this->signal_block_count_ );
    double scale = this->tool_->spacing_state_[ index ]->get() / this->src_spacing_[ index ];
    this->tool_->spacing_state_[ ( index + 1 ) % 3 ]->set( 
      this->src_spacing_[ ( index + 1 ) % 3 ] * scale );
    this->tool_->spacing_state_[ ( index + 2 ) % 3 ]->set( 
      this->src_spacing_[ ( index + 2 ) % 3 ] * scale );
  }
  
  this->update_grid_transform();
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void TransformToolPrivate::handle_keep_aspect_ratio_changed( bool keep )
{
  if ( keep )
  {
    this->handle_spacing_changed( 0 );
  }
}

void TransformToolPrivate::handle_show_border_changed( bool show )
{
  if ( this->tool_->target_group_state_->get() != "" )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  }
}

void TransformToolPrivate::handle_show_preview_changed( bool show )
{
  if ( this->tool_->preview_layer_state_->get() != "" )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  }
}

void TransformToolPrivate::initialize_gl()
{
  lock_type lock( this->get_mutex() );
  if ( this->shader_ )
  {
    return;
  }

  {
    Core::RenderResources::lock_type rr_lock( Core::RenderResources::GetMutex() );
    this->shader_.reset( new MaskShader );
    this->shader_->initialize();
  }
  
  this->shader_->enable();
  this->shader_->set_border_width( 0 );
  this->shader_->set_texture( 0 );
  this->shader_->set_opacity( 0.5f );
  this->shader_->disable();
}

void TransformToolPrivate::update_grid_transform()
{
  ASSERT_IS_APPLICATION_THREAD();

  lock_type lock( this->get_mutex() );
  Core::Point origin( this->tool_->origin_state_[ 0 ]->get(), 
    this->tool_->origin_state_[ 1 ]->get(),
    this->tool_->origin_state_[ 2 ]->get() );
  this->grid_transform_.load_basis( origin, 
    Core::Vector( this->tool_->spacing_state_[ 0 ]->get(), 0, 0 ),
    Core::Vector( 0, this->tool_->spacing_state_[ 1 ]->get(), 0 ),
    Core::Vector( 0, 0, this->tool_->spacing_state_[ 2 ]->get() ) );
  this->world_min_ = this->grid_transform_ * Core::Point( -0.5, -0.5, -0.5 );
  this->world_max_ = this->grid_transform_ * Core::Point( this->size_[ 0 ] - 0.5, 
    this->size_[ 1 ] - 0.5, this->size_[ 2 ] - 0.5 );
}

void TransformToolPrivate::hit_test( ViewerHandle viewer, int x, int y )
{
  this->hit_pos_ = Core::HitPosition::NONE_E;

  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( viewer->is_volume_view() ||
    this->tool_->target_group_state_->get() == "" ||
    ( !this->tool_->show_border_state_->get() && 
    !this->tool_->show_preview_state_->get() ) )
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
  Core::VolumeSlice::ProjectOntoSlice( slice_type, this->world_min_, left, bottom );
  Core::VolumeSlice::ProjectOntoSlice( slice_type, this->world_max_, right, top );

  // Test where the mouse hits the crop box boundary
  // The threshold is 2 pixels.
  double range_x = pixel_width * 2;
  double range_y = pixel_height * 2;

  // Test top boundary
  if ( Core::Abs( world_y - top ) <= range_y &&
    world_x + range_x >= left && 
    world_x - range_x <= right )
  {
    this->hit_pos_ |= Core::HitPosition::TOP_E;
    return;
  }

  // Test right boundary
  if ( Core::Abs( world_x - right ) <= range_x &&
    world_y + range_y >= bottom && 
    world_y - range_y <= top )
  {
    this->hit_pos_ |= Core::HitPosition::RIGHT_E;
    return;
  }

  if ( world_x > left && world_x < right &&
    world_y > bottom && world_y < top )
  {
    this->hit_pos_ = Core::HitPosition::INSIDE_E;
  }
}

void TransformToolPrivate::update_cursor( ViewerHandle viewer )
{
  switch ( this->hit_pos_ )
  {
  case Core::HitPosition::INSIDE_E:
    viewer->set_cursor( Core::CursorShape::SIZE_ALL_E );
    break;
  case Core::HitPosition::RIGHT_E:
    viewer->set_cursor( Core::CursorShape::SIZE_HOR_E );
    break;
  case Core::HitPosition::TOP_E:
    viewer->set_cursor( Core::CursorShape::SIZE_VER_E );
    break;
  default:
    viewer->set_cursor( Core::CursorShape::CROSS_E );
    break;
  }
}

void TransformToolPrivate::resize( ViewerHandle viewer, int x0, int y0, int x1, int y1 )
{
  double world_x0, world_y0, world_x1, world_y1;
  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    viewer->window_to_world( x0, y0, world_x0, world_y0 );
    viewer->window_to_world( x1, y1, world_x1, world_y1 );
  }
  double dx = world_x1 - world_x0;
  double dy = world_y1 - world_y0;
  const double epsilon = 1e-6;

  if ( ( this->hit_pos_ & Core::HitPosition::RIGHT_E ) && Core::Abs( dx ) > epsilon )
  {
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      this->tool_->spacing_state_[ this->hor_index_ ], dx / this->size_[ this->hor_index_ ] );
  }

  if ( ( this->hit_pos_ & Core::HitPosition::TOP_E ) && Core::Abs( dy ) > epsilon )
  {
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      this->tool_->spacing_state_[ this->ver_index_ ], dy / this->size_[ this->ver_index_ ] );
  }

  if ( this->hit_pos_ == Core::HitPosition::INSIDE_E )
  {
    if ( Core::Abs( dx ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->origin_state_[ this->hor_index_ ], dx );
    }
    if ( Core::Abs( dy ) > epsilon )
    {
      Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
        this->tool_->origin_state_[ this->ver_index_ ], dy );
    }
  }
}


//////////////////////////////////////////////////////////////////////////
// Class TransformTool
//////////////////////////////////////////////////////////////////////////

TransformTool::TransformTool( const std::string& toolid ) :
  GroupTargetTool( Core::VolumeType::ALL_REGULAR_E, toolid ),
  private_( new TransformToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->signal_block_count_ = 0;

  double inf = std::numeric_limits< double >::infinity();
  double epsilon = 0.001;
  this->add_state( "origin_x", this->origin_state_[ 0 ], 0.0, -inf, inf, 0.01 );
  this->add_state( "origin_y", this->origin_state_[ 1 ], 0.0, -inf, inf, 0.01 );
  this->add_state( "origin_z", this->origin_state_[ 2 ], 0.0, -inf, inf, 0.01 );
  this->add_state( "spacing_x", this->spacing_state_[ 0 ], 1.0, epsilon, inf, 0.01 );
  this->add_state( "spacing_y", this->spacing_state_[ 1 ], 1.0, epsilon, inf, 0.01 );
  this->add_state( "spacing_z", this->spacing_state_[ 2 ], 1.0, epsilon, inf, 0.01 );

  this->add_state( "keep_aspect_ratio", this->keep_aspect_ratio_state_, true );
  this->add_state( "preview_layer", this->preview_layer_state_, "", "" );
  this->add_state( "show_border", this->show_border_state_, true );
  this->add_state( "show_preview", this->show_preview_state_, true );

  this->add_state( "replace", this->replace_state_, false );

  this->add_connection( this->target_group_state_->value_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_target_group_changed, this->private_, _2 ),
    boost::signals2::at_front ) );
  this->add_connection( this->target_layers_state_->state_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_target_layers_changed, this->private_ ) ) );

  for ( int i = 0; i < 3; ++i )
  {
    this->add_connection( this->origin_state_[ i ]->state_changed_signal_.connect(
      boost::bind( &TransformToolPrivate::handle_origin_changed, this->private_ ) ) );
    this->add_connection( this->spacing_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &TransformToolPrivate::handle_spacing_changed, this->private_, i ) ) );
  }

  this->add_connection( this->keep_aspect_ratio_state_->value_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_keep_aspect_ratio_changed, this->private_, _1 ) ) );
  this->add_connection( this->preview_layer_state_->state_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_preview_layer_changed, this->private_ ) ) );
  this->add_connection( this->show_border_state_->value_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_show_border_changed, this->private_, _1 ) ) );
  this->add_connection( this->show_preview_state_->value_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_show_preview_changed, this->private_, _1 ) ) );

  {
    Core::ScopedCounter signal_block( this->private_->signal_block_count_ );
    this->private_->handle_target_group_changed( this->target_group_state_->get() );
    this->private_->handle_target_layers_changed();
  }
}

TransformTool::~TransformTool()
{
  this->disconnect_all();
}

void TransformTool::execute( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
 
  Core::Point origin( this->origin_state_[ 0 ]->get(),
    this->origin_state_[ 1 ]->get(), this->origin_state_[ 2 ]->get() );
  Core::Vector spacing( this->spacing_state_[ 0 ]->get(),
    this->spacing_state_[ 1 ]->get(), this->spacing_state_[ 2 ]->get() );
  // Reverse the order of target layers
  std::vector< std::string > target_layers = this->target_layers_state_->get();
  std::reverse( target_layers.begin(), target_layers.end() );

  ActionTransform::Dispatch( context, target_layers, origin, spacing, this->replace_state_->get() );
  Core::Application::PostEvent( boost::bind( &Core::StateBool::set, 
    this->show_preview_state_, false, Core::ActionSource::NONE_E ) );
  Core::Application::PostEvent( boost::bind( &Core::StateBool::set, 
    this->show_border_state_, false, Core::ActionSource::NONE_E ) );
}

void TransformTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat,
  int viewer_width, int viewer_height )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  bool show_border;
  bool show_preview;
  Core::VolumeSliceHandle vol_slice;
  Core::Color color;
  double depth;
  Core::GridTransform grid_trans;
  Core::Point world_min, world_max;

  {
    TransformToolPrivate::lock_type private_lock( this->private_->get_mutex() );
    grid_trans = this->private_->grid_transform_;
    world_min = this->private_->world_min_;
    world_max = this->private_->world_max_;
  }
  
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

    show_border = this->show_border_state_->get();
    std::string preview_layer = this->preview_layer_state_->get();
    show_preview = this->show_preview_state_->get() && preview_layer != "";

    if ( show_preview )
    {
      LayerHandle layer = LayerManager::Instance()->find_layer_by_id( preview_layer );
      switch ( layer->get_type() )
      {
      case Core::VolumeType::MASK_E:
        {
          MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
          color = PreferencesManager::Instance()->get_color( 
            mask_layer->color_state_->get() );
          Core::MaskVolumeHandle dummy_vol( new Core::MaskVolume( grid_trans,
            mask_layer->get_mask_volume()->get_mask_data_block() ) );
          vol_slice.reset( new Core::MaskVolumeSlice( dummy_vol, slice_type ) );
        }
        break;
      case Core::VolumeType::DATA_E:
        {
          DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
          Core::DataVolumeHandle dummy_vol( new Core::DataVolume( grid_trans,
            data_layer->get_data_volume()->get_data_block() ) );
          vol_slice.reset( new Core::DataVolumeSlice( dummy_vol, slice_type ) );
        }
        break;
      default:
        assert( false );
      }

      vol_slice->move_slice_to( depth );
    }
  }

  this->private_->initialize_gl();

  double start_x, start_y, end_x, end_y, depth_min, depth_max;
  Core::VolumeSlice::ProjectOntoSlice( slice_type, world_min, start_x, start_y, depth_min );
  Core::VolumeSlice::ProjectOntoSlice( slice_type, world_max, end_x, end_y, depth_max );

  bool in_range = depth >= depth_min && depth <= depth_max;

  glPushAttrib( GL_LINE_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT );
  
  Core::Texture::SetActiveTextureUnit( 0 );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glLineWidth( 1.0f );

  // Render the preview if the volume slice is available
  if ( show_preview && in_range && vol_slice->is_valid() )
  {
    vol_slice->initialize_texture();
    vol_slice->upload_texture();

    MaskShader::lock_type shader_lock( this->private_->shader_->get_mutex() );
    Core::Texture2DHandle slice_tex = vol_slice->get_texture();
    Core::Texture::lock_type tex_lock( slice_tex->get_mutex() );

    if ( vol_slice->get_volume()->get_type() == Core::VolumeType::MASK_E )
    {
      this->private_->shader_->enable();
      this->private_->shader_->set_color( static_cast< float >( color.r() / 255 ), 
        static_cast< float >( color.g() / 255 ), static_cast< float >( color.b() / 255 ) );
    }

    slice_tex->enable();

    glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2d( start_x, start_y );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2d( end_x, start_y );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2d( end_x, end_y );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2d( start_x, end_y );
    glEnd();

    slice_tex->disable();

    if ( vol_slice->get_volume()->get_type() == Core::VolumeType::MASK_E )
    {
      this->private_->shader_->disable();
    }
  }
  
  if ( show_border )
  {
    glColor4f( 1.0f, 0.0f, 0.0f, in_range ? 1.0f : 0.5f );

    glBegin( GL_LINE_LOOP );
    glVertex2d( start_x, start_y );
    glVertex2d( end_x, start_y );
    glVertex2d( end_x, end_y );
    glVertex2d( start_x, end_y );
    glEnd();
  }
  
  glPopMatrix();
  glPopAttrib();
  glFinish();
}

bool TransformTool::has_2d_visual()
{
  return true;
}

bool TransformTool::handle_mouse_leave( ViewerHandle viewer )
{
  this->private_->resizing_ = false;
  return true;
}

bool TransformTool::handle_mouse_move( ViewerHandle viewer, 
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

bool TransformTool::handle_mouse_press( ViewerHandle viewer, 
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

bool TransformTool::handle_mouse_release( ViewerHandle viewer, 
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

void TransformTool::deactivate()
{
  ViewerManager::Instance()->reset_cursor();
}

void TransformTool::reset()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &TransformTool::reset, this ) );
    return;
  }
  
  const std::string& group_id = this->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }
  
  this->private_->handle_target_group_changed( group_id );
  if ( this->show_border_state_->get() || ( this->show_preview_state_->get() &&
    this->preview_layer_state_->get() != "" ) )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  }
}

} // end namespace Seg3D
