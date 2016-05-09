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

#include <Application/Viewer/Actions/ActionAutoView.h>
#include <Application/ViewerManager/ViewerManager.h>

CORE_REGISTER_ACTION( Seg3D, AutoView )

namespace Seg3D
{

bool ActionAutoView::validate( Core::ActionContextHandle& context )
{
  ViewerHandle viewer = this->viewer_weak_handle_.lock();
  if ( !viewer )
  {
    viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
    if ( !viewer )
    {
      context->report_error( std::string( "Viewer '" ) 
        + Core::ExportToString( this->viewer_id_ )
        + "' does not exist" );
      return false;
    }
    this->viewer_weak_handle_ = viewer;
  }

  return true;
}

bool ActionAutoView::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  ViewerHandle viewer = this->viewer_weak_handle_.lock();
  if ( viewer )
  {
    if ( viewer->lock_state_->get() )
    {
      std::vector< size_t > locked_viewers = ViewerManager::Instance()->
        get_locked_viewers( viewer->view_mode_state_->index() );
      for ( size_t i = 0; i < locked_viewers.size(); i++ )
      {
        ViewerManager::Instance()->get_viewer( locked_viewers[ i ] )->auto_view();
      }
    }
    else
    {
      viewer->auto_view();
    }
    return true;
  }
  return false;
}

void ActionAutoView::Dispatch( Core::ActionContextHandle context, size_t viewer_id )
{
  ActionAutoView* action = new ActionAutoView;
  action->viewer_id_ = viewer_id;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
