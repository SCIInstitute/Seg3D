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

#ifndef APPLICATION_TOOLS_SpeedLINETOOL_H
#define APPLICATION_TOOLS_SpeedLINETOOL_H

#include <Application/Tool/SingleTargetTool.h>

namespace Seg3D
{

class SpeedlineToolPrivate;
typedef boost::shared_ptr< SpeedlineToolPrivate > SpeedlineToolPrivateHandle;

class SpeedlineTool;
typedef boost::shared_ptr<SpeedlineTool> SpeedlineToolHandle;
typedef boost::weak_ptr< SpeedlineTool > SpeedlineToolWeakHandle;

class SpeedlineTool : public SingleTargetTool, public boost::enable_shared_from_this< SpeedlineTool >
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "SpeedlineTool", "Tool for drawing with Speedlines" )
  SEG3D_TOOL_MENULABEL( "Speedline" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+6" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:SpeedlineTool:1" )
  SEG3D_TOOL_HOTKEYS( "Add Point=Left Mouse Button|Move Point=Left Mouse Button"
    "|Delete Point=Right Mouse Button"
    "|Paint Floodfill=F|Erase Floodfill=E|Clear Speedline=C" )  
)

  // -- constructor/destructor --
public:
  SpeedlineTool( const std::string& toolid );
  virtual ~SpeedlineTool();

public:

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
    
  /// HANDLE_MOUSE_MOVE:
  /// Called when the mouse moves in a viewer.
  virtual bool handle_mouse_move( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// HANDLE_KEY_PRESS:
  /// Called when a key is pressed
  virtual bool handle_key_press( ViewerHandle viewer, int key, int modifiers ); 
  
  /// REDRAW:
  /// Draw seed points in the specified viewer.
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

  /// POST_LOAD_STATES:
  /// This virtual function can be implemented in the StateHandlers and will be called after its
  /// states are loaded.  If it doesn't succeed it needs to return false.
  virtual bool post_load_states( const Core::StateIO& state_io );

  // -- dispatch functions --
public:
  void fill( Core::ActionContextHandle context );
  void erase( Core::ActionContextHandle context );
  void reset( Core::ActionContextHandle context );
  void calculate_speedimage( Core::ActionContextHandle context );
  void reset_parameters( Core::ActionContextHandle context );

  // -- State Variables --
public:
  Core::StatePointVectorHandle vertices_state_;
  Core::StateRangedDoubleHandle termination_state_;

  /// Number of iterations the filter needs to run
  Core::StateRangedIntHandle iterations_state_;

  Core::StateLabeledOptionHandle gradient_state_;
  Core::StateBoolHandle gradient_created_state_; // user created, true; loaded false;

  Core::StateSpeedlinePathHandle itk_path_state_;
  Core::StateSpeedlinePathHandle path_state_;

  /// This state variable is only used for path, as we redraw when path state changes. We cannot use
  /// the current vertices state.
  Core::StatePointVectorHandle path_vertices_state_;   

  Core::StateBoolHandle valid_gradient_state_;

  Core::StateIntHandle current_vertex_index_state_;

  Core::StateLabeledOptionHandle target_data_layer_state_;

  Core::StateBoolHandle valid_target_data_layer_state_;
  Core::StateBoolHandle use_smoothing_state_;
  Core::StateBoolHandle use_rescale_state_;

private:
  SpeedlineToolPrivateHandle private_;
};

} // end namespace

#endif
