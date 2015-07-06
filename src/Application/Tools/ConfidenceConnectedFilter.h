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

#ifndef APPLICATION_TOOLS_CONFIDENCECONNECTEDFILTER_H
#define APPLICATION_TOOLS_CONFIDENCECONNECTEDFILTER_H

#include <Application/Tool/SeedPointsTool.h>

namespace Seg3D
{

class ConfidenceConnectedFilter : public SeedPointsTool
{

SEG3D_TOOL(
  SEG3D_TOOL_NAME( "ConfidenceConnectedFilter", "Mask out statistically connected data" )
  SEG3D_TOOL_MENULABEL( "Confidence Connected" )
  SEG3D_TOOL_MENU( "Advanced Filters" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+Z" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:ConfidenceConnectedFilter:1" )
  SEG3D_TOOL_HOTKEYS("C=Clear seed points.|Left Mouse Button=Add seed point.|Right Mouse Button=Delete seed point.")
)

public:
  ConfidenceConnectedFilter( const std::string& toolid );
  virtual ~ConfidenceConnectedFilter();
  
  /// EXECUTE:
  /// Fire off the action that executes the filter
  virtual void execute( Core::ActionContextHandle context );

  // -- state --
public:
  Core::StateRangedIntHandle iterations_state_;
  Core::StateRangedDoubleHandle multiplier_state_;
};

} // end namespace

#endif
