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

#ifndef CORE_RENDERERBASE_RENDERERBASE_H
#define CORE_RENDERERBASE_RENDERERBASE_H

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Core/EventHandler/EventHandler.h>
#include <Core/Graphics/FramebufferObject.h>
#include <Core/Graphics/Renderbuffer.h>
#include <Core/Graphics/Texture.h>
#include <Core/RenderResources/RenderContext.h>
#include <Core/RendererBase/AbstractRenderer.h>

namespace Core
{

// Forward declarations
class RendererBase;

// Class definitions
class RendererBase : public AbstractRenderer, protected Core::EventHandler
{

  // -- constructor/destructor --
public:
  RendererBase();
  virtual ~RendererBase();

public:
  
  // INITIALIZE:
  // Initialize the renderer.
  virtual void initialize();

  // RESIZE:
  // resize the renderer.
  virtual void resize( int width, int height );

  // REDRAW:
  // Calls the "render" function to render onto the FBO.
  // It triggers the "redraw_completed_signal_" at the end.
  // "delay_update" indicates whether the UI should delay the update on receiving
  // the new texture.
  virtual void redraw( bool delay_update = false );

  // REDRAW_OVERLAY:
  // Calls the "render_overlay" function to render the overlay onto the FBO.
  // It triggers the "redraw_overlay_completed_signal_" at the end.
  virtual void redraw_overlay( bool delay_update = false );

  // Activate the renderer
  virtual void activate() 
  { 
    lock_type lock( this->get_mutex() );
    this->active_ = true; 
  }

  // Deactivate the renderer
  virtual void deactivate() 
  {
    lock_type lock( this->get_mutex() );
    this->active_ = false; 
  }

  // Return the status of the renderer
  virtual bool is_active() 
  { 
    lock_type lock( this->get_mutex() );
    return this->active_; 
  }

protected:

  // POST_INITIALIZE:
  // Called at the end of "initialize". The default implementation does not do anything.
  virtual void post_initialize();

  // POST_RESIZE:
  // Called at the end of "resize". The default implementation does not do anything.
  virtual void post_resize();

  // RENDER:
  // Re-implement this function to do the real rendering.
  // The default implementation renders a black scene.
  // This function should return true if the rendering was successful, or false if there
  // were errors or the rendering was interrupted.
  virtual bool render();

  // RENDER_OVERLAY:
  // Re-implement this function to render the overlay.
  // The default implementation renders a black scene.
  // This function should return true if the rendering was successful, or false if there
  // were errors or the rendering was interrupted.
  virtual bool render_overlay();

  bool redraw_needed()
  {
    lock_type lock( this->get_mutex() );
    return this->redraw_needed_;
  }

  void set_redraw_needed()
  {
    lock_type lock( this->get_mutex() );
    this->redraw_needed_ = true;
  }

  bool redraw_overlay_needed()
  {
    lock_type lock( this->get_mutex() );
    return this->redraw_overlay_needed_;
  }

  void set_redraw_overlay_needed()
  {
    lock_type lock( this->get_mutex() );
    this->redraw_overlay_needed_ = true;
  }

  typedef boost::mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  mutex_type& get_mutex()
  {
    return this->mutex_;
  }

protected:

  int width_;
  int height_;

private:

  // GL context for rendering
  Core::RenderContextHandle context_;

  Core::Texture2DHandle textures_[ 4 ];
  Core::RenderbufferHandle depth_buffer_;
  Core::FramebufferObjectHandle frame_buffer_;

  int active_render_texture_;
  int active_overlay_texture_;

  bool redraw_needed_;
  bool redraw_overlay_needed_;

  bool active_;

  mutex_type mutex_;
};

} // end namespace Core

#endif
