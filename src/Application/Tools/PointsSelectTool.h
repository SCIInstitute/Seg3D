/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_TOOLS_POINTSSELECTTOOL_H
#define APPLICATION_TOOLS_POINTSSELECTTOOL_H

#include <Application/Tool/SeedPointsTool.h>

namespace Seg3D
{

class PointsSelectToolPrivate;
typedef boost::shared_ptr< PointsSelectToolPrivate > PointsSelectToolPrivateHandle;
  
class PointsSelectTool : public SeedPointsTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "PointsSelectTool", "Tool for selecting points in a layer" )
  SEG3D_TOOL_MENULABEL( "Select Points" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+P" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:PointsSelectTool:1" )
  SEG3D_TOOL_HOTKEYS( "C=Clear seed points.|Left Mouse Button=Add seed point.|Right Mouse Button=Delete seed point." )

)

public:
  PointsSelectTool( const std::string& toolid );
  virtual ~PointsSelectTool();

public:
  /// HANDLE_MOUSE_PRESS:
  /// Called when a mouse button has been pressed.
  virtual bool handle_mouse_press( ViewerHandle viewer, 
                                   const Core::MouseHistory& mouse_history, 
                                   int button, int buttons, int modifiers );

  /// HANDLE_SEED_POINTS_CHANGED:
  /// Called when the seed points have changed.
  virtual void handle_seed_points_changed();
  
  /// REDRAW:
  /// Draw seed points in the specified viewer.
  /// The function should only be called by the renderer, which has a valid GL context.
  /// Locks: StateEngine and RenderResources (not at same time)
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat,
                       int viewer_width, int viewer_height );
  
  // -- signals --
public:
  /// UNITS_CHANGED_SIGNAL:
  typedef boost::signals2::signal< void () > units_changed_signal_type;
  units_changed_signal_type units_changed_signal_;
  
  // -- state --
public:
  /// Selection between display of index and world units.  Needed for radio button group.
  Core::StateLabeledOptionHandle units_selection_state_; 
  
  /// Boolean indicating whether world units (true) or index units (false) should be displayed.
  Core::StateBoolHandle use_world_units_state_; 

  // seed points in world coordinates by default
  Core::StatePointVectorHandle seed_points_index_state_;
  
public:
  static const std::string INDEX_UNITS_C;
  static const std::string WORLD_UNITS_C;
  
private:
  PointsSelectToolPrivateHandle private_;
};

}

#endif