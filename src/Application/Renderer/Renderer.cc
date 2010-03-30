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
#if defined(WIN32) || defined(APPLE) || defined(X11_THREADSAFE)
#ifdef X11_THREADSAFE
  SCI_LOG_DEBUG(std::string("Multithreaded rendering enabled on X-Window"));
#endif
  // NOTE: it is important to postpone the allocation of OpenGL objects to the 
  // rendering thread. If created in a different thread, these objects might not
  // be ready when the rendering thread uses them the first time, which caused
  // the scene to be blank sometimes.
  if ( !is_eventhandler_thread() )
  {
    if ( !Utils::RenderResources::Instance()->create_render_context( context_ ) )
    {
      SCI_THROW_EXCEPTION( "Failed to create a valid rendering context" );
    }
    post_event( boost::bind( &Renderer::initialize, this ) );
    start_eventhandler();
    return;
  }
#else
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &Renderer::initialize, this ) );
    return;
  }
  if ( !Utils::RenderResources::Instance()->create_render_context( context_ ) )
  {
    SCI_THROW_EXCEPTION("Failed to create a valid rendering context");
  }
#endif

  SCI_LOG_DEBUG(std::string("Renderer ") + Utils::to_string(this->viewer_id_)
    + ": initializing");

  // Make the GL context current. Since it is the only context in the rendering
  // thread, this call is only needed once.
  this->context_->make_current();

  //glEnable(GL_CULL_FACE);

  // lock the shared render context
  Utils::RenderResources::lock_type lock( Utils::RenderResources::GetMutex() );

  textures_[ 0 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
  textures_[ 1 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
  depth_buffer_ = Utils::RenderbufferHandle( new Utils::Renderbuffer() );
  frame_buffer_ = Utils::FramebufferObjectHandle( new Utils::FramebufferObject() );
  frame_buffer_->attach_renderbuffer( depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );
  this->cube_ = Utils::UnitCubeHandle( new Utils::UnitCube() );
  this->slice_shader_->initialize();

  // release the lock
  lock.unlock();

  this->add_connection( ViewerManager::Instance()->get_viewer( this->viewer_id_ )
    ->redraw_signal_.connect( boost::bind( &Renderer::redraw, this ) ) );
}

void Renderer::redraw()
{
#if defined(WIN32) || defined(APPLE) || defined(X11_THREADSAFE)
  if ( !is_eventhandler_thread() )
  {
    boost::unique_lock<boost::recursive_mutex> lock( this->redraw_needed_mutex_ );

    this->redraw_needed_ = true;
    this->post_event( boost::bind( &Renderer::redraw, this ) );
    
    return;
  }
#else
  if ( !Interface::IsInterfaceThread() )
  {
    boost::unique_lock< boost::recursive_mutex > lock( redraw_needed_mutex_ );

    this->redraw_needed_ = true;
    Interface::PostEvent( boost::bind( &Renderer::redraw, this ) );
    return;
  }
#endif

  {
    boost::unique_lock< boost::recursive_mutex > lock( this->redraw_needed_mutex_ );
    if ( this->redraw_needed_ == false )
    {
      return;
    }
    this->redraw_needed_ = false;
  }

#if !defined(WIN32) && !defined(APPLE) && !defined(X11_THREADSAFE)
  this->context_->make_current();
#endif

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

  //glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
  SCI_CHECK_OPENGL_ERROR();

  glViewport( 0, 0, width_, height_ );
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  SCI_CHECK_OPENGL_ERROR();
  // do some rendering
  // ...
  // ...
  //glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_FRONT);

  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  // Lock the state engine
  StateEngine::lock_type state_lock( StateEngine::Instance()->get_mutex() );

  // Get a snapshot of current layers
  LayerSceneHandle layer_scene = LayerManager::Instance()->compose_layer_scene( this->viewer_id_ );

  if ( viewer->is_volume_view() )
  {
    Utils::View3D view3d( viewer->volume_view_state_->get() );

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

    state_lock.unlock();

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    double left, right, top, bottom;
    view2d.compute_clipping_planes( this->width_ / ( 1.0 * this->height_ ), 
      left, right, bottom, top );
    gluOrtho2D( left, right, bottom, top );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

/*
    glRotatef( 25.0f * ( this->viewer_id_ + 1 ), 1, 0, 1 );
    glScalef( 0.5f, 0.5f, 0.5f );
    glTranslatef( -0.5f, -0.5f, -0.5f );
    this->cube_->draw();
*/
    this->slice_shader_->enable();
    this->slice_shader_->set_texture( 0 );

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
          this->slice_shader_->set_contrast( static_cast< float >( data_layer_item->contrast_ ) );
          this->slice_shader_->set_brightness( 
            static_cast< float >( data_layer_item->brightness_ / 50 ) );
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

      this->slice_shader_->set_opacity( static_cast< float >( layer_item->opacity_ ) );
      Utils::TextureHandle slice_tex = volume_slice->get_texture();
      Utils::Texture::lock_type slice_tex_lock( slice_tex->get_mutex() );
      slice_tex->enable();
      glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
      glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex2d( volume_slice->left(), volume_slice->bottom() );
      glTexCoord2f( 1.0f, 0.0f );
      glVertex2d( volume_slice->right(), volume_slice->bottom() );
      glTexCoord2f( 1.0f, 1.0f );
      glVertex2d( volume_slice->right(), volume_slice->top() );
      glTexCoord2f( 0.0f, 1.0f );
      glVertex2d( volume_slice->left(), volume_slice->top() );
      glEnd();
      slice_tex->disable();

    } // end for

    this->slice_shader_->disable();
  }

  SCI_CHECK_OPENGL_ERROR();

  glFinish();

  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();


  // signal rendering completed
  this->rendering_completed_signal_( this->textures_[ this->active_render_texture_ ] );

  // swap render textures 
  this->active_render_texture_ = ( ~this->active_render_texture_ ) & 1;
}

void Renderer::resize( int width, int height )
{
#if defined(WIN32) || defined(APPLE) || defined(X11_THREADSAFE)
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
#endif

  if ( width == 0 || height == 0 || ( width_ == width && height_ == height ) )
  {
    return;
  }

  {
    Utils::RenderResources::lock_type lock( Utils::RenderResources::GetMutex() );
    textures_[ 0 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
    textures_[ 1 ] = Utils::Texture2DHandle( new Utils::Texture2D() );
    textures_[ 0 ]->set_image( width, height, GL_RGBA );
    textures_[ 1 ]->set_image( width, height, GL_RGBA );

    depth_buffer_->set_storage( width, height, GL_DEPTH_COMPONENT );
  }

  width_ = width;
  height_ = height;

  {
    boost::unique_lock< boost::recursive_mutex > lock( this->redraw_needed_mutex_ );
    this->redraw_needed_ = true;
  }


  redraw();
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


