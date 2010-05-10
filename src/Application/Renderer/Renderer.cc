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

// Boost includes
#include <boost/lexical_cast.hpp>


// Core includes
#include <Core/Application/Application.h>
#include <Core/Utils/Log.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Geometry/View3D.h>
#include <Core/Graphics/UnitCube.h>

// Application includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Renderer/Renderer.h>
#include <Application/ViewerManager/ViewerManager.h>


namespace Seg3D
{

static const unsigned int PATTERN_SIZE_C = 6;
static const unsigned char MASK_PATTERNS_C[][ PATTERN_SIZE_C ][ PATTERN_SIZE_C ] =
{
  //{ { 0, 0, 0, 255, 255, 255, 0, 0 }, { 0, 0, 0, 0, 255, 255, 255, 0 }, 
  //  { 0, 0, 0, 0, 0, 255, 255, 255 }, { 255, 0, 0, 0, 0, 0, 255, 255 },
  //  { 255, 255, 0, 0, 0, 0, 0, 255 }, { 255, 255, 255, 0, 0, 0, 0, 0 },
  //  { 0, 255, 255, 255, 0, 0, 0, 0 }, { 0, 0, 255, 255, 255, 0, 0, 0 } },

  //{ { 0, 0, 0, 255, 255, 0, 0, 0 }, { 0, 0, 0, 0, 255, 255, 0, 0 }, 
  //  { 0, 0, 0, 0, 0, 255, 255, 0 }, { 0, 0, 0, 0, 0, 0, 255, 255 },
  //  { 255, 0, 0, 0, 0, 0, 0, 255 }, { 255, 255, 0, 0, 0, 0, 0, 0 },
  //  { 0, 255, 255, 0, 0, 0, 0, 0 }, { 0, 0, 255, 255, 0, 0, 0, 0 } }

  { { 0, 0, 0, 255, 0, 0 }, { 0, 0, 0, 0, 255, 0 }, 
    { 0, 0, 0, 0, 0, 255 }, { 255, 0, 0, 0, 0, 0 },
    { 0, 255, 0, 0, 0, 0 }, { 0, 0, 255, 0, 0, 0 } }
};

static const int NUM_OF_PATTERNS_C = 
  sizeof( MASK_PATTERNS_C ) / ( PATTERN_SIZE_C * PATTERN_SIZE_C );

Renderer::Renderer() :
  RendererBase(), 
  ConnectionHandler(), 
  slice_shader_( new SliceShader ),
  text_renderer_( new Core::TextRenderer )
{
}

Renderer::~Renderer()
{
  this->disconnect_all();
}

void Renderer::post_initialize()
{
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glPixelStorei( GL_PACK_ALIGNMENT, 1 );
  glDisable( GL_CULL_FACE );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDepthFunc( GL_LEQUAL );

  {
    // lock the shared render context
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );

    this->cube_ = Core::UnitCubeHandle( new Core::UnitCube() );
    this->slice_shader_->initialize();
    this->pattern_texture_ = Core::Texture3DHandle( new Core::Texture3D );
    this->pattern_texture_->set_image( PATTERN_SIZE_C, PATTERN_SIZE_C, NUM_OF_PATTERNS_C, 
      GL_ALPHA, MASK_PATTERNS_C, GL_ALPHA, GL_UNSIGNED_BYTE );
    this->text_texture_ = Core::Texture2DHandle( new Core::Texture2D );
  }

