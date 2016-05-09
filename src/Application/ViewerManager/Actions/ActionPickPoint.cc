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

#include <Core/Action/ActionDispatcher.h>
#include <Core/Action/ActionFactory.h>

#include <Application/ViewerManager/Actions/ActionPickPoint.h>
#include <Application/ViewerManager/ViewerManager.h>

CORE_REGISTER_ACTION( Seg3D, PickPoint )

namespace Seg3D
{

ActionPickPoint::ActionPickPoint()
{
  this->add_parameter( this->point_ );
  this->viewer_ = -1;
}

bool ActionPickPoint::validate( Core::ActionContextHandle& context )
{
  // TODO: Need validation here
  return true;
}

bool ActionPickPoint::run( Core::ActionContextHandle& context,
              Core::ActionResultHandle& result )
{
  if ( this->viewer_ >= 0 )
  {
    ViewerManager::Instance()->pick_point( static_cast< size_t >( this->viewer_ ),
      this->point_ );
    return true;
  }
  else
  { 
    ViewerManager::Instance()->pick_point( this->point_ );
    return true;
  }
  return false;
}

void ActionPickPoint::Dispatch( Core::ActionContextHandle context, const Core::Point& pt )
{
  ActionPickPoint* action = new ActionPickPoint;
  action->point_ = pt;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionPickPoint::Dispatch( Core::ActionContextHandle context, size_t viewer, 
  const Core::Point& pt )
{
  ActionPickPoint* action = new ActionPickPoint;
  action->viewer_ = static_cast< int >( viewer );
  action->point_ = pt;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
