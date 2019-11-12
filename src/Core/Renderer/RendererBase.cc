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

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Graphics/FramebufferObject.h>
#include <Core/Graphics/Renderbuffer.h>
#include <Core/Graphics/Texture.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/RenderResources/RenderContext.h>
#include <Core/RenderResources/RenderContextBinding.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/Interface/Interface.h>
#include <Core/Renderer/RendererBase.h>

#if defined(__APPLE__) || defined(X11_THREADSAFE) // Windows isn't considered threadsafe for OpenGL
#define MULTITHREADED_RENDERING 1
#else
#define MULTITHREADED_RENDERING 0
#endif

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Implementation of class RendererBasePrivate
//////////////////////////////////////////////////////////////////////////

class RendererBasePrivate : public Lockable
{
public:
  bool render_scene( PickPointHandle pick_point );
  bool render_overlay();

  void redraw_scene( PickPointHandle pick_point );
  void redraw_overlay();
  void redraw_all();

  RendererBase * renderer_;

  Core::RenderContextHandle context_;
  Core::Texture2DHandle textures_[ 4 ];
  Core::RenderbufferHandle scene_depth_buffer_;
  Core::RenderbufferHandle overlay_depth_buffer_;
  Core::FramebufferObjectHandle frame_buffer_;

  int active_scene_texture_;
  int active_overlay_texture_;

  bool redraw_needed_;
  bool redraw_overlay_needed_;
  bool active_;
};


void RendererBasePrivate::redraw_scene( PickPointHandle pick_point )
{
  // Migrate to the right thread
  if ( !this->renderer_->is_renderer_thread() )
  {
    this->renderer_->post_renderer_event( boost::bind( &RendererBasePrivate::redraw_scene,
      this, pick_point ) );
    return;
  }

  if ( this->render_scene( pick_point ) )
  {
    if( !pick_point )
    {
      // signal rendering completed
      this->renderer_->redraw_completed_signal_(
        this->textures_[ this->active_scene_texture_ ], false );

      // swap render textures
      this->active_scene_texture_ = ( ~this->active_scene_texture_ ) & 1;
    }
  }
}

void RendererBasePrivate::redraw_overlay()
{
  // Migrate to the right thread
  if ( !this->renderer_->is_renderer_thread() )
  {
    this->renderer_->post_renderer_event( boost::bind( &RendererBasePrivate::redraw_overlay,
      this) );
    return;
  }

  if ( this->render_overlay() )
  {
    // signal rendering completed
    this->renderer_->redraw_overlay_completed_signal_(
      this->textures_[ this->active_overlay_texture_ ], false );

    // swap render textures
    this->active_overlay_texture_ = ( ~( this->active_overlay_texture_ - 2 ) ) & 1 + 2;
  }
}

void RendererBasePrivate::redraw_all()
{
  // Migrate to the right thread
  if ( !this->renderer_->is_renderer_thread() )
  {
    this->renderer_->post_renderer_event( boost::bind( &RendererBasePrivate::redraw_all,
      this) );
    return;
  }

  bool render_scene_success = this->render_scene( PickPointHandle() );
  bool render_overlay_success = this->render_overlay();

  if ( render_scene_success )
  {
    // signal scene rendering completed
    this->renderer_->redraw_completed_signal_(
      this->textures_[ this->active_scene_texture_ ], render_overlay_success );

    // swap render textures
    this->active_scene_texture_ = ( ~this->active_scene_texture_ ) & 1;
  }

  if ( render_overlay_success )
  {
    // signal overlay rendering completed
    this->renderer_->redraw_overlay_completed_signal_(
      this->textures_[ this->active_overlay_texture_ ], false );

    // swap render textures
    this->active_overlay_texture_ = ( ~( this->active_overlay_texture_ - 2 ) ) & 1 + 2;
  }
}