  this->slice_shader_->enable();
  this->slice_shader_->set_slice_texture( 0 );
  this->slice_shader_->set_pattern_texture( 1 );
  this->slice_shader_->disable();
  Core::Texture::SetActiveTextureUnit( 1 );
  this->pattern_texture_->bind();
  this->pattern_texture_->set_mag_filter( GL_LINEAR );
  this->pattern_texture_->set_min_filter( GL_LINEAR );
  this->pattern_texture_->set_wrap_s( GL_REPEAT );
  this->pattern_texture_->set_wrap_t( GL_REPEAT );
  this->pattern_texture_->set_wrap_r( GL_CLAMP );
  Core::Texture::SetActiveTextureUnit( 0 );

  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
  this->add_connection( viewer->redraw_signal_.connect( 
    boost::bind( &Renderer::redraw, this, _1 ) ) );
  this->add_connection( viewer->redraw_overlay_signal_.connect( 
    boost::bind( &Renderer::redraw_overlay, this, _1 ) ) );

  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; i++ )
  {
    if ( i != this->viewer_id_ )
    {
      this->add_connection( ViewerManager::Instance()->get_viewer( i )->
        slice_changed_signal_.connect( boost::bind( &Renderer::viewer_slice_changed, this, _1 ) ) );
      this->add_connection( ViewerManager::Instance()->get_viewer( i )->view_mode_state_->
        state_changed_signal_.connect( boost::bind( &Renderer::viewer_mode_changed, this, i ) ) );
    }
  }
  this->add_connection( ViewerManager::Instance()->picking_target_changed_signal_.connect(
    boost::bind( &Renderer::picking_target_changed, this, _1 ) ) );
}

