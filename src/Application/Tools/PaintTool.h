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

class PaintTool : public Tool
{
SCI_TOOL_TYPE("PaintTool", "Paint Brush", "Alt+P", ToolGroupType::TOOL_E, "http://seg3d.org/")
  // -- constructor/destructor --
public:
  PaintTool( const std::string& toolid );
  virtual ~PaintTool();

  // -- state constraints --

  // Constrain viewer to right painting tool when layer is selected
  void target_constraint( std::string layerid );

  // Constrain state to right options when mask layer is selected
  void mask_constraint( std::string layerid );

  // -- activate/deactivate tool --

  virtual void activate();
  virtual void deactivate();
  
private:
  // -- handle updates from layermanager --
  void handle_layers_changed();

  void update_target_options();
  void update_constraint_options();

private:
  std::vector< std::string > target_layer_options_;
  std::vector< std::string > data_constraint_options_;
  std::vector< std::string > mask_constraint_options_;

  size_t signal_block_count_;

  // -- state --
public:
  Core::StateStringHandle target_layer_state_;
  Core::StateStringHandle mask_layer_state_;

  Core::StateStringHandle target_layer_name_state_;
  Core::StateStringHandle data_constraint_layer_name_state_;
  Core::StateStringHandle mask_constraint_layer_name_state_;
  
  // Radius of the brush
  Core::StateRangedIntHandle brush_radius_state_;

  // Upper threshold for painting
  Core::StateRangedDoubleHandle upper_threshold_state_;

  // Lower threshold for painting
  Core::StateRangedDoubleHandle lower_threshold_state_;

  // Erase data instead of painting
  Core::StateBoolHandle erase_state_;
};

} // end namespace

#endif
