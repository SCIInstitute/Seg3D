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

#ifndef APPLICATION_TOOLS_THRESHOLDTOOL_H
#define APPLICATION_TOOLS_THRESHOLDTOOL_H

#include <Application/Tool/SeedPointsTool.h>

namespace Seg3D
{

class ThresholdToolPrivate;
typedef boost::shared_ptr< ThresholdToolPrivate > ThresholdToolPrivateHandle;

class ThresholdTool : public SeedPointsTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "ThresholdTool", "Tool for thresholding data" )
  SEG3D_TOOL_MENULABEL( "Threshold" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+8" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:ThresholdTool:1" )
  SEG3D_TOOL_HOTKEYS( "M=Toggle visibility of the preview mask.|C=Clear seed points.|Left Mouse Button=Add seed point.|Right Mouse Button=Delete seed point." )
)

  // -- constructor/destructor --
public:
  ThresholdTool( const std::string& toolid );
  virtual ~ThresholdTool();

public:
  /// REDRAW:
  /// Draw a preview of the threshold result in the specified viewer.
  /// The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat,
    int viewer_width, int viewer_height );

  /// EXECUTE:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

  /// HANDLE_SEED_POINTS_CHANGED:
  /// Called when the seed points have changed.
  virtual void handle_seed_points_changed();

  /// HANDLE_KEY_PRESS:
  /// Called when a key is pressed.
  virtual bool handle_key_press( ViewerHandle viewer, int key, int modifiers );

  // -- state --
public:

  /// Upper threshold for painting
  Core::StateRangedDoubleHandle upper_threshold_state_;

  /// Lower threshold for painting
  Core::StateRangedDoubleHandle lower_threshold_state_;

  /// Whether to show the preview
  Core::StateBoolHandle show_preview_state_;

  /// The opacity of the preview
  Core::StateRangedDoubleHandle preview_opacity_state_;

private:
  ThresholdToolPrivateHandle private_;

};

} // end namespace

#endif
