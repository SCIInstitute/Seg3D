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

// Core includes
#include <Core/Graphics/FramebufferObject.h>
#include <Core/Graphics/Renderbuffer.h>
#include <Core/Graphics/Texture.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/RenderResources/RenderContext.h>
#include <Core/RenderResources/RenderContextBinding.h>
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>
#include <Core/Renderer/RendererBase.h>

#if defined(_WIN32) || defined(__APPLE__) || defined(X11_THREADSAFE)
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
  void redraw( bool delay_update );
  void redraw_overlay( bool delay_update );

  RendererBase * renderer_;

  Core::RenderContextHandle context_;
  Core::Texture2DHandle textures_[ 4 ];
  Core::RenderbufferHandle depth_buffer_;
  Core::FramebufferObjectHandle frame_buffer_;

  int active_render_texture_;
  int active_overlay_texture_;

  bool redraw_needed_;
  bool redraw_overlay_needed_;
  bool active_;
};

void RendererBasePrivate::redraw( bool delay_update )
{
  if ( !this->renderer_->is_renderer_thread() )
  {
    this->renderer_->post_renderer_event( boost::bind( &RendererBasePrivate::redraw, 
      this, delay_update ) );
    return;
  }

#if !MULTITHREADED_RENDERING
  RenderContextBindingHandle context_binding( new RenderContextBinding( this->context_ ) );
#endif

  if ( !this->renderer_->is_active() || 
    this->renderer_->width_ == 0 || 
    this->renderer_->height_ == 0 )
  {
    return;
  }

  {
    lock_type lock( this->get_mutex() );
    if ( !this->redraw_needed_ )
    {
      return;
    }
    this->redraw_needed_ = false;
  }

  // lock the active render texture
  Core::Texture::lock_type texture_lock( this->textures_[ this->active_render_texture_ ]->get_mutex() );

  // bind the framebuffer object
  this->frame_buffer_->enable();
  // attach texture
  this->frame_buffer_->attach_texture( this->textures_[ this->active_render_texture_ ] );
  this->frame_buffer_->attach_renderbuffer( this->depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );

  if ( !this->frame_buffer_->check_status() )
  {
    this->frame_buffer_->disable();
    return;
  }

  if ( !this->renderer_->render() )
  {
    return;
  }

  // Synchronization call for multi threaded rendering
  glFinish();

  CORE_CHECK_OPENGL_ERROR();

  this->frame_buffer_->detach_texture( this->textures_[ this->active_render_texture_ ] );
  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();

#if !MULTITHREADED_RENDERING
  context_binding.reset();
#endif

  // signal rendering completed
  this->renderer_->redraw_completed_signal_( 
    this->textures_[ this->active_render_texture_ ], delay_update );

  // swap render textures 
  this->active_render_texture_ = ( ~this->active_render_texture_ ) & 1;
}

void RendererBasePrivate::redraw_overlay( bool delay_update )
{
  if ( !this->renderer_->is_renderer_thread() )
  {
    this->renderer_->post_renderer_event( boost::bind( 
      &RendererBasePrivate::redraw_overlay, this, delay_update ) );
    return;
  }

#if !MULTITHREADED_RENDERING
  RenderContextBindingHandle context_binding( new RenderContextBinding( this->context_ ) );
#endif

  if ( !this->renderer_->is_active() || 
    this->renderer_->width_ == 0 || 
    this->renderer_->height_ == 0 )
  {
    return;
  }

  {
    lock_type lock( this->get_mutex() );
    if ( !this->redraw_overlay_needed_ )
    {
      return;
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
  this->frame_buffer_->attach_renderbuffer( this->depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );

  if ( !this->frame_buffer_->check_status() )
  {
    this->frame_buffer_->disable();
    return;
  }

  if ( !this->renderer_->render_overlay() )
  {
    return;
  }
  glFinish();

  CORE_CHECK_OPENGL_ERROR();

  this->frame_buffer_->detach_texture( this->textures_[ this->active_overlay_texture_ ] );
  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();

#if !MULTITHREADED_RENDERING
  context_binding.reset();
#endif

  // signal rendering completed
  this->renderer_->redraw_overlay_completed_signal_( 
    this->textures_[ this->active_overlay_texture_ ], delay_update );

  // swap render textures 
  this->active_overlay_texture_ = ( ~( this->active_overlay_texture_ - 2 ) ) & 1 + 2;
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
  if ( !Core::RenderResources::Instance()->create_render_context( this->private_->context_ ) )
  {
    CORE_THROW_EXCEPTION( "Failed to create a valid rendering context" );
  }
  this->private_->renderer_ = this;
  this->private_->active_render_texture_ = 0;
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

  // Make the GL context current. In multi-threaded rendering mode,
  // this call is only needed once.
  this->private_->context_->make_current();

  {
    // lock the shared render context
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );

    this->private_->depth_buffer_.reset( new Core::Renderbuffer );
    this->private_->frame_buffer_.reset( new Core::FramebufferObject );
    this->private_->frame_buffer_->attach_renderbuffer( 
      this->private_->depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );
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

void RendererBase::redraw( bool delay_update )
{
  this->set_redraw_needed();
  this->private_->redraw( delay_update );
}

void RendererBase::redraw_overlay( bool delay_update )
{
  this->set_redraw_overlay_needed();
  this->private_->redraw_overlay( delay_update );
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
    this->private_->depth_buffer_->set_storage( width, height, GL_DEPTH_COMPONENT );
  }

  this->width_ = width;
  this->height_ = height;

  glViewport( 0, 0, this->width_, this->height_ );

  this->post_resize();

#if !MULTITHREADED_RENDERING
  context_binding.reset();
#endif

  this->redraw( true );
  this->redraw_overlay();
}

void RendererBase::post_initialize()
{
}

void RendererBase::post_resize()
{
}

bool RendererBase::render()
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


