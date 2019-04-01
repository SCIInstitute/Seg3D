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

#ifndef APPLICATION_TOOLS_PADTOOL_H
#define APPLICATION_TOOLS_PADTOOL_H

#include <Application/Tool/GroupTargetTool.h>

namespace Seg3D
{

class PadToolPrivate;
typedef boost::shared_ptr< PadToolPrivate > PadToolPrivateHandle;

class PadTool : public GroupTargetTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "PadTool", "Tool for padding data" )
  SEG3D_TOOL_MENULABEL( "Pad" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+0" )
  SEG3D_TOOL_URL( "http://sciinstitute.github.io/seg3d.pages/Tools/PadTool.html" )
)

  // -- constructor/destructor --
public:
  PadTool( const std::string& toolid );
  virtual ~PadTool();

public:
  /// EXECUTE:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

  // -- state --
public:
  Core::StateIntHandle input_dimensions_state_[ 3 ];
  Core::StateLabeledOptionHandle dst_group_state_;
  Core::StateLabeledOptionHandle padding_value_state_;
  Core::StateRangedIntHandle pad_level_state_[ 3 ];
  Core::StateBoolHandle constraint_aspect_state_;
  Core::StateBoolHandle replace_state_;

  const double SCALE_MAX_C;

private:
  PadToolPrivateHandle private_;
};

}


#endif
