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

#ifndef APPLICATION_TOOLS_BINARYDILATEERODEFILTER_H
#define APPLICATION_TOOLS_BINARYDILATEERODEFILTER_H

#include <Application/Tool/SliceTargetTool.h>

namespace Seg3D
{

class BinaryDilateErodeFilter : public SliceTargetTool
{

SEG3D_TOOL(
SEG3D_TOOL_NAME( "BinaryDilateErodeFilter", "Grow and Shrink segmentations" )
SEG3D_TOOL_MENULABEL( "Fast Binary Dilate -> Erode" )
SEG3D_TOOL_MENU( "Mask Filters" )
SEG3D_TOOL_SHORTCUT_KEY( "Ctrl+Alt+T" )
SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:BinaryDilateErodeFilter:1" )
)

public:
  BinaryDilateErodeFilter( const std::string& toolid );
  virtual ~BinaryDilateErodeFilter();

  // -- state --
public:
  /// How many iterations for dilating 
  Core::StateRangedIntHandle dilate_state_;

  /// How many iterations for eroding 
  Core::StateRangedIntHandle erode_state_;

  /// Whether to replace the input layer with the output layer
  Core::StateBoolHandle replace_state_;

  /// Layerid of the mask layer
  Core::StateLabeledOptionHandle mask_state_;
    
  /// Whether mask should be inverted in this filter
  Core::StateBoolHandle mask_invert_state_;
  
  /// Whether the filters runs 2d or 3d
  Core::StateBoolHandle only2d_state_;

public:

  /// Execute the dilate -> erode action
  void execute_dilateerode( Core::ActionContextHandle context );
  
  /// Execute the dilate action 
  void execute_dilate( Core::ActionContextHandle context );
  
  /// Execute the erode action
  void execute_erode( Core::ActionContextHandle context );

private:
  /// Internal function for getting the slice type
  int get_slice_type();
};

} // end namespace

#endif
