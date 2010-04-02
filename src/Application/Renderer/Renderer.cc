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

// Utils includes
#include <Utils/Core/Log.h>
#include <Utils/RenderResources/RenderResources.h>
#include <Utils/EventHandler/DefaultEventHandlerContext.h>
#include <Utils/Geometry/View3D.h>
#include <Utils/Graphics/UnitCube.h>

// Application includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Renderer/Renderer.h>
#include <Application/ViewerManager/ViewerManager.h>

#if defined(_WIN32) || defined(__APPLE__) || defined(X11_THREADSAFE)
#define MULTITHREADED_RENDERING 1
#else
#define MULTITHREADED_RENDERING 0
#endif

namespace Seg3D
{

class RendererEventHandlerContext : public Utils::DefaultEventHandlerContext
{
public:

  RendererEventHandlerContext() :
    DefaultEventHandlerContext()
  {
  }
  virtual ~RendererEventHandlerContext()
  {
  }

  virtual void post_event( Utils::EventHandle& event )
  {
    boost::unique_lock< boost::mutex > lock( event_queue_mutex_ );

    // discard the previous rendering event
    //if (!event_queue_.empty())
    //{
    //  event_queue_.pop();
    //}

    event_queue_.push( event );
    event_queue_new_event_.notify_all();
  }
};

static const unsigned char MASK_PATTERNS_C[][ 8 ][ 8 ] =
{
  { { 0, 0, 0, 255, 255, 255, 0, 0 }, { 0, 0, 0, 0, 255, 255, 255, 0 }, 
    { 0, 0, 0, 0, 0, 255, 255, 255 }, { 255, 0, 0, 0, 0, 0, 255, 255 },
    { 255, 255, 0, 0, 0, 0, 0, 255 }, { 255, 255, 255, 0, 0, 0, 0, 0 },
    { 0, 255, 255, 255, 0, 0, 0, 0 }, { 0, 0, 255, 255, 255, 0, 0, 0 } }
};

static const int NUM_OF_PATTERNS_C = sizeof( MASK_PATTERNS_C ) / 64;

Renderer::Renderer() :
  ViewerRenderer(), 
  EventHandler(), 
  slice_shader_( new SliceShader ),
  active_render_texture_( 0 ), 
  width_( 0 ), 
  height_( 0 ),
  redraw_needed_( false )
{
}

Renderer::~Renderer()
{
  this->disconnect_all();
}

void Renderer::initialize()
{
#if MULTITHREADED_RENDERING
  // NOTE: it is important to postpone the allocation of OpenGL objects to the 
  // rendering thread. If created in a different thread, these objects might not
  // be ready when the rendering thread uses them the first time, which caused
  // the scene to be blank sometimes.
  if ( !is_eventhandler_thread() )
  {
    if ( !Utils::RenderResources::Instance()->create_render_context( this->context_ ) )
    {
      SCI_THROW_EXCEPTION( "Failed to create a valid rendering context" );
    }
    this->post_event( boost::bind( &Renderer::initialize, this ) );
    this->start_eventhandler();
    return;
  }

  SCI_LOG_DEBUG( "Initializing renderer in a separate thread" );

#else
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &Renderer::initialize, this ) );
    return;
  }

  SCI_LOG_DEBUG( "Initializing renderer in the interface thread" );

  if ( !Utils::RenderResources::Instance()->create_render_context( this->context_ ) )
  {
    SCI_THROW_EXCEPTION("Failed to create a valid rendering context");
  }
