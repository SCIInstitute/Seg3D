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

#ifndef APPLICATION_TOOLS_PAINTTOOL_H
#define APPLICATION_TOOLS_PAINTTOOL_H

#include <boost/smart_ptr.hpp>

#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>

#include <Application/Tool/SeedPointsTool.h>

namespace Seg3D
{

class PaintTool;
typedef boost::shared_ptr< PaintTool > PaintToolHandle;
typedef boost::weak_ptr< PaintTool > PaintToolWeakHandle;

class PaintToolPrivate;
typedef boost::shared_ptr< PaintToolPrivate > PaintToolPrivateHandle;

class PaintInfo
{
public:
  /// Slice we paint on
  std::string target_layer_id_;
  Core::MaskVolumeSliceHandle target_slice_;
  
  /// Data constraint
  std::string data_constraint_layer_id_;
  Core::DataVolumeSliceHandle data_constraint_slice_;
  double min_val_;
  double max_val_;
  bool negative_data_constraint_;
  
  /// Mask constraint 1
  std::string mask_constraint1_layer_id_;
  Core::MaskVolumeSliceHandle mask_constraint1_slice_;
  bool negative_mask_constraint1_;

  /// Mask constraint 2
  std::string mask_constraint2_layer_id_;
  Core::MaskVolumeSliceHandle mask_constraint2_slice_;
  bool negative_mask_constraint2_;

  /// Brush stroke
  std::vector<int> x_;
  std::vector<int> y_;
  
  int brush_radius_;
  bool erase_;
};

class PaintTool : public SeedPointsTool, public boost::enable_shared_from_this< PaintTool >//, public SingleTargetTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "PaintTool", "Tool for painting on masks" )
  SEG3D_TOOL_MENULABEL( "Paint Brush" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+5" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:PaintTool:1" )
  SEG3D_TOOL_HOTKEYS( "Increase Brush Size=Mouse Scroll Up|Decrease Brush Size=Mouse"
    "Scroll Down|Paint Floodfill=F|Erase Floodfill=E|Paint=Left Mouse Button|"
    "Erase=Right Mouse Button|Clear Seed Points=C|Insert Seed Point=Alt Left Mouse Button|"
    "Remove Seed Point=Alt Right Mouse Button" )
  SEG3D_TOOL_VERSION( "1.0" )
)

  // -- constructor/destructor --
public:
  PaintTool( const std::string& toolid );

  virtual ~PaintTool();

public:
  /// HANDLE_MOUSE_ENTER:
  /// Called when the mouse has entered a viewer.
  virtual bool handle_mouse_enter( ViewerHandle viewer, int x, int y );

  /// HANDLE_MOUSE_LEAVE:
  /// Called when the mouse has left a viewer.
  virtual bool handle_mouse_leave( ViewerHandle viewer );

  /// HANDLE_MOUSE_MOVE:
  /// Called when the mouse moves in a viewer.
  virtual bool handle_mouse_move( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// HANDLE_MOUSE_PRESS:
  /// Called when a mouse button has been pressed.
  virtual bool handle_mouse_press( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// HANDLE_MOUSE_RELEASE:
  /// Called when a mouse button has been released.
  virtual bool handle_mouse_release( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// HANDLE_WHEEL:
  /// Called when the mouse wheel has been rotated.
  virtual bool handle_wheel( ViewerHandle viewer, int delta, 
    int x, int y, int buttons, int modifiers );

  /// HANDLE_KEY_PRESS:
  /// Called when a key is pressed.
  virtual bool handle_key_press( ViewerHandle viewer, int key, int modifiers );

  /// HANDLE_UPDATE_CURSOR:
  /// Called when a viewer requires an update to its cursor.
  virtual bool handle_update_cursor( ViewerHandle viewer );
  
  /// REDRAW:
  /// Draw the paint tool in the specified viewer.
  /// The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat,
    int viewer_width, int viewer_height );

  /// HAS_2D_VISUAL:
  /// Returns true if the tool draws itself in the 2D view, otherwise false.
  /// The default implementation returns false.
  virtual bool has_2d_visual();

  /// ACTIVATE:
  /// Activate a tool: this tool is set as the active tool and hence it should
  /// setup the right mouse tools in the viewers.
  virtual void activate();

  /// DEACTIVATE:
  /// Deactivate a tool. A tool is always deactivate before the next one is
  /// activated.
  virtual void deactivate();

protected:
  virtual bool post_load_states();

private:
  friend class ActionPaint;

  bool paint( const PaintInfo& info );
  
  /// HANDLEPAINT:
  /// THis function is called to relay the paint call to the application thread
  static void HandlePaint( PaintToolWeakHandle tool, const PaintInfo info );

public:
  void flood_fill( Core::ActionContextHandle context, bool erase );

  // -- state --
public:

  Core::StateLabeledOptionHandle data_constraint_layer_state_;
  Core::StateLabeledOptionHandle mask_constraint1_layer_state_;
  Core::StateLabeledOptionHandle mask_constraint2_layer_state_;

  Core::StateBoolHandle negative_data_constraint_state_;
  Core::StateBoolHandle negative_mask_constraint1_state_;
  Core::StateBoolHandle negative_mask_constraint2_state_;
  Core::StateBoolHandle show_data_cstr_bound_state_;
  
  /// Radius of the brush
  Core::StateRangedIntHandle brush_radius_state_;

  /// Upper threshold for painting
  Core::StateRangedDoubleHandle upper_threshold_state_;

  /// Lower threshold for painting
  Core::StateRangedDoubleHandle lower_threshold_state_;

  /// Erase data instead of painting
  Core::StateBoolHandle erase_state_;

private:
  PaintToolPrivateHandle private_;

};

} // end namespace

#endif