bool Renderer::render()
{
  CORE_LOG_DEBUG( std::string("Renderer ") + Core::ToString( this->viewer_id_ ) 
    + ": starting redraw" );

  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  // Lock the state engine
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );

  if ( viewer->is_volume_view() )
  {
    Core::View3D view3d( viewer->volume_view_state_->get() );
    std::vector< LayerSceneHandle > layer_scenes;
    std::vector< double > depths;
    std::vector< std::string > view_modes;
    size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
    for ( size_t i = 0; i < num_of_viewers; i++ )
    {
      ViewerHandle other_viewer = ViewerManager::Instance()->get_viewer( i );
      if ( !other_viewer->viewer_visible_state_->get() || 
        !other_viewer->slice_visible_state_->get() || 
        other_viewer->is_volume_view() )
      {
        continue;
      }
      // Get a snapshot of current layers
      LayerSceneHandle layer_scene = LayerManager::Instance()->compose_layer_scene( i );
      
      // Copy slices from viewer
      {
        Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
        this->process_slices( layer_scene, other_viewer );
      }

      if ( layer_scene->size() > 0 )
      {
        layer_scenes.push_back( layer_scene );
        Core::StateView2D* view2d_state = static_cast< Core::StateView2D* >(
          other_viewer->get_active_view_state().get() );
        depths.push_back( view2d_state->get().center().z() );
        view_modes.push_back( other_viewer->view_mode_state_->get() );
      }
    }

    Core::BBox bbox = LayerManager::Instance()->get_layers_bbox();

    // We have got everything we want from the state engine, unlock before we do any rendering
    state_lock.unlock();

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    double znear, zfar;
    view3d.compute_clipping_planes( bbox, znear, zfar );
    gluPerspective( view3d.fov(), this->width_ / ( 1.0 * this->height_ ), znear, zfar );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( view3d.eyep().x(), view3d.eyep().y(), view3d.eyep().z(), view3d.lookat().x(),
        view3d.lookat().y(), view3d.lookat().z(), view3d.up().x(), view3d.up().y(), view3d.up().z() );

    this->draw_slices_3d( bbox, layer_scenes, depths, view_modes );

    glDisable( GL_BLEND );
  }
  else
  {
    // Get a snapshot of current layers
    LayerSceneHandle layer_scene = LayerManager::Instance()->compose_layer_scene( this->viewer_id_ );
    
    // Copy slices from viewer
    {
      Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
      this->process_slices( layer_scene, viewer );
    }

    Core::View2D view2d(
        static_cast< Core::StateView2D* > ( viewer->get_active_view_state().get() )->get() );

    // We have got everything we want from the state engine, unlock before we do any rendering
    state_lock.unlock();

    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    double left, right, top, bottom;
    view2d.compute_clipping_planes( this->width_ / ( 1.0 * this->height_ ), 
      left, right, bottom, top );
    Core::Matrix proj_mat;
    Core::Transform::BuildOrtho2DMatrix( proj_mat, left, right, bottom, top );
    glMultMatrixd( proj_mat.data() );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    this->slice_shader_->enable();
    this->slice_shader_->set_border_width( 1 );

    Core::VolumeSlice* volume_slice = 0;
    for ( size_t layer_num = 0; layer_num < layer_scene->size(); layer_num++ )
    {
      LayerSceneItemHandle layer_item = ( *layer_scene )[ layer_num ];
      this->slice_shader_->set_volume_type( layer_item->type() );
      switch ( layer_item->type() )
      {
      case Core::VolumeType::DATA_E:
        {
          DataLayerSceneItem* data_layer_item = 
            dynamic_cast< DataLayerSceneItem* >( layer_item.get() );
          Core::DataVolumeSlice* data_slice = data_layer_item->data_volume_slice_.get();
          volume_slice = data_slice;

          // Convert contrast to range ( 0, 1 ] and brightness to [ -1, 1 ]
          double contrast = ( 1 - data_layer_item->contrast_ / 101 );
          double brightness = data_layer_item->brightness_ / 50 - 1.0;

          // NOTE: The equations for computing scale and bias are as follows:
          //
          // double scale = numeric_range / ( contrast * value_range );
          // double window_max = -brightness * ( value_max - value_min ) + value_max;
          // double bias = ( numeric_max - window_max * scale ) / numeric_max;
          //
          // However, since we always rescale the data to unsigned short when uploading
          // textures, numeric_range and value_range are the same, 
          // and thus the computation can be simplified.
          
          double scale = 1.0 / contrast;
          double bias = 1.0 - ( 1.0 - brightness ) * scale;
          this->slice_shader_->set_scale_bias( static_cast< float >( scale ), 
            static_cast< float >( bias ) );
        }
        break;
      case Core::VolumeType::MASK_E:
        {
          MaskLayerSceneItem* mask_layer_item = 
            dynamic_cast< MaskLayerSceneItem* >( layer_item.get() );
          Core::MaskVolumeSlice* mask_slice = mask_layer_item->mask_volume_slice_.get();
          volume_slice = mask_slice;
          this->slice_shader_->set_mask_mode( 0 );
          this->slice_shader_->set_mask_color( 1.0f, 0.6f, 0.0f );
        }
        break;
      default:
        assert( false );
        continue;
      } // end switch

      // Compute the size of the slice on screen
      Core::Vector slice_x( volume_slice->right() - volume_slice->left(), 0.0, 0.0 );
      slice_x = proj_mat * slice_x;
      double slice_screen_width = slice_x.x() / 2.0 * this->width_;
      double slice_screen_height = ( volume_slice->top() - volume_slice->bottom() ) / 
        ( volume_slice->right() - volume_slice->left() ) * slice_screen_width;
      float pattern_repeats_x = static_cast< float >( slice_screen_width / PATTERN_SIZE_C );
      float pattern_repeats_y = static_cast< float >( slice_screen_height / PATTERN_SIZE_C );

      this->slice_shader_->set_opacity( static_cast< float >( layer_item->opacity_ ) );
      this->slice_shader_->set_pixel_size( static_cast< float >( 1.0 / slice_screen_width ), 
        static_cast< float >( 1.0 /slice_screen_height ) );
      Core::TextureHandle slice_tex = volume_slice->get_texture();
      Core::Texture::lock_type slice_tex_lock( slice_tex->get_mutex() );
      slice_tex->bind();
      glBegin( GL_QUADS );
      glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 0.0f );
      glMultiTexCoord3f( GL_TEXTURE1, 0.0f, 0.0f, 0.0f );
      glVertex2d( volume_slice->left(), volume_slice->bottom() );
      glMultiTexCoord2f( GL_TEXTURE0, 1.0f, 0.0f );
      glMultiTexCoord3f( GL_TEXTURE1, pattern_repeats_x, 0.0f, 0.0f );
      glVertex2d( volume_slice->right(), volume_slice->bottom() );
      glMultiTexCoord2f( GL_TEXTURE0, 1.0f, 1.0f );
      glMultiTexCoord3f( GL_TEXTURE1, pattern_repeats_x, pattern_repeats_y, 0.0f );
      glVertex2d( volume_slice->right(), volume_slice->top() );
      glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 1.0f );
      glMultiTexCoord3f( GL_TEXTURE1, 0.0f, pattern_repeats_y, 0.0f );
      glVertex2d( volume_slice->left(), volume_slice->top() );
      glEnd();
      // NOTE: Always unbind, because we are deleting textures in a separate thread/context.
      // In this case texture binding of the rendering thread won't be reverted to 0, which
      // will cause problem when a new texture with the same ID is generated and bound to
      // this context, because the driver would think it's already bound.
      slice_tex->unbind();

    } // end for

    this->slice_shader_->disable();
    glDisable( GL_BLEND );
  }

  CORE_LOG_DEBUG( std::string("Renderer ") + Core::ToString( this->viewer_id_ ) 
    + ": done redraw" );

  return true;
}