#endif

  // Make the GL context current. In multi-threaded rendering mode,
  // this call is only needed once.
  this->context_->make_current();

  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

  {
    // lock the shared render context
    Utils::RenderResources::lock_type lock( Utils::RenderResources::GetMutex() );

    this->textures_[ 0 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
    this->textures_[ 1 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
    this->depth_buffer_ = Utils::RenderbufferHandle( new Utils::Renderbuffer() );
    this->frame_buffer_ = Utils::FramebufferObjectHandle( new Utils::FramebufferObject() );
    this->frame_buffer_->attach_renderbuffer( depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );
    this->cube_ = Utils::UnitCubeHandle( new Utils::UnitCube() );
    this->slice_shader_->initialize();
    this->pattern_texture_ = Utils::Texture3DHandle( new Utils::Texture3D );
    this->pattern_texture_->set_image( 8, 8, NUM_OF_PATTERNS_C, GL_ALPHA, 
      MASK_PATTERNS_C, GL_ALPHA, GL_UNSIGNED_BYTE );
  }

  SCI_CHECK_OPENGL_ERROR();

  this->slice_shader_->enable();
  this->slice_shader_->set_slice_texture( 0 );
  this->slice_shader_->set_pattern_texture( 1 );
  this->slice_shader_->disable();
  Utils::Texture::SetActiveTextureUnit( 1 );
  this->pattern_texture_->bind();
  this->pattern_texture_->set_mag_filter( GL_LINEAR );
  this->pattern_texture_->set_min_filter( GL_LINEAR );
  this->pattern_texture_->set_wrap_s( GL_REPEAT );
  this->pattern_texture_->set_wrap_t( GL_REPEAT );
  this->pattern_texture_->set_wrap_r( GL_CLAMP );
  Utils::Texture::SetActiveTextureUnit( 0 );

  SCI_CHECK_OPENGL_ERROR();

  this->add_connection( ViewerManager::Instance()->get_viewer( this->viewer_id_ )
    ->redraw_signal_.connect( boost::bind( &Renderer::redraw, this ) ) );

  SCI_LOG_DEBUG( std::string("Renderer ") + Utils::to_string( this->viewer_id_ ) 
    + " initialized" );
}

void Renderer::redraw()
{
  if ( !this->is_active() )
  {
    return;
  }

#if MULTITHREADED_RENDERING
  if ( !is_eventhandler_thread() )
  {
    lock_type lock( this->redraw_needed_mutex_ );
    this->redraw_needed_ = true;
    this->post_event( boost::bind( &Renderer::redraw, this ) );
    return;
  }
#else
  if ( !Interface::IsInterfaceThread() )
  {
    lock_type lock( this->redraw_needed_mutex_ );
    this->redraw_needed_ = true;
    Interface::PostEvent( boost::bind( &Renderer::redraw, this ) );
    return;
  }
  // Make the GL context current in the interface thread
  this->context_->make_current();
#endif

  {
    lock_type lock( this->redraw_needed_mutex_ );
    if ( !this->redraw_needed_ )
    {
      return;
    }
    this->redraw_needed_ = false;
  }

  SCI_LOG_DEBUG( std::string("Renderer ") + Utils::to_string( this->viewer_id_ ) 
    + ": starting redraw" );

  // lock the active render texture
  Utils::Texture::lock_type texture_lock( this->textures_[ this->active_render_texture_ ]->get_mutex() );

  // bind the framebuffer object
  this->frame_buffer_->enable();

  // attach texture
  this->frame_buffer_->attach_texture( this->textures_[ this->active_render_texture_ ] );

  SCI_CHECK_OPENGL_ERROR();

  if ( !this->frame_buffer_->check_status() )
  {
    this->frame_buffer_->disable();
    return;
  }

  SCI_CHECK_OPENGL_ERROR();

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  // Lock the state engine
  StateEngine::lock_type state_lock( StateEngine::Instance()->get_mutex() );
  // Get a snapshot of current layers
  LayerSceneHandle layer_scene = LayerManager::Instance()->compose_layer_scene( this->viewer_id_ );

  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );

  if ( viewer->is_volume_view() )
  {
    Utils::View3D view3d( viewer->volume_view_state_->get() );

    // We have got everything we want from the state engine, unlock before we do any rendering
    state_lock.unlock();

    glEnable( GL_DEPTH_TEST );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    gluPerspective( view3d.fov(), this->width_ / ( 1.0 * this->height_ ), 0.1, 5.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( view3d.eyep().x(), view3d.eyep().y(), view3d.eyep().z(), view3d.lookat().x(),
        view3d.lookat().y(), view3d.lookat().z(), view3d.up().x(), view3d.up().y(), view3d.up().z() );

    glRotatef( 25.0f * ( this->viewer_id_ + 1 ), 1, 0, 1 );
    glScalef( 0.5f, 0.5f, 0.5f );
    glTranslatef( -0.5f, -0.5f, -0.5f );
    this->cube_->draw();
  }
  else
  {
    // Copy slices from viewer
    {
      Utils::RenderResources::lock_type lock( Utils::RenderResources::GetMutex() );
      this->process_slices( layer_scene, viewer );
    }

    Utils::View2D view2d(
        dynamic_cast< StateView2D* > ( viewer->get_active_view_state().get() )->get() );

    // We have got everything we want from the state engine, unlock before we do any rendering
    state_lock.unlock();

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    double left, right, top, bottom;
    view2d.compute_clipping_planes( this->width_ / ( 1.0 * this->height_ ), 
      left, right, bottom, top );
    Utils::Matrix proj_mat;
    Utils::Transform::BuildOrtho2DMatrix( proj_mat, left, right, bottom, top );
    glMultMatrixd( proj_mat.data() );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    this->slice_shader_->enable();

    Utils::VolumeSlice* volume_slice = 0;
    for ( size_t layer_num = 0; layer_num < layer_scene->size(); layer_num++ )
    {
      LayerSceneItemHandle layer_item = ( *layer_scene )[ layer_num ];
      switch ( layer_item->type() )
      {
      case Utils::VolumeType::DATA_E:
        {
          DataLayerSceneItem* data_layer_item = 
            dynamic_cast< DataLayerSceneItem* >( layer_item.get() );
          Utils::DataVolumeSlice* data_slice = data_layer_item->data_volume_slice_.get();
          volume_slice = data_slice;
          this->slice_shader_->set_mask_mode( false );

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
          this->slice_shader_->set_scale_bias( scale, bias );
        }
        break;
      case Utils::VolumeType::MASK_E:
        {
          MaskLayerSceneItem* mask_layer_item = 
            dynamic_cast< MaskLayerSceneItem* >( layer_item.get() );
          Utils::MaskVolumeSlice* mask_slice = mask_layer_item->mask_volume_slice_.get();
          volume_slice = mask_slice;
          this->slice_shader_->set_mask_mode( true );
        }
        break;
      default:
        assert( false );
        continue;
      } // end switch

      // Compute the size of the slice on screen
      Utils::Vector slice_x( volume_slice->right() - volume_slice->left(), 0.0, 0.0 );
      slice_x = proj_mat * slice_x;
      double slice_screen_width = slice_x.x() / 2.0 * this->width_;
      double slice_screen_height = ( volume_slice->top() - volume_slice->bottom() ) / 
        ( volume_slice->right() - volume_slice->left() ) * slice_screen_width;

      this->slice_shader_->set_opacity( static_cast< float >( layer_item->opacity_ ) );
      Utils::TextureHandle slice_tex = volume_slice->get_texture();
      Utils::Texture::lock_type slice_tex_lock( slice_tex->get_mutex() );
      slice_tex->bind();
      glBegin( GL_QUADS );
      glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 0.0f );
      glMultiTexCoord3f( GL_TEXTURE1, 0.0f, 0.0f, 0.0f );
      glVertex2d( volume_slice->left(), volume_slice->bottom() );
      glMultiTexCoord2f( GL_TEXTURE0, 1.0f, 0.0f );
      glMultiTexCoord3f( GL_TEXTURE1, slice_screen_width / 8.0f, 0.0f, 0.0f );
      glVertex2d( volume_slice->right(), volume_slice->bottom() );
      glMultiTexCoord2f( GL_TEXTURE0, 1.0f, 1.0f );
      glMultiTexCoord3f( GL_TEXTURE1, slice_screen_width / 8.0f, slice_screen_height / 8.0f, 0.0f );
      glVertex2d( volume_slice->right(), volume_slice->top() );
      glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 1.0f );
      glMultiTexCoord3f( GL_TEXTURE1, 0.0f, slice_screen_height / 8.0f, 0.0f );
      glVertex2d( volume_slice->left(), volume_slice->top() );
      glEnd();

    } // end for

    this->slice_shader_->disable();
  }

  SCI_CHECK_OPENGL_ERROR();

  glFinish();

  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();

  SCI_LOG_DEBUG( std::string("Renderer ") + Utils::to_string( this->viewer_id_ ) 
    + ": done redraw" );

  // signal rendering completed
  this->rendering_completed_signal_( this->textures_[ this->active_render_texture_ ] );

  // swap render textures 
  this->active_render_texture_ = ( ~this->active_render_texture_ ) & 1;
}

