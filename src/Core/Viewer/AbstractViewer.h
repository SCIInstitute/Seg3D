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

#ifndef APPLICATION_VIEWER_ABSTRACTVIEWER_H
#define APPLICATION_VIEWER_ABSTRACTVIEWER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Boost includes 
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/Renderer/AbstractRenderer.h>
#include <Core/State/State.h>
#include <Core/State/StateHandler.h>
#include <Core/Utils/EnumClass.h>
#include <Core/Utils/Lockable.h>
#include <Core/Viewer/Cursor.h>
#include <Core/Viewer/Mouse.h>

namespace Core
{

// Forward declarations
class AbstractViewer;
typedef boost::shared_ptr< AbstractViewer > AbstractViewerHandle;
typedef boost::weak_ptr< AbstractViewer > AbstractViewerWeakHandle;

class AbstractViewerPrivate;
typedef boost::shared_ptr< AbstractViewerPrivate > AbstractViewerPrivateHandle;

// Class definition
class AbstractViewer : public StateHandler, private Lockable
{

  // -- constructor/destructor --
public:
  AbstractViewer( size_t viewer_id );
  virtual ~AbstractViewer();

  // -- mouse events handling --
public:

  // MOUSE_MOVE_EVENT:
  /// This function is called by the render widget when a mouse move event has occurred.
  /// This function needs to be overloaded to record mouse movements.
  virtual void mouse_move_event( const MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
    
  // MOUSE_PRESS_EVENT:
  /// This function is called by the render widget when a mouse button press event has occurred.
  /// This function needs to be overloaded to record mouse buttons being pressed.
  virtual void mouse_press_event( const MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  
  // MOUSE_RELEASE_EVENT:
  /// This function is called by the render widget when a mouse button release event has occurred.
  /// This function needs to be overloaded to record mouse buttons being released.
  virtual void mouse_release_event( const MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  // MOUSE_ENTER_EVENT:
  /// This function is called by the render widget when a mouse enter event has occurred.
  virtual void mouse_enter_event( int x, int y );

  // MOUSE_LEAVE_EVENT:
  /// This function is called by the render widget when a mouse leave event has occurred.
  virtual void mouse_leave_event();
  
  // WHEEL_EVENT:
  /// This function is called by the render widget when a mouse wheel event has occurred.
  /// This function needs to be overloaded to track mouse wheel events. 
  virtual bool wheel_event( int delta, int x, int y, int buttons, int modifiers );

  // KEY_PRESS_EVENT:
  /// This function is called when a key is pressed while hovering over the render widget
  virtual bool key_press_event( int key, int modifiers, int x, int y );

  // KEY_RELEASE_EVENT:
  /// This function is called when a key is released while hovering over the render widget
  virtual bool key_release_event( int key, int modifiers, int x, int y );

  // RESIZE:
  /// This function is called when the viewer is resized by the render widget.
  virtual void resize( int width, int height );

  // INSTALL_RENDERER:
  /// Install a renderer to the viewer.
  virtual void install_renderer( AbstractRendererHandle renderer );

  // GET_VIEWER_ID:
  /// Get the id number of this viewer. All the viewers are number and can be identified by
  /// this number
  size_t get_viewer_id() const;

  // -- Signals and Slots --
public:
  /// Types of signals
  typedef boost::signals2::signal< void() > update_display_signal_type;
  typedef boost::signals2::signal< void() > redraw_signal_type;

  // UPDATE_DISPLAY_SIGNAL_:
  /// Triggered when new texture is received, or a different cursor shape is set.
  update_display_signal_type update_display_signal_;

  // REDRAW_SCENE_SIGNAL:
  /// Signals that the scene needs to be redrawn.
  redraw_signal_type redraw_scene_signal_;

  // REDRAW_OVERLAY_SIGNAL_:
  /// Signals that the overlay needs to be redrawn.
  redraw_signal_type redraw_overlay_signal_;

  // REDRAW_ALL_SIGNAL:
  /// Signals that the scene needs to be redrawn.
  redraw_signal_type redraw_all_signal_;

  // -- renderer/texture access --
public:
  // GET_TEXTURE:
  /// Returns the texture generated by the renderer
  Texture2DHandle get_texture();

  // GET_OVERLAY_TEXTURE:
  /// Returns the overlay texture generated by the renderer
  Texture2DHandle get_overlay_texture();

  // GET_WIDTH:
  /// Get the width of the viewer
  int get_width() const;

  // GET_HEIGHT:
  /// Get the height of the viewer
  int get_height() const;

  // SET_CURSOR:
  /// Set the shape of the cursor in the viewer.
  void set_cursor( CursorShape cursor );

  // GET_CURSOR:
  /// Get the shape of the cursor in the viewer.
  CursorShape get_cursor() const;

  // -- internals of the abstract renderer --
private:
  friend class AbstractViewerPrivate;

  AbstractViewerPrivateHandle private_;

  // -- State information --
public:
  /// State variable that keeps track of whether this viewer is visible and hence needs
  /// data to be rendered.
  StateBoolHandle viewer_visible_state_;

};

} // end namespace Core

#endif
