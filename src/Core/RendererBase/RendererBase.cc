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
#include <Core/Utils/Log.h>
#include <Core/RenderResources/RenderResources.h>

// Application includes
#include <Core/Interface/Interface.h>
#include <Core/RendererBase/RendererBase.h>


#if defined(_WIN32) || defined(__APPLE__) || defined(X11_THREADSAFE)
#define MULTITHREADED_RENDERING 1
#else
#define MULTITHREADED_RENDERING 0
#endif

namespace Core
{

RendererBase::RendererBase() :
  EventHandler(), 
  width_( 0 ), 
  height_( 0 ),
  active_render_texture_( 0 ), 
  active_overlay_texture_( 2 ),
  redraw_needed_( false ),
  redraw_overlay_needed_( false ),
  active_( false )
{
  if ( !Core::RenderResources::Instance()->create_render_context( this->context_ ) )
  {
    CORE_THROW_EXCEPTION( "Failed to create a valid rendering context" );
  }
#if MULTITHREADED_RENDERING
  this->start_eventhandler();
#endif
}

RendererBase::~RendererBase()
{
}

void RendererBase::initialize()
{
#if MULTITHREADED_RENDERING
  // NOTE: it is important to postpone the allocation of OpenGL objects to the 
  // rendering thread. If created in a different thread, these objects might not
  // be ready when the rendering thread uses them the first time, which caused
  // the scene to be blank sometimes.
  if ( !is_eventhandler_thread() )
  {
    this->post_event( boost::bind( &RendererBase::initialize, this ) );
    return;
  }

  CORE_LOG_DEBUG( "Initializing renderer in a separate thread" );
#else
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &RendererBase::initialize, this ) );
    return;
  }

  CORE_LOG_DEBUG( "Initializing renderer in the interface thread" );
  
  // Save old GL context so it can be restored at the end
  RenderContextHandle old_context = RenderResources::Instance()->get_current_context();
#endif

  // Make the GL context current. In multi-threaded rendering mode,
  // this call is only needed once.
  this->context_->make_current();

  {
    // lock the shared render context
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );

    this->depth_buffer_ = Core::RenderbufferHandle( new Core::Renderbuffer );
    this->frame_buffer_ = Core::FramebufferObjectHandle( new Core::FramebufferObject );
    this->frame_buffer_->attach_renderbuffer( this->depth_buffer_, GL_DEPTH_ATTACHMENT_EXT );
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
#if MULTITHREADED_RENDERING
  if ( !is_eventhandler_thread() )
  {
    lock_type lock( this->get_mutex() );
    this->redraw_needed_ = true;
    this->post_event( boost::bind( &RendererBase::redraw, this, delay_update ) );
    return;
  }
#else
  if ( !Interface::IsInterfaceThread() )
  {
    lock_type lock( this->get_mutex() );
    this->redraw_needed_ = true;
    Interface::PostEvent( boost::bind( &RendererBase::redraw, this, delay_update ) );
    return;
  }
  RenderContextHandle old_context = RenderResources::Instance()->get_current_context();
  // Make the GL context current in the interface thread
  this->context_->make_current();
#endif

  if ( !this->is_active() || this->width_ == 0 || this->height_ == 0 )
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

  if ( !this->frame_buffer_->check_status() )
  {
    this->frame_buffer_->disable();
    return;
  }

  if ( !this->render() )
  {
    return;
  }
  
  // Synchronization calln for multi threaded rendering
  glFinish();

  CORE_CHECK_OPENGL_ERROR();

  this->frame_buffer_->detach_texture( this->textures_[ this->active_render_texture_ ] );
  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();

  // signal rendering completed
  this->redraw_completed_signal_( 
    this->textures_[ this->active_render_texture_ ], delay_update );

  // swap render textures 
  this->active_render_texture_ = ( ~this->active_render_texture_ ) & 1;

#if !MULTITHREADED_RENDERING
  if ( old_context )
  {
    old_context->make_current();
  }
