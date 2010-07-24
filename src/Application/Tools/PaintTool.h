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

#include <Application/Tool/Tool.h>

namespace Seg3D
{


class PaintToolPrivate;
typedef boost::shared_ptr< PaintToolPrivate > PaintToolPrivateHandle;

class PaintTool : public Tool
{
SCI_TOOL_TYPE("PaintTool", "Paint Brush", "Alt+P", ToolGroupType::TOOL_E, "http://seg3d.org/")
  // -- constructor/destructor --
public:
  PaintTool( const std::string& toolid, bool auto_number = true );

  virtual ~PaintTool();

  virtual void activate();
  virtual void deactivate();

public:
  virtual bool handle_mouse_enter( size_t viewer_id, int x, int y );
  virtual bool handle_mouse_leave( size_t viewer_id );
  virtual bool handle_mouse_move( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  virtual bool handle_mouse_press( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  virtual bool handle_mouse_release( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  virtual bool handle_wheel( int delta, int x, int y, int buttons, int modifiers );
  
  // REDRAW:
  // Draw the paint tool in the specified viewer.
  // The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat );

  // HAS_2D_VISUAL:
  // Returns true if the tool draws itself in the 2D view, otherwise false.
  // The default implementation returns false.
  virtual bool has_2d_visual();

protected:
  virtual bool post_load_states();

  // -- state --
public:

  Core::StateLabeledOptionHandle target_layer_state_;
  Core::StateLabeledOptionHandle data_constraint_layer_state_;
  Core::StateLabeledOptionHandle mask_constraint_layer_state_;

  Core::StateBoolHandle use_active_layer_state_;
  Core::StateBoolHandle negative_data_constraint_state_;
  Core::StateBoolHandle negative_mask_constraint_state_;
  
  // Radius of the brush
  Core::StateRangedIntHandle brush_radius_state_;

  // Upper threshold for painting
  Core::StateRangedDoubleHandle upper_threshold_state_;

  // Lower threshold for painting
  Core::StateRangedDoubleHandle lower_threshold_state_;

  // Erase data instead of painting
  Core::StateBoolHandle erase_state_;

private:
  PaintToolPrivateHandle private_;

private:
  const static size_t VERSION_NUMBER_C;
};

} // end namespace

#endif
