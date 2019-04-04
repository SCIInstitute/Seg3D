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

#ifndef APPLICATION_TOOLS_IMPLICITMODELTOOL_H
#define APPLICATION_TOOLS_IMPLICITMODELTOOL_H

#include <Application/Tool/SeedPointsTool.h>
#include <Core/Geometry/Point.h>

#include <map>

namespace Seg3D
{


class ImplicitModelTool : public SeedPointsTool
{
SEG3D_TOOL(
  SEG3D_TOOL_NAME( "ImplicitModelTool", "Implicit model segmentation tool." )
  SEG3D_TOOL_MENULABEL( "Implicit Model" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+r" )
  SEG3D_TOOL_URL( "http://sciinstitute.github.io/seg3d.pages/Tools/ImplicitModel.html" )
  SEG3D_TOOL_HOTKEYS( "C=Clear seed points.|Left Mouse Button=Add point.|Right Mouse Button=Delete point." )
)

public:
  ImplicitModelTool( const std::string& toolid );
  virtual ~ImplicitModelTool();

  // -- state --
public:
  Core::StateRangedDoubleHandle normalOffset_state_;
  Core::StateOptionHandle kernel_state_;
  Core::StateStringVectorHandle view_modes_state_;
  Core::StateBoolHandle compute_2D_convex_hull_state_;
  Core::StateBoolHandle invert_seed_order_state_;
  Core::StateLabeledOptionHandle convex_hull_selection_state_;

  // TODO: temporary
  Core::StateBoolHandle disabled_widget_state_;

public:
  static const std::string CONVEX_HULL_2D_C;
  static const std::string CONVEX_HULL_3D_C;

  // -- execute --
public:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context ) override;

  /// HANDLE_MOUSE_PRESS:
  /// Called when a mouse button has been pressed.
  virtual bool handle_mouse_press( ViewerHandle viewer,
                                   const Core::MouseHistory& mouse_history,
                                   int button, int buttons, int modifiers ) override;

  /// HANDLE_SEED_POINTS_CHANGED:
  /// Called when the seed points have changed.
  /// The default implementation will cause all the 2D viewers to redraw overlay.
  virtual void handle_seed_points_changed() override;

private:
  void handle_target_layer_changed();
  void handle_convex_hull_type_changed( const std::string& type );
};
  
} // end namespace

#endif