bool Renderer::render_overlay()
{
  CORE_LOG_DEBUG( std::string("Renderer ") + Core::ToString( this->viewer_id_ ) 
    + ": starting redraw overlay" );

  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  // Lock the state engine
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );

  if ( viewer->is_volume_view() )
  {
    state_lock.unlock();
  }
  else
  {
    bool show_grid = viewer->slice_grid_state_->get();
    Core::View2D view2d(
      static_cast< Core::StateView2D* > ( viewer->get_active_view_state().get() )->get() );
    int view_mode = viewer->view_mode_state_->index();
    ViewerInfoList viewers_info[ 3 ];
    ViewerManager::Instance()->get_2d_viewers_info( viewers_info );

    // We have got everything we want from the state engine, unlock before we do any rendering
    state_lock.unlock();

    double left, right, top, bottom;
    view2d.compute_clipping_planes( this->width_ / ( 1.0 * this->height_ ), 
      left, right, bottom, top );
    Core::Matrix proj_mat;
    Core::Transform::BuildOrtho2DMatrix( proj_mat, left, right, bottom, top );

    glDisable( GL_DEPTH_TEST );
    // Enable blending
    glEnable( GL_BLEND );
    // NOTE: The result of the following blend function is that, color channels contains
    // colors modulated by alpha, alpha channel stores the value of "1-alpha"
    glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA , 
      GL_ZERO, GL_ONE_MINUS_SRC_ALPHA  );

    gluOrtho2D( 0, this->width_, 0, this->height_ );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Render the grid
    if ( show_grid )
    {
      glColor4f( 0.1f, 0.1f, 0.1f, 0.75f );
      int grid_spacing = 50;
      int center_x = this->width_ / 2;
      int center_y = this->height_ / 2;
      int vertical_lines = center_x / grid_spacing;
      int horizontal_lines = center_y / grid_spacing;
      glBegin( GL_LINES );
      glVertex2i( center_x, 0 );
      glVertex2i( center_x, this->height_ );
      for ( int i = 1; i <= vertical_lines; i++ )
      {
        glVertex2i( center_x - grid_spacing * i, 0 );
        glVertex2i( center_x - grid_spacing * i, this->height_ );
        glVertex2i( center_x + grid_spacing * i, 0 );
        glVertex2i( center_x + grid_spacing * i, this->height_ );
      }
      glVertex2i( 0, center_y );
      glVertex2i( this->width_, center_y );
      for ( int i = 0; i <= horizontal_lines; i++ )
      {
        glVertex2i( 0, center_y - grid_spacing * i );
        glVertex2i( this->width_, center_y - grid_spacing * i );
        glVertex2i( 0, center_y + grid_spacing * i );
        glVertex2i( this->width_, center_y + grid_spacing * i );
      }
      glEnd();
    }
    
    // Render the positions of slices in other viewers
    glLineStipple( 1, 0x1C47 );
    int vert_slice_mode = ( view_mode + 2 ) % 3;
    int hori_slice_mode = ( view_mode + 1 ) % 3;
    size_t num_of_vert_slices = viewers_info[ vert_slice_mode ].size();
    size_t num_of_hori_slices = viewers_info[ hori_slice_mode ].size();
    for ( size_t i = 0; i < num_of_vert_slices; i++ )
    {
      Core::Point pt( viewers_info[ vert_slice_mode ][ i ]->depth_, 0, 0 );
      pt = proj_mat * pt;
      int slice_pos = Core::Round( ( pt.x() + 1.0 ) / 2.0 * ( this->width_ - 1 ) ) + 1;
      float color[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
      color[ vert_slice_mode ] = 1.0f;
      if ( viewers_info[ vert_slice_mode ][ i ]->is_picking_target_ )
      {
        glDisable( GL_LINE_STIPPLE );
      }
      else
      {
        glEnable( GL_LINE_STIPPLE );
        color[ 3 ] = 0.5f;
      }
      //color[ 3 ] = viewers_info[ vert_slice_mode ][ i ]->is_picking_target_ ? 0.75f : 0.3f;
      glColor4fv( color );
      glBegin( GL_LINES );
      glVertex2i( slice_pos, 0 );
      glVertex2i( slice_pos, this->height_ );
      glEnd();
    }
    for ( size_t i = 0; i < num_of_hori_slices; i++ )
    {
      Core::Point pt( 0, viewers_info[ hori_slice_mode ][ i ]->depth_, 0 );
      pt = proj_mat * pt;
      int slice_pos = Core::Round( ( pt.y() + 1.0 ) / 2.0 * ( this->height_ - 1 ) ) + 1;
      float color[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
      color[ hori_slice_mode ] = 1.0f;
      if ( viewers_info[ hori_slice_mode ][ i ]->is_picking_target_ )
      {
        glDisable( GL_LINE_STIPPLE );
      }
      else
      {
        glEnable( GL_LINE_STIPPLE );
        color[ 3 ] = 0.5f;
      }
      //color[ 3 ] = viewers_info[ hori_slice_mode ][ i ]->is_picking_target_ ? 0.75f : 0.3f;
      glColor4fv( color );    
      glBegin( GL_LINES );
      glVertex2i( 0, slice_pos);
      glVertex2i( this->width_, slice_pos );
      glEnd();
    }
    glDisable( GL_LINE_STIPPLE );

    // Render the text
    std::vector< unsigned char > buffer( this->width_ * this->height_, 0 );
    std::vector< std::string > text;
    text.push_back( std::string( "Konnichi wa minasan! " ) );
    text.push_back( std::string( "NUMIRA" ) );
    this->text_renderer_->render( text, &buffer[ 0 ], this->width_, this->height_, 5, 20, 10, -1 );
    //this->text_renderer_->render_aligned( text[ 1 ], &buffer[ 0 ], this->width_, this->height_, 48,
    //  50, Core::TextHAlignmentType::CENTER_E, Core::TextVAlignmentType::CENTER_E );
    this->text_texture_->enable();
    this->text_texture_->set_sub_image( 0, 0, this->width_, this->height_,
        &buffer[ 0 ], GL_ALPHA, GL_UNSIGNED_BYTE );

    // Blend the text onto the framebuffer
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
    glBegin( GL_QUADS );
    glColor4f( 1.0f, 0.1f, 0.1f, 0.4f );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2i( 0, 0 );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2i( this->width_, 0 );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2i( this->width_, this->height_ );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2i( 0, this->height_ );
    glEnd();
    this->text_texture_->disable();

    glDisable( GL_BLEND );
  }

  CORE_LOG_DEBUG( std::string("Renderer ") + Core::ToString( this->viewer_id_ ) 
    + ": done redraw overlay" );

  return true;
}

