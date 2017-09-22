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

#ifndef APPLICATION_TOOLS_CROPTOOL_H
#define APPLICATION_TOOLS_CROPTOOL_H

#include <Application/Tool/GroupTargetTool.h>

namespace Seg3D
{

class CropToolPrivate;
typedef boost::shared_ptr< CropToolPrivate > CropToolPrivateHandle;

class CropTool : public GroupTargetTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "CropTool", "Tool for cropping data" )
  SEG3D_TOOL_MENULABEL( "Crop" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+2" )
  SEG3D_TOOL_URL( "http://sciinstitute.github.io/seg3d.pages/Tools/CropTool.html" )
)

  // -- constructor/destructor --
public:
  CropTool( const std::string& toolid );
  virtual ~CropTool();

  virtual void deactivate();

public:

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

  /// REDRAW:
  /// Draw seed points in the specified viewer.
  /// The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat,
    int viewer_width, int viewer_height );

  /// HAS_2D_VISUAL:
  /// Returns true if the tool draws itself in the 2D view, otherwise false.
  /// The default implementation returns false.
  virtual bool has_2d_visual();

  /// EXECUTE:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

  /// RESET:
  /// Reset the crop box to the full range of the layer.
  void reset();

  // -- state --
public:
  Core::StateIntHandle input_dimensions_state_[ 3 ];
  Core::StateRangedDoubleHandle cropbox_origin_state_[ 3 ];
  Core::StateRangedDoubleHandle cropbox_size_state_[ 3 ];
  Core::StateRangedIntHandle cropbox_origin_index_state_[ 3 ];
  Core::StateRangedIntHandle cropbox_size_index_state_[ 3 ];
  Core::StateBoolHandle crop_in_index_space_state_;
  Core::StateBoolHandle replace_state_;

private:
  CropToolPrivateHandle private_;
};

} // end namespace

#endif