#endif
}

void RendererBase::redraw_overlay( bool delay_update )
{
#if MULTITHREADED_RENDERING
  if ( !is_eventhandler_thread() )
  {
    lock_type lock( this->get_mutex() );
    this->redraw_overlay_needed_ = true;
    this->post_event( boost::bind( &RendererBase::redraw_overlay, this, delay_update ) );
    return;
  }
#else
  if ( !Interface::IsInterfaceThread() )
  {
    lock_type lock( this->get_mutex() );
    this->redraw_overlay_needed_ = true;
    Interface::PostEvent( boost::bind( &RendererBase::redraw_overlay, this, delay_update ) );
    return;
  }
  RenderContextHandle old_context = RenderResources::Instance()->get_current_context();
  // Make the GL context current in the interface thread
  this->context_->make_current();
#endif

  if ( !this->is_active() || this->width_ == 0 || this->height_ == 0 )
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
  Core::Texture::lock_type texture_lock( this->textures_[ this->active_overlay_texture_ ]->get_mutex() );

  // bind the framebuffer object
  this->frame_buffer_->enable();
  // attach texture
  this->frame_buffer_->attach_texture( this->textures_[ this->active_overlay_texture_ ] );

  if ( !this->frame_buffer_->check_status() )
  {
    this->frame_buffer_->disable();
    return;
  }

  if ( !this->render_overlay() )
  {
    return;
  }
  glFinish();

  CORE_CHECK_OPENGL_ERROR();

  this->frame_buffer_->detach_texture( this->textures_[ this->active_overlay_texture_ ] );
  this->frame_buffer_->disable();

  // release the lock on the active render texture
  texture_lock.unlock();

  // signal rendering completed
  this->redraw_overlay_completed_signal_( 
    this->textures_[ this->active_overlay_texture_ ], delay_update );

  // swap render textures 
  this->active_overlay_texture_ = ( ~( this->active_overlay_texture_ - 2 ) ) & 1 + 2;

#if !MULTITHREADED_RENDERING
  if ( old_context )
  {
    old_context->make_current();
  }
#endif
}

void RendererBase::resize( int width, int height )
{
#if MULTITHREADED_RENDERING
  if ( !is_eventhandler_thread() )
  {
    this->post_event( boost::bind( &RendererBase::resize, this, width, height ) );
    return;
  }
#else
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &RendererBase::resize, this, width, height ) );
    return;
  }
  RenderContextHandle old_context = RenderResources::Instance()->get_current_context();
  // Make the GL context current in the interface thread
  this->context_->make_current();
#endif

  if ( width <= 0 || height <= 0 || ( this->width_ == width && this->height_ == height ) )
  {
    return;
  }

  {
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
    this->textures_[ 0 ] = Core::Texture2DHandle( new Core::Texture2D );
    this->textures_[ 1 ] = Core::Texture2DHandle( new Core::Texture2D );
    this->textures_[ 2 ] = Core::Texture2DHandle( new Core::Texture2D );
    this->textures_[ 3 ] = Core::Texture2DHandle( new Core::Texture2D );
    this->textures_[ 0 ]->set_image( width, height, GL_RGBA );
    this->textures_[ 1 ]->set_image( width, height, GL_RGBA );
    this->textures_[ 2 ]->set_image( width, height, GL_RGBA );
    this->textures_[ 3 ]->set_image( width, height, GL_RGBA );
    this->depth_buffer_->set_storage( width, height, GL_DEPTH_COMPONENT );
  }

  this->width_ = width;
  this->height_ = height;

  glViewport( 0, 0, this->width_, this->height_ );

  {
    lock_type lock( this->get_mutex() );
    this->redraw_needed_ = true;
    this->redraw_overlay_needed_ = true;
  }

  this->post_resize();

  this->redraw( true );
  this->redraw_overlay();

#if !MULTITHREADED_RENDERING
  if ( old_context )
  {
    old_context->make_current();
  }
#endif
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

} // end namespace Core


