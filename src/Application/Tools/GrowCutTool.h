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

#ifndef APPLICATION_TOOLS_GROWCUT_H
#define APPLICATION_TOOLS_GROWCUT_H

#include <boost/smart_ptr.hpp>

#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>

#include <Application/Tools/PaintTool.h>
#include <Application/Tool/SingleTargetTool.h>

namespace Seg3D
{

class GrowCutTool;
typedef boost::shared_ptr< GrowCutTool > GrowCutToolHandle;
typedef boost::weak_ptr< GrowCutTool > GrowCutToolWeakHandle;

class GrowCutToolPrivate;
typedef boost::shared_ptr< GrowCutToolPrivate > GrowCutToolPrivateHandle;

class GrowCutTool : public SingleTargetTool, public boost::enable_shared_from_this< GrowCutTool >
{

  SEG3D_TOOL
  (
    SEG3D_TOOL_NAME( "GrowCutTool", "Tool for grow cut segmentation painting on mask" )
    SEG3D_TOOL_MENULABEL( "Grow Cut" )
    SEG3D_TOOL_MENU( "Tools" )
    SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+5" )
    SEG3D_TOOL_URL( "http://www.sci.utah.edu/SCIRunDocs/index.php/CIBC:Seg3D2:GrowCutTool:1" )
    SEG3D_TOOL_HOTKEYS( "Increase Brush Size=Mouse Scroll Up|Decrease Brush Size=Mouse "
                        "Scroll Down|Paint Foreground=Left Mouse Button|"
                        "Paint Background=Right Mouse Button|Erase Foreground=Ctrl Left Mouse Button|"
                        "Erase Background=Ctrl Right Mouse Button|Run Growcut=R|PostProcess=S|Run Growcut and PostProcess=B" )
    SEG3D_TOOL_VERSION( "1.0" )
  )

  // -- constructor/destructor --
public:
  GrowCutTool( const std::string& toolid );

  virtual ~GrowCutTool();

public:
  // HANDLE_MOUSE_ENTER:
  // Called when the mouse has entered a viewer.
  virtual bool handle_mouse_enter( ViewerHandle viewer, int x, int y );

  // HANDLE_MOUSE_LEAVE:
  // Called when the mouse has left a viewer.
  virtual bool handle_mouse_leave( ViewerHandle viewer );

  // HANDLE_MOUSE_MOVE:
  // Called when the mouse moves in a viewer.
  virtual bool handle_mouse_move( ViewerHandle viewer,
                                  const Core::MouseHistory& mouse_history,
                                  int button, int buttons, int modifiers );

  // HANDLE_MOUSE_PRESS:
  // Called when a mouse button has been pressed.
  virtual bool handle_mouse_press( ViewerHandle viewer,
                                   const Core::MouseHistory& mouse_history,
                                   int button, int buttons, int modifiers );

  // HANDLE_MOUSE_RELEASE:
  // Called when a mouse button has been released.
  virtual bool handle_mouse_release( ViewerHandle viewer,
                                     const Core::MouseHistory& mouse_history,
                                     int button, int buttons, int modifiers );

  // HANDLE_WHEEL:
  // Called when the mouse wheel has been rotated.
  virtual bool handle_wheel( ViewerHandle viewer, int delta,
                             int x, int y, int buttons, int modifiers );

  // HANDLE_KEY_PRESS:
  // Called when a key is pressed.
  virtual bool handle_key_press( ViewerHandle viewer, int key, int modifiers );

  // HANDLE_UPDATE_CURSOR:
  // Called when a viewer requires an update to its cursor.
  virtual bool handle_update_cursor( ViewerHandle viewer );

  // REDRAW:
  // Draw the paint tool in the specified viewer.
  // The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat,
                       int viewer_width, int viewer_height );

  // HAS_2D_VISUAL:
  // Returns true if the tool draws itself in the 2D view, otherwise false.
  // The default implementation returns false.
  virtual bool has_2d_visual();

  // ACTIVATE:
  // Activate a tool: this tool is set as the active tool and hence it should
  // setup the right mouse tools in the viewers.
  virtual void activate();

  // DEACTIVATE:
  // Deactivate a tool. A tool is always deactivate before the next one is
  // activated.
  virtual void deactivate();

protected:
  virtual bool post_load_states();

private:
  friend class ActionPaint;

  bool paint( const PaintInfo& info );

  // HANDLEPAINT:
  // THis function is called to relay the paint call to the application thread
  static void HandlePaint( GrowCutToolWeakHandle tool, const PaintInfo info );



public:

  void initialize_layers( Core::ActionContextHandle context );
  void run_growcut( Core::ActionContextHandle context, bool smoothing );
  void run_smoothing( Core::ActionContextHandle context );

  void handle_layer_data_changed();

  // -- state --
public:
  Core::StateLabeledOptionHandle foreground_layer_state_;
  Core::StateLabeledOptionHandle background_layer_state_;
  Core::StateLabeledOptionHandle output_layer_state_;

  // Radius of the brush
  Core::StateRangedIntHandle brush_radius_state_;

  // Erase data instead of painting
  Core::StateBoolHandle erase_state_;

private:
  GrowCutToolPrivateHandle private_;
};
} // end namespace

#endif // ifndef APPLICATION_TOOLS_GROWCUT_H