void Renderer::post_resize()
{
  {
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
    this->text_texture_->set_image( this->width_, this->height_, GL_ALPHA );
  }
}

void Renderer::process_slices( LayerSceneHandle& layer_scene, ViewerHandle& viewer )
{
  for ( size_t layer_num = 0; layer_num < layer_scene->size(); layer_num++ )
  {
    LayerSceneItemHandle layer_item = ( *layer_scene )[ layer_num ];
    switch ( layer_item->type() )
    {
    case Core::VolumeType::DATA_E:
      {
        DataLayerSceneItem* data_layer_item = 
          dynamic_cast< DataLayerSceneItem* >( layer_item.get() );
        Core::DataVolumeSliceHandle data_volume_slice = 
          viewer->get_data_volume_slice( layer_item->layer_id_ );
        if ( data_volume_slice && !data_volume_slice->out_of_boundary() )
        {
          data_volume_slice->initialize_texture();
          data_volume_slice->upload_texture();
          data_layer_item->data_volume_slice_ = 
            Core::DataVolumeSliceHandle( new Core::DataVolumeSlice( *data_volume_slice ) );
        }
        else
        {
          layer_scene->erase( layer_scene->begin() + layer_num );
          layer_num--;
        }
      }
      break;
    case Core::VolumeType::MASK_E:
      {
        MaskLayerSceneItem* mask_layer_item = 
          dynamic_cast< MaskLayerSceneItem* >( layer_item.get() );
        Core::MaskVolumeSliceHandle mask_volume_slice = 
          viewer->get_mask_volume_slice( layer_item->layer_id_ );
        if ( mask_volume_slice && !mask_volume_slice->out_of_boundary() )
        {
          mask_volume_slice->initialize_texture();
          mask_volume_slice->upload_texture();
          mask_layer_item->mask_volume_slice_ = 
            Core::MaskVolumeSliceHandle( new Core::MaskVolumeSlice( *mask_volume_slice ) );
        }
        else
        {
          layer_scene->erase( layer_scene->begin() + layer_num );
          layer_num--;
        }
      }
      break;
    } // end switch
  } // end for

  // Make the bottom layer fully opaque
  if ( layer_scene->size() != 0 )
  {
    ( *layer_scene )[ 0 ]->opacity_ = 1.0;
  }
}

