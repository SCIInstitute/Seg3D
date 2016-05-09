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

#ifndef CORE_RENDERER_ABSTRACTRENDERER_H
#define CORE_RENDERER_ABSTRACTRENDERER_H

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

// Core includes
#include <Core/Graphics/Texture.h>

namespace Core
{

// Forward declaration
class AbstractRenderer;
typedef boost::shared_ptr< AbstractRenderer> AbstractRendererHandle;

class AbstractRenderer
{
public:
  AbstractRenderer() {}
  virtual ~AbstractRenderer() {}

public:
  // INITIALIZE:
  /// Initialize the renderer.
  virtual void initialize() = 0;

  // RESIZE:
  /// resize the renderer.
  virtual void resize( int width, int height ) = 0;

  // REDRAW_SCENE:
  /// It triggers the "redraw_completed_signal_" at the end.
  virtual void redraw_scene() = 0;

  // REDRAW_OVERLAY:
  /// It triggers the "redraw_overlay_completed_signal_" at the end.
  virtual void redraw_overlay() = 0;

  // REDRAW_ALL:
  /// It triggers the "redraw_overlay_completed_signal_" and
  /// "redraw_completed_signal_" at the end.
  virtual void redraw_all() = 0;

  // REDRAW_COMPLETED_SIGNAL_
  /// Triggered when redraw is done. The first parameter is a handle to the texture
  /// containing the redraw result, the second indicates whether the update should be
  /// delayed, such as when there will be another texture coming after this signal.
  typedef boost::signals2::signal< void( Core::Texture2DHandle, bool ) > 
    redraw_completed_signal_type;
  redraw_completed_signal_type redraw_completed_signal_;

  // REDRAW_OVERLAY_COMPLETED_SIGNAL_
  /// Triggered when redraw_overlay is done.
  /// The parameter is a handle to the texture containing the overlay.
  redraw_completed_signal_type redraw_overlay_completed_signal_;

protected:
  friend class AbstractViewer;
  friend class AbstractViewerPrivate;

  // ACTIVATE:
  /// Activate the renderer
  virtual void activate() = 0;

  // DEACTIVATE:
  /// Deactivate the renderer
  virtual void deactivate() = 0;  

  // IS_ACTIVE:
  /// Return the status of the renderer
  virtual bool is_active() = 0;

};

} // end namespace Core

#endif