void Renderer::resize( int width, int height )
{
#if MULTITHREADED_RENDERING
  if ( !is_eventhandler_thread() )
  {
    this->post_event( boost::bind( &Renderer::resize, this, width, height ) );
    return;
  }
#else
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &Renderer::resize, this, width, height ) );
    return;
  }
  // Make the GL context current in the interface thread
  this->context_->make_current();
#endif

  if ( width == 0 || height == 0 || ( this->width_ == width && this->height_ == height ) )
  {
    return;
  }

  {
    Utils::RenderResources::lock_type lock( Utils::RenderResources::GetMutex() );
    this->textures_[ 0 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
    this->textures_[ 1 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
    this->textures_[ 0 ]->set_image( width, height, GL_RGBA );
    this->textures_[ 1 ]->set_image( width, height, GL_RGBA );
    this->depth_buffer_->set_storage( width, height, GL_DEPTH_COMPONENT );
  }

  this->width_ = width;
  this->height_ = height;

  glViewport( 0, 0, this->width_, this->height_ );

  {
    lock_type lock( this->redraw_needed_mutex_ );
    this->redraw_needed_ = true;
  }

  // Getting here means the size is valid, make sure that the renderer is active
  this->activate();
  this->redraw();
}

void Renderer::process_slices( LayerSceneHandle& layer_scene, ViewerHandle& viewer )
{
  for ( size_t layer_num = 0; layer_num < layer_scene->size(); layer_num++ )
  {
    LayerSceneItemHandle layer_item = ( *layer_scene )[ layer_num ];
    switch ( layer_item->type() )
    {
    case Utils::VolumeType::DATA_E:
      {
        DataLayerSceneItem* data_layer_item = 
          dynamic_cast< DataLayerSceneItem* >( layer_item.get() );
        Utils::DataVolumeSliceHandle data_volume_slice = 
          viewer->get_data_volume_slice( layer_item->layer_id_ );
        if ( data_volume_slice && !data_volume_slice->out_of_boundary() )
        {
          data_volume_slice->initialize_texture();
          data_volume_slice->upload_texture();
          data_layer_item->data_volume_slice_ = 
            Utils::DataVolumeSliceHandle( new Utils::DataVolumeSlice( *data_volume_slice ) );
        }
        else
        {
          layer_scene->erase( layer_scene->begin() + layer_num );
          layer_num--;
        }
      }
      break;
    case Utils::VolumeType::MASK_E:
      {
        MaskLayerSceneItem* mask_layer_item = 
          dynamic_cast< MaskLayerSceneItem* >( layer_item.get() );
        Utils::MaskVolumeSliceHandle mask_volume_slice = 
          viewer->get_mask_volume_slice( layer_item->layer_id_ );
        if ( mask_volume_slice && !mask_volume_slice->out_of_boundary() )
        {
          mask_volume_slice->initialize_texture();
          mask_volume_slice->upload_texture();
          mask_layer_item->mask_volume_slice_ = 
            Utils::MaskVolumeSliceHandle( new Utils::MaskVolumeSlice( *mask_volume_slice ) );
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

} // end namespace Seg3D


