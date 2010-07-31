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

#include <Core/Action/ActionFactory.h>

#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/Actions/ActionPaint.h>

CORE_REGISTER_ACTION( Seg3D, Paint )

namespace Seg3D
{

ActionPaint::ActionPaint()
{
  this->add_argument( this->toolid_ );
  this->add_argument( this->x0_ );
  this->add_argument( this->y0_ );
  this->add_argument( this->x1_ );
  this->add_argument( this->y1_ );
}

ActionPaint::~ActionPaint()
{
}

bool ActionPaint::validate( Core::ActionContextHandle& context )
{
  PaintToolHandle paint_tool( this->paint_tool_weak_handle_.lock() );
  if ( !paint_tool )
  {
    context->report_error( "Paint tool does not exist" );
    return false;
  }
  return true;
}

bool ActionPaint::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  PaintToolHandle paint_tool( this->paint_tool_weak_handle_.lock() );
  if ( paint_tool )
  {
    return paint_tool->paint( this->x0_.value(), this->y0_.value(),
      this->x1_.value(), this->y1_.value() );
  }

  return false;
}

Core::ActionHandle ActionPaint::Create( const PaintToolHandle& paint_tool, int x0, int y0, int x1, int y1 )
{
  ActionPaint* action = new ActionPaint;
  action->paint_tool_weak_handle_ = paint_tool;
  action->toolid_.set_value( paint_tool->toolid() );
  action->x0_.set_value( x0 );
  action->y0_.set_value( y0 );
  action->x1_.set_value( x1 );
  action->y1_.set_value( y1 );

  return Core::ActionHandle( action );
}

void ActionPaint::Dispatch( Core::ActionContextHandle context, const PaintToolHandle& paint_tool, int x0, int y0, int x1, int y1 )
{
  Core::ActionDispatcher::PostAction( Create( paint_tool, x0, y0, x1, y1 ), context );
}

} // end namespace Seg3D