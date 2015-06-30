/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_TOOLS_RADIALBASISFUNCTIONTOOL_H
#define APPLICATION_TOOLS_RADIALBASISFUNCTIONTOOL_H

#include <Application/Tool/SeedPointsTool.h>

namespace Seg3D
{

class RadialBasisFunctionTool;

class RadialBasisFunctionTool : public SeedPointsTool
{
SEG3D_TOOL(
  SEG3D_TOOL_NAME( "RadialBasisFunctionTool", "Radial basis function segmentation tool." )
  SEG3D_TOOL_MENULABEL( "Radial Basis Function" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+r" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:RBF:1" )
  SEG3D_TOOL_HOTKEYS( "" )
)

public:
  RadialBasisFunctionTool( const std::string& toolid );
  virtual ~RadialBasisFunctionTool();

  // -- state --
public:
  Core::StateRangedDoubleHandle normalOffset_state_;
	Core::StateOptionHandle kernel_state_;

  // -- execute --
public:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

//  /// HANDLE_MOUSE_PRESS:
//  /// Called when a mouse button has been pressed.
//  virtual bool handle_mouse_press( ViewerHandle viewer,
//                                   const Core::MouseHistory& mouse_history,
//                                   int button, int buttons, int modifiers );
//
//  /// HANDLE_MOUSE_RELEASE:
//  /// Called when a mouse button has been released.
//  virtual bool handle_mouse_release( ViewerHandle viewer,
//                                     const Core::MouseHistory& mouse_history,
//                                     int button, int buttons, int modifiers );
//
//  /// HANDLE_MOUSE_MOVE:
//  /// Called when the mouse moves in a viewer.
//  virtual bool handle_mouse_move( ViewerHandle viewer,
//                                  const Core::MouseHistory& mouse_history,
//                                  int button, int buttons, int modifiers );
//
//  /// REDRAW:
//  /// Draw seed points in the specified viewer.
//  /// The function should only be called by the renderer, which has a valid GL context.
//  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat,
//                       int viewer_width, int viewer_height );
//
//  /// HAS_2D_VISUAL:
//  /// Returns true if the tool draws itself in the 2D view, otherwise false.
//  /// The default implementation returns false.
//  virtual bool has_2d_visual();
//  
//  /// HANDLE_SEED_POINTS_CHANGED:
//  /// Called when the seed points have changed.
//  /// The default implementation will cause all the 2D viewers to redraw overlay.
//  virtual void handle_seed_points_changed();

private:
  void handle_target_layer_changed();
	size_t signal_block_count_;
};
  
} // end namespace

#endif
