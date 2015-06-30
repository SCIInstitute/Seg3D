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

#ifndef APPLICATION_TOOLS_FLIPTOOL_H
#define APPLICATION_TOOLS_FLIPTOOL_H

#include <Application/Tool/GroupTargetTool.h>

namespace Seg3D
{

class FlipTool : public GroupTargetTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "FlipTool", "Flip or Rotate a dataset" )
  SEG3D_TOOL_MENULABEL( "Flip/Rotate" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+3" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:FlipTool:1" )
  SEG3D_TOOL_VERSION( "1" )
)

  // -- constructor/destructor --
public:
  FlipTool( const std::string& toolid );
  virtual ~FlipTool();

  void dispatch_flip( Core::ActionContextHandle context, int index );
  void dispatch_rotate( Core::ActionContextHandle context, int axis, bool ccw );

private:
  void dispatch( Core::ActionContextHandle context, const std::vector< int >& permutation );

public:
  Core::StateBoolHandle replace_state_;
};

} // end namespace

#endif
