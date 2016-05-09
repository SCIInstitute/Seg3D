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

#ifndef CORE_RENDERER_RENDERERBASE_H
#define CORE_RENDERER_RENDERERBASE_H

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Core/EventHandler/EventHandler.h>
#include <Core/Renderer/AbstractRenderer.h>
#include <Core/Renderer/PickPoint.h>

namespace Core
{

// Forward declarations
class RendererBase;
typedef boost::shared_ptr< RendererBase > RendererBaseHandle;
class RendererBasePrivate;
typedef boost::shared_ptr< RendererBasePrivate > RendererBasePrivateHandle;

// Class definitions
class RendererBase : public AbstractRenderer, private EventHandler
{
  // -- constructor/destructor --
public:
  RendererBase();
  virtual ~RendererBase();

public:
  
  // INITIALIZE:
  /// Initialize the renderer.
  virtual void initialize();

  // RESIZE:
  /// resize the renderer.
  virtual void resize( int width, int height );

  // REDRAW_SCENE:
  /// Calls the "render" function to render into the FBO.
  /// It triggers the "redraw_completed_signal_" at the end.
  virtual void redraw_scene();

  // REDRAW_OVERLAY:
  /// Calls the "render_overlay" function to render the overlay into the FBO.
  /// It triggers the "redraw_overlay_completed_signal_" at the end.
  virtual void redraw_overlay();

  // REDRAW_ALL:
  /// Calls the "render" and "render_overlay" function to render everything into the FBO.
  /// It triggers the "redraw_overlay_completed_signal_" at the end.
  virtual void redraw_all();

  // VOLUME_PICK_POINT_SIGNAL_:
  /// Triggered when user "picks" in volume view.   
  typedef boost::signals2::signal< void( Point world_pick_point ) >
    volume_pick_point_signal_type;
  volume_pick_point_signal_type volume_pick_point_signal_;

protected:
  friend class RendererBasePrivate;

  /// Activate the renderer
  virtual void activate();

  /// Deactivate the renderer
  virtual void deactivate();

  /// Return the status of the renderer
  virtual bool is_active();

  // POST_INITIALIZE:
  /// Called at the end of "initialize". The default implementation does not do anything.
  virtual void post_initialize();

  // POST_RESIZE:
  /// Called at the end of "resize". The default implementation does not do anything.
  virtual void post_resize();

  // RENDER:
  /// Re-implement this function to do the real rendering.
  /// The default implementation renders a black scene.
  /// This function should return true if the rendering was successful, or false if there
  // were errors or the rendering was interrupted.
  virtual bool render_scene();

  // RENDER_OVERLAY:
  /// Re-implement this function to render the overlay.
  /// The default implementation renders a black scene.
  /// This function should return true if the rendering was successful, or false if there
  /// were errors or the rendering was interrupted.
  virtual bool render_overlay();

  // REDRAW_SCENE:
  /// If not picking (empty pick_point handle):
  /// - Calls the "render" function to render into the FBO.
  /// - It triggers the "redraw_completed_signal_" at the end.
  /// Else if picking:
  /// - Find 3D pick point
  /// - It triggers the volume_pick_point_signal_" at the end.
  void redraw_scene( PickPointHandle pick_point );

  // REDRAW_NEEDED:
  /// Returns true if there is redraw pending, otherwise false.
  bool redraw_needed();

  // SET_REDRAW_NEEDED:
  /// Set the redraw flag to the given value.
  void set_redraw_needed( bool needed = true );

  // REDRAW_OVERLAY_NEEDED:
  /// Returns true if there is redraw_overlay pending, otherwise false.
  bool redraw_overlay_needed();

  // SET_REDRAW_OVERLAY_NEEDED:
  /// Set the redraw_overlay flag to the given value.
  void set_redraw_overlay_needed( bool needed = true );

  // IS_RENDERER_THREAD:
  /// Returns true if the calling thread is the renderer thread, otherwise false.
  bool is_renderer_thread();

  // POST_RENDERER_EVENT:
  /// Post an event to the renderer thread.
  void post_renderer_event( boost::function< void () > event );

protected:

  int width_;
  int height_;

private:
  RendererBasePrivateHandle private_;
};

} // end namespace Core

#endif