bool RendererBasePrivate::render_scene( PickPointHandle pick_point )
{

#if !MULTITHREADED_RENDERING
  RenderContextBindingHandle context_binding( new RenderContextBinding( this->context_ ) );
#endif

  if ( !this->renderer_->is_active() ||
    this->renderer_->width_ == 0 ||
    this->renderer_->height_ == 0 )
  {
    return false;
  }

  {
    lock_type lock( this->get_mutex() );
    if ( !this->redraw_needed_ )
    {
      return false;
    }
    this->redraw_needed_ = false;
  }

  // lock the active render texture
  Core::Texture::lock_type texture_lock( this->textures_[ this->active_scene_texture_ ]->get_mutex() );

  // bind the framebuffer object
  this->frame_buffer_->enable();

  // attach texture
  this->frame_buffer_->attach_texture( this->textures_[ this->active_scene_texture_ ] );
  this->frame_buffer_->attach_renderbuffer( this->scene_depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );

  if ( !this->frame_buffer_->check_status() )
  {
    this->frame_buffer_->disable();
    return false;
  }

  // 3D picking
  if( pick_point )
  {
    GLint viewport[ 4 ];
    GLdouble modelview[ 16 ];
    GLdouble projection[ 16 ];
    GLfloat win_x, win_y, win_z;
    GLdouble pos_x, pos_y, pos_z;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    win_x = static_cast< float >( pick_point->window_x_ );
    win_y = static_cast< float >(
      static_cast< float >( viewport[ 3 ] ) - static_cast< float >( pick_point->window_y_ ) );
    // Get the depth value from the last render pass at the pick point
    // FBO is already bound
    glReadPixels( pick_point->window_x_, static_cast< int >( win_y ), 1, 1, GL_DEPTH_COMPONENT,
      GL_FLOAT, &win_z );

    // If z isn't on near or far plane
    if( 0.0 < win_z && win_z < 1.0 )
    {
      // Convert window coordinates + depth into a 3D world coordinate
      gluUnProject( win_x, win_y, win_z, modelview, projection, viewport, &pos_x, &pos_y, &pos_z );

      Point world_pick_point( pos_x, pos_y, pos_z );

      // Emit signal with picked point
      this->renderer_->volume_pick_point_signal_( world_pick_point );
    }
  }
  else
  {
    if ( !this->renderer_->render_scene() )
    {
      CORE_LOG_DEBUG( "Rendering of the Scene failed" );
      return false;
    }

    // Synchronization call for multi threaded rendering
    glFinish();
  }

  CORE_CHECK_OPENGL_ERROR();

  this->frame_buffer_->detach_texture( this->textures_[ this->active_scene_texture_ ] );
  this->frame_buffer_->detach_renderbuffer( this->scene_depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );
  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();

#if !MULTITHREADED_RENDERING
  context_binding.reset();
#endif

  return true;
}

bool RendererBasePrivate::render_overlay()
{

#if !MULTITHREADED_RENDERING
  RenderContextBindingHandle context_binding( new RenderContextBinding( this->context_ ) );
#endif

  if ( !this->renderer_->is_active() ||
    this->renderer_->width_ == 0 ||
    this->renderer_->height_ == 0 )
  {
    return false;
  }

  {
    lock_type lock( this->get_mutex() );
    if ( !this->redraw_overlay_needed_ )
    {
      return false;
    }
    this->redraw_overlay_needed_ = false;
  }

  // lock the active render texture
  Core::Texture::lock_type texture_lock(
    this->textures_[ this->active_overlay_texture_ ]->get_mutex() );

  // bind the framebuffer object
  this->frame_buffer_->enable();
  // attach texture
  this->frame_buffer_->attach_texture( this->textures_[ this->active_overlay_texture_ ] );
  this->frame_buffer_->attach_renderbuffer( this->overlay_depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );

  if ( !this->frame_buffer_->check_status() )
  {
    this->frame_buffer_->disable();
    return false;
  }

  if ( !this->renderer_->render_overlay() )
  {
    CORE_LOG_DEBUG( "Rendering of the Overlay failed" );
    return false;
  }
  glFinish();

  CORE_CHECK_OPENGL_ERROR();

  this->frame_buffer_->detach_texture( this->textures_[ this->active_overlay_texture_ ] );
  this->frame_buffer_->detach_renderbuffer( this->overlay_depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );
  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();

#if !MULTITHREADED_RENDERING
  context_binding.reset();
#endif

  return true;
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class RendererBase
//////////////////////////////////////////////////////////////////////////

RendererBase::RendererBase() :
  EventHandler(),
  width_( 0 ),
  height_( 0 ),
  private_( new RendererBasePrivate )
{
  this->private_->renderer_ = this;
  this->private_->active_scene_texture_ = 0;
  this->private_->active_overlay_texture_ = 2;
  this->private_->redraw_needed_ = false;
  this->private_->redraw_overlay_needed_ =  false;
  this->private_->active_ = false;

#if MULTITHREADED_RENDERING
  this->start_eventhandler();
#endif
}

RendererBase::~RendererBase()
{
}

void RendererBase::initialize()
{
  // NOTE: it is important to postpone the allocation of OpenGL objects to the
  // rendering thread. If created in a different thread, these objects might not
  // be ready when the rendering thread uses them the first time, which caused
  // the scene to be blank sometimes.

  if ( !this->is_renderer_thread() )
  {
    this->post_renderer_event( boost::bind( &RendererBase::initialize, this ) );
    return;
  }

#if MULTITHREADED_RENDERING
  CORE_LOG_DEBUG( "Initializing renderer in a separate thread" );
#else
  CORE_LOG_DEBUG( "Initializing renderer in the interface thread" );
  // Save old GL context so it can be restored at the end
  RenderContextHandle old_context = RenderResources::Instance()->get_current_context();
#endif

  if ( !Core::RenderResources::Instance()->create_render_context( this->private_->context_ ) )
  {
    CORE_THROW_EXCEPTION( "Failed to create a valid rendering context" );
  }

  // Make the GL context current. In multi-threaded rendering mode,
  // this call is only needed once.
  this->private_->context_->make_current();

  {
    // lock the shared render context
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );

    this->private_->scene_depth_buffer_.reset( new Core::Renderbuffer );
    this->private_->overlay_depth_buffer_.reset( new Core::Renderbuffer );
    this->private_->frame_buffer_.reset( new Core::FramebufferObject );
  }

  this->post_initialize();

  CORE_CHECK_OPENGL_ERROR();

#if !MULTITHREADED_RENDERING
  if ( old_context )
  {
    old_context->make_current();
  }
#endif
}

