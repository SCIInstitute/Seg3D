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

#ifndef APPLICATION_TOOLS_ORFILTER_H
#define APPLICATION_TOOLS_ORFILTER_H

// Application includes
#include <Application/Tool/SingleTargetTool.h>

namespace Seg3D
{

class OrFilter : public SingleTargetTool
{

SEG3D_TOOL(
SEG3D_TOOL_NAME( "OrFilter", "Boolean OR operation between two mask layers." )
SEG3D_TOOL_MENULABEL( "Boolean OR" )
SEG3D_TOOL_MENU( "Mask Filters" )
SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+W" )
SEG3D_TOOL_URL( "http://sciinstitute.github.io/seg3d.pages/Tools/BooleanOR.html" )
SEG3D_TOOL_VERSION( "1" )
)

  // -- constructor/destructor --
public:
  OrFilter( const std::string& toolid );
  virtual ~OrFilter();

  // -- state --
public:
  /// Whether the layer needs to be replaced
  Core::StateBoolHandle replace_state_;

  /// Layerid of the mask layer
  Core::StateLabeledOptionHandle mask_state_;
  /// -- execute --
public:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

};

} // end namespace

#endif
