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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIOINPAINT_H
#define APPLICATION_TOOLS_ACTIONS_ACTIOINPAINT_H

#include <Core/Action/Actions.h>
#include <Application/Tools/PaintTool.h>

namespace Seg3D
{

class ActionPaintPrivate;
typedef boost::shared_ptr< ActionPaintPrivate > ActionPaintPrivateHandle;

class ActionPaint : public Core::Action
{
  CORE_ACTION
  ( 
    CORE_ACTION_TYPE( "Paint", "Paint with the specified paint tool.")
    CORE_ACTION_ARGUMENT( "target", "The ID of the target mask layer." )
    CORE_ACTION_ARGUMENT( "slice_type", "The slicing direction to be painted on." )
    CORE_ACTION_ARGUMENT( "slice_number", "The slice number to be painted on." )
    CORE_ACTION_ARGUMENT( "data_constraint", "The ID of data constraint layer." )
    CORE_ACTION_ARGUMENT( "min_value", "The minimum data constraint value." )
    CORE_ACTION_ARGUMENT( "max_value", "The maximum data constraint value." )
    CORE_ACTION_ARGUMENT( "negative_data_constraint", "Whether to negate the data constraint." )
    CORE_ACTION_ARGUMENT( "mask_constraint", "The ID of mask constraint layer." )
    CORE_ACTION_ARGUMENT( "negative_mask_constraint", "Whether to negate the mask constraint." )
    CORE_ACTION_ARGUMENT( "x0", "X coordinate of the start position (in index space)." )
    CORE_ACTION_ARGUMENT( "y0", "Y coordinate of the start position (in index space)." )
    CORE_ACTION_ARGUMENT( "x1", "X coordinate of the end position (in index space)." )
    CORE_ACTION_ARGUMENT( "y1", "Y coordinate of the end position (in index space)." )
    CORE_ACTION_ARGUMENT( "brush_radius", "Radius of the paint brush." )
    CORE_ACTION_ARGUMENT( "erase", "Whether to erase instead of painting." )
  )

public:
  ActionPaint();
  virtual ~ActionPaint();

  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  ActionPaintPrivateHandle private_;

public:
  static Core::ActionHandle Create( const PaintToolHandle& paint_tool, 
    const PaintInfo& paint_info );
  
  static void Dispatch( Core::ActionContextHandle context, 
    const PaintToolHandle& paint_tool, const PaintInfo& paint_info );
};

} // end namespace Seg3D

#endif