void Renderer::viewer_slice_changed( size_t viewer_id )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  ViewerHandle self_viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
  ViewerHandle updated_viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( self_viewer->view_mode_state_->index() !=
    updated_viewer->view_mode_state_->index() )
  {
    if ( self_viewer->is_volume_view() )
    {
      this->redraw();
    }
    else
    {
      this->redraw_overlay();
    }
  }
}

void Renderer::viewer_mode_changed( size_t viewer_id )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  if ( ViewerManager::Instance()->get_viewer( viewer_id )->viewer_visible_state_->get() )
  {
    if ( ViewerManager::Instance()->get_viewer( this->viewer_id_ )->is_volume_view() )
    {
      this->redraw();
    }
    else
    {
      this->redraw_overlay();
    }
  }
}

void Renderer::picking_target_changed( size_t viewer_id )
{
  if ( this->viewer_id_ == viewer_id )
  {
    return;
  }
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  ViewerHandle self_viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
  ViewerHandle updated_viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( self_viewer->view_mode_state_->index() !=
    updated_viewer->view_mode_state_->index() )
  {
    this->redraw_overlay();
  }
}

void Renderer::draw_slices_3d( const Core::BBox& bbox, 
                const std::vector< LayerSceneHandle >& layer_scenes, 
                const std::vector< double >& depths,
                const std::vector< std::string >& view_modes )
{
  this->slice_shader_->enable();
  size_t num_of_viewers = layer_scenes.size();
  // for each visible 2D viewer
  for ( size_t i = 0; i < num_of_viewers; i++ )
  {
    std::string view_mode = view_modes[ i ];
    double depth = depths[ i ];
    LayerSceneHandle layer_scene = layer_scenes[ i ];
    Core::Point bottomleft, bottomright, topleft, topright;
    double left, right, bottom, top;
    if ( view_mode == Viewer::AXIAL_C )
    {
      bottomleft = bbox.min();
      bottomleft[ 2 ] = depth;

      bottomright[ 0 ] = bbox.max()[ 0 ];
      bottomright[ 1 ] = bbox.min()[ 1 ];
      bottomright[ 2 ] = depth;
      
      topleft[ 0 ] = bbox.min()[ 0 ];
      topleft[ 1 ] = bbox.max()[ 1 ];
      topleft[ 2 ] = depth;
      
      topright = bbox.max();
      topright[ 2 ] = depth;

      left = bottomleft[ 0 ];
      right = bottomright[ 0 ];
      bottom = bottomleft[ 1 ];
      top = topleft[ 1 ];
    }
    else if ( view_mode == Viewer::CORONAL_C )
    {
      bottomleft = bbox.min();
      bottomleft[ 1 ] = depth;
      
      bottomright[ 0 ] = bbox.min()[ 0 ];
      bottomright[ 2 ] = bbox.max()[ 2 ];
      bottomright[ 1 ] = depth;

      topleft[ 0 ] = bbox.max()[ 0 ];
      topleft[ 2 ] = bbox.min()[ 2 ];
      topleft[ 1 ] = depth;

      topright = bbox.max();
      topright[ 1 ] = depth;

      left = bottomleft[ 2 ];
      right = bottomright[ 2 ];
      bottom = bottomleft[ 0 ];
      top = topleft[ 0 ];
    }
    else
    {
      bottomleft = bbox.min();
      bottomleft[ 0 ] = depth;
    
      bottomright[ 1 ] = bbox.max()[ 1 ];
      bottomright[ 2 ] = bbox.min()[ 2 ];
      bottomright[ 0 ] = depth;

      topleft[ 1 ] = bbox.min()[ 1 ];
      topleft[ 2 ] = bbox.max()[ 2 ];
      topleft[ 0 ] = depth;

      topright = bbox.max();
      topright[ 0 ] = depth;

      left = bottomleft[ 1 ];
      right = bottomright[ 1 ];
      bottom = bottomleft[ 2 ];
      top = topleft[ 2 ];
    }

    Core::VolumeSlice* volume_slice = 0;
    // for each slice
    for ( size_t layer_num = 0; layer_num < layer_scene->size(); layer_num++ )
    {
      LayerSceneItemHandle layer_item = ( *layer_scene )[ layer_num ];
      this->slice_shader_->set_volume_type( layer_item->type() );
      switch ( layer_item->type() )
      {
      case Core::VolumeType::DATA_E:
        {
          DataLayerSceneItem* data_layer_item = 
            dynamic_cast< DataLayerSceneItem* >( layer_item.get() );
          Core::DataVolumeSlice* data_slice = data_layer_item->data_volume_slice_.get();
          volume_slice = data_slice;

          // Convert contrast to range ( 0, 1 ] and brightness to [ -1, 1 ]
          double contrast = ( 1 - data_layer_item->contrast_ / 101 );
          double brightness = data_layer_item->brightness_ / 50 - 1.0;
          double scale = 1.0 / contrast;
          double bias = 1.0 - ( 1.0 - brightness ) * scale;
          this->slice_shader_->set_scale_bias( static_cast< float >( scale ), 
            static_cast< float >( bias ) );
        }
        break;
      case Core::VolumeType::MASK_E:
        {
          MaskLayerSceneItem* mask_layer_item = 
            dynamic_cast< MaskLayerSceneItem* >( layer_item.get() );
          Core::MaskVolumeSlice* mask_slice = mask_layer_item->mask_volume_slice_.get();
          volume_slice = mask_slice;
          this->slice_shader_->set_mask_mode( 2 );
          this->slice_shader_->set_mask_color( 1.0f, 0.6f, 0.0f );
        }
        break;
      default:
        assert( false );
        continue;
      } // end switch


      this->slice_shader_->set_opacity( static_cast< float >( layer_item->opacity_ ) );
      double slice_width = volume_slice->right() - volume_slice->left();
      double slice_height = volume_slice->top() - volume_slice->bottom();
      double tex_left = ( left - volume_slice->left() ) / slice_width;
      double tex_right = ( right - volume_slice->right() ) / slice_width + 1.0;
      double tex_bottom = ( bottom - volume_slice->bottom() ) / slice_height;
      double tex_top = ( top - volume_slice->top() ) / slice_height + 1.0;
      Core::TextureHandle slice_tex = volume_slice->get_texture();
      Core::Texture::lock_type slice_tex_lock( slice_tex->get_mutex() );
      slice_tex->bind();
      glBegin( GL_QUADS );
      glMultiTexCoord2d( GL_TEXTURE0, tex_left, tex_bottom );
      glVertex3dv( &bottomleft[ 0 ] );
      glMultiTexCoord2d( GL_TEXTURE0, tex_right, tex_bottom );
      glVertex3dv( &bottomright[ 0 ] );
      glMultiTexCoord2d( GL_TEXTURE0, tex_right, tex_top );
      glVertex3dv( &topright[ 0 ] );
      glMultiTexCoord2d( GL_TEXTURE0, tex_left, tex_top );
      glVertex3dv( &topleft[ 0 ] );
      glEnd();
      // NOTE: Always unbind, because we are deleting textures in a separate thread/context.
      // In this case texture binding of the rendering thread won't be reverted to 0, which
      // will cause problem when a new texture with the same ID is generated and bound to
      // this context, because the driver would think it's already bound.
      slice_tex->unbind();

    } // end for

  } // end for each viewer
  this->slice_shader_->disable();
}

} // end namespace Seg3D


