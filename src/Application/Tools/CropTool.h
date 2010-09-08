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
  SEG3D_TOOL_MENU( "tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "Alt+T" )
  SEG3D_TOOL_URL( "http://seg3d.org/" )
)

  // -- constructor/destructor --
public:
  CropTool( const std::string& toolid );
  virtual ~CropTool();

public:
  // REDRAW:
  // Draw seed points in the specified viewer.
  // The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat );

  // HAS_2D_VISUAL:
  // Returns true if the tool draws itself in the 2D view, otherwise false.
  // The default implementation returns false.
  virtual bool has_2d_visual();

  // EXECUTE:
  // Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

  // -- state --
public:
  Core::StateIntHandle input_dimensions_state_[ 3 ];
  Core::StateRangedDoubleHandle cropbox_origin_state_[ 3 ];
  Core::StateRangedDoubleHandle cropbox_size_state_[ 3 ];
  Core::StateBoolHandle replace_state_;

private:
  CropToolPrivateHandle private_;
};

} // end namespace

#endif
