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

#ifndef APPLICATION_TOOLS_CLIPBOARDTOOL_H
#define APPLICATION_TOOLS_CLIPBOARDTOOL_H

#include <Application/Tool/SliceTargetTool.h>

namespace Seg3D
{

class ClipboardToolPrivate;
typedef boost::shared_ptr< ClipboardToolPrivate > ClipboardToolPrivateHandle;

class ClipboardTool : public SliceTargetTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "ClipboardTool", "Tool for copy/paste mask slices" )
  SEG3D_TOOL_MENULABEL( "Copy/Paste" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+1" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:CopyPasteTool:1" )
)

  // -- constructor/destructor --
public:
  ClipboardTool( const std::string& toolid );
  virtual ~ClipboardTool();

  // -- dispatch functions --
public:
  void copy( Core::ActionContextHandle context );
  void paste( Core::ActionContextHandle context );

  void grab_min_paste_slice();
  void grab_max_paste_slice();
  
  // -- State Variables --
public:
  Core::StateRangedIntHandle copy_slice_number_state_;
  Core::StateRangedIntHandle paste_min_slice_number_state_;
  Core::StateRangedIntHandle paste_max_slice_number_state_;

private:
  ClipboardToolPrivateHandle private_;
};

} // end namespace

#endif
