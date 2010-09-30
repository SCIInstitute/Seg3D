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
#include <Application/Tools/TransformTool.h>
#include <Application/Tools/detail/MaskShader.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
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

class TransformToolPrivate : Core::Lockable
{
public:

  void handle_target_group_changed( std::string group_id );
  void handle_layers_changed();

  void handle_preview_layer_changed();
  void handle_origin_changed();
  void handle_spacing_changed( int index );
  void handle_keep_aspect_ratio_changed( bool keep );
  void handle_show_border_changed( bool show );

  void initialize_gl();

  size_t signal_block_count_;
  TransformTool* tool_;
  int nx_, ny_, nz_;
  Core::Vector src_spacing_;
  MaskShaderHandle shader_;
};

void TransformToolPrivate::handle_target_group_changed( std::string group_id )
{
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  // NOTE: Lock the state engine because the following changes need to be atomic
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  {
    Core::ScopedCounter signal_block( this->signal_block_count_ );

    LayerGroupHandle layer_group = LayerManager::Instance()->get_layer_group( group_id );
    const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
    this->nx_ = static_cast< int >( grid_trans.get_nx() );
    this->ny_ = static_cast< int >( grid_trans.get_ny() );
    this->nz_ = static_cast< int >( grid_trans.get_nz() );

    Core::Point origin = grid_trans * Core::Point( 0, 0, 0 );
    this->src_spacing_ = grid_trans * Core::Vector( 1, 1, 1 );
    for ( int i = 0; i < 3; ++i )
    {
      this->tool_->origin_state_[ i ]->set( origin[ i ] );
      this->tool_->spacing_state_[ i ]->set( this->src_spacing_[ i ] );
    }
  }
}

void TransformToolPrivate::handle_layers_changed()
{
  this->tool_->preview_layer_state_->set_option_list(   this->tool_->
    target_layers_state_->get_option_list() );
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


//////////////////////////////////////////////////////////////////////////
// Class TransformTool
//////////////////////////////////////////////////////////////////////////

TransformTool::TransformTool( const std::string& toolid ) :
  GroupTargetTool( Core::VolumeType::ALL_E, toolid ),
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

  this->add_state( "replace", this->replace_state_, false );

  this->add_connection( this->target_group_state_->value_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_target_group_changed, this->private_, _2 ),
    boost::signals2::at_front ) );
  this->add_connection( this->target_layers_state_->optionlist_changed_signal_.connect(
    boost::bind( &TransformToolPrivate::handle_layers_changed, this->private_ ) ) );

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

  {
    Core::ScopedCounter signal_block( this->private_->signal_block_count_ );
    this->private_->handle_target_group_changed( this->target_group_state_->get() );
    this->private_->handle_layers_changed();
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
  ActionTransform::Dispatch( context, this->target_layers_state_->get(), 
    origin, spacing, this->replace_state_->get() );
}

void TransformTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  bool show_border;
  Core::Point world_min, world_max;
  Core::VolumeSliceHandle vol_slice;
  Core::Color color;

  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    std::string target_group_id = this->target_group_state_->get();
    if ( viewer->is_volume_view() || 
      target_group_id == "" || 
      target_group_id == Tool::NONE_OPTION_C )
    {
      return;
    }

    LayerGroupHandle layer_group = LayerManager::Instance()->get_layer_group( target_group_id );
    const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
    int nx = static_cast< int >( grid_trans.get_nx() );
    int ny = static_cast< int >( grid_trans.get_ny() );
    int nz = static_cast< int >( grid_trans.get_nz() );

    if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      slice_type = Core::VolumeSliceType::SAGITTAL_E;
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      slice_type = Core::VolumeSliceType::CORONAL_E;
    }

    Core::Point origin;
    Core::Vector spacing;
    for ( int i = 0; i < 3; ++i )
    {
      origin[ i ] = this->origin_state_[ i ]->get();
      spacing[ i ] = this->spacing_state_[ i ]->get();
    }

    Core::Transform transform;
    transform.load_basis( origin, Core::Vector( spacing[ 0 ], 0, 0 ), 
      Core::Vector( 0, spacing[ 1 ], 0 ), Core::Vector( 0, 0, spacing[ 2 ] ) );
    world_min = transform * Core::Point( -0.5, -0.5, -0.5 );
    world_max = transform * Core::Point( nx - 0.5, ny - 0.5, nz - 0.5 );

    show_border = this->show_border_state_->get();
    std::string preview_layer = this->preview_layer_state_->get();

    vol_slice = viewer->get_volume_slice( preview_layer );
    if ( vol_slice->is_valid() )
    {
      vol_slice->initialize_texture();
      vol_slice->upload_texture();
      vol_slice = vol_slice->clone();
    }
    else
    {
      vol_slice.reset();
    }

    LayerHandle layer = LayerManager::Instance()->get_layer_by_id( preview_layer );
    MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
    if ( mask_layer != 0 )
    {
      color = PreferencesManager::Instance()->get_color( 
        mask_layer->color_state_->get() );
    }   
  }

  this->private_->initialize_gl();

  double start_x, start_y, end_x, end_y;
  Core::VolumeSlice::ProjectOntoSlice( slice_type, world_min, start_x, start_y );
  Core::VolumeSlice::ProjectOntoSlice( slice_type, world_max, end_x, end_y );

  glPushAttrib( GL_LINE_BIT | GL_POINT_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT );
  
  Core::Texture::SetActiveTextureUnit( 0 );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glLineWidth( 2.0f );
  glPointSize( 2.0f );

  // Render the preview if the volume slice is available
  if ( vol_slice )
  {
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
  }
  
  glPopMatrix();
  glPopAttrib();
}

bool TransformTool::has_2d_visual()
{
  return true;
}

bool TransformTool::handle_mouse_leave( ViewerHandle viewer )
{
  return false;
}

bool TransformTool::handle_mouse_move( ViewerHandle viewer, 
                 const Core::MouseHistory& mouse_history, 
                 int button, int buttons, int modifiers )
{ 
  return false;
}

bool TransformTool::handle_mouse_press( ViewerHandle viewer, 
                  const Core::MouseHistory& mouse_history, 
                  int button, int buttons, int modifiers )
{
  return false;
}

bool TransformTool::handle_mouse_release( ViewerHandle viewer, 
                  const Core::MouseHistory& mouse_history, 
                  int button, int buttons, int modifiers )
{
  return false;
}

} // end namespace Seg3D
