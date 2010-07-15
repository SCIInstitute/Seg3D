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

#include <Core/Interface/Interface.h>
#include <Application/ViewerManager/Actions/ActionPickPoint.h>
#include <Application/ViewerManager/ViewerManager.h>

CORE_REGISTER_ACTION( Seg3D, PickPoint )

namespace Seg3D
{

ActionPickPoint::ActionPickPoint()
{
  this->add_argument( this->point_ );
}

ActionPickPoint::~ActionPickPoint()
{
}

bool ActionPickPoint::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool ActionPickPoint::run( Core::ActionContextHandle& context,
              Core::ActionResultHandle& result )
{
  if ( this->source_viewer_ >= 0)
  {
    ViewerManager::Instance()->pick_point( static_cast< size_t >( this->source_viewer_ ),
      this->point_.value() );
    return true;
  }
  return false;
}

Core::ActionHandle ActionPickPoint::Create( size_t src_viewer, const Core::Point& pt )
{
  ActionPickPoint* action = new ActionPickPoint;
  action->source_viewer_ = static_cast< int >( src_viewer );
  action->point_.value() = pt;

  return Core::ActionHandle( action );
}

void ActionPickPoint::Dispatch( Core::ActionContextHandle context, size_t src_viewer, 
  const Core::Point& pt )
{
  Core::ActionDispatcher::PostAction( Create( src_viewer, pt ), context );
}

} // end namespace Seg3D