void RendererBase::redraw_scene()
{
  this->redraw_scene( PickPointHandle() );
}

void RendererBase::redraw_overlay()
{
  this->set_redraw_overlay_needed();

  this->private_->redraw_overlay();
}

void RendererBase::redraw_all()
{
  this->set_redraw_overlay_needed();
  this->set_redraw_needed();

  this->private_->redraw_all();
}

void RendererBase::resize( int width, int height )
{
  if ( !this->is_renderer_thread() )
  {
    this->post_renderer_event( boost::bind( &RendererBase::resize, this, width, height ) );
    return;
  }

  if ( width <= 0 || height <= 0 || ( this->width_ == width && this->height_ == height ) )
  {
    return;
  }

#if !MULTITHREADED_RENDERING
  RenderContextBindingHandle context_binding( new RenderContextBinding( this->private_->context_ ) );
#endif

  {
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
    this->private_->textures_[ 0 ].reset( new Core::Texture2D );
    this->private_->textures_[ 1 ].reset( new Core::Texture2D );
    this->private_->textures_[ 2 ].reset( new Core::Texture2D );
    this->private_->textures_[ 3 ].reset( new Core::Texture2D );
    this->private_->textures_[ 0 ]->set_image( width, height, GL_RGBA );
    this->private_->textures_[ 1 ]->set_image( width, height, GL_RGBA );
    this->private_->textures_[ 2 ]->set_image( width, height, GL_RGBA );
    this->private_->textures_[ 3 ]->set_image( width, height, GL_RGBA );
    this->private_->scene_depth_buffer_->set_storage( width, height, GL_DEPTH_COMPONENT24 );
    this->private_->overlay_depth_buffer_->set_storage( width, height, GL_DEPTH_COMPONENT24 );
  }

  this->width_ = width;
  this->height_ = height;

  glViewport( 0, 0, this->width_, this->height_ );

  this->post_resize();

#if !MULTITHREADED_RENDERING
  context_binding.reset();
#endif

  this->redraw_all();
}

void RendererBase::post_initialize()
{
}

void RendererBase::post_resize()
{
}

bool RendererBase::render_scene()
{
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClear( GL_COLOR_BUFFER_BIT );
  return true;
}

bool RendererBase::render_overlay()
{
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClear( GL_COLOR_BUFFER_BIT );
  return true;
}

void RendererBase::redraw_scene( PickPointHandle pick_point )
{
  this->set_redraw_needed();

  this->private_->redraw_scene( pick_point );
}

bool RendererBase::redraw_needed()
{
  RendererBasePrivate::lock_type lock( this->private_->get_mutex() );
  return this->private_->redraw_needed_;
}

void RendererBase::set_redraw_needed( bool needed )
{
  RendererBasePrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->redraw_needed_ = needed;
}

bool RendererBase::redraw_overlay_needed()
{
  RendererBasePrivate::lock_type lock( this->private_->get_mutex() );
  return this->private_->redraw_overlay_needed_;
}

void RendererBase::set_redraw_overlay_needed( bool needed )
{
  RendererBasePrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->redraw_overlay_needed_ = needed;
}

void RendererBase::activate()
{
  RendererBasePrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->active_ = true;
}

void RendererBase::deactivate()
{
  RendererBasePrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->active_ = false;
}

bool RendererBase::is_active()
{
  RendererBasePrivate::lock_type lock( this->private_->get_mutex() );
  return this->private_->active_;
}

bool RendererBase::is_renderer_thread()
{
#if MULTITHREADED_RENDERING
  return this->is_eventhandler_thread();
#else
  return Interface::IsInterfaceThread();
#endif
}

void RendererBase::post_renderer_event( boost::function< void () > event )
{
#if MULTITHREADED_RENDERING
  this->post_event( event );
#else
  Interface::PostEvent( event );
#endif
}

} // end namespace Core
