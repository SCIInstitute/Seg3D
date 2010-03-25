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

#include <Application/Interface/Interface.h>
#include <Application/Viewer/Actions/ActionAutoView.h>
#include <Application/ViewerManager/ViewerManager.h>

namespace Seg3D
{

SCI_REGISTER_ACTION( AutoView );

ActionAutoView::ActionAutoView()
{
  add_argument( this->viewer_name_ );
}

bool ActionAutoView::validate( ActionContextHandle& context )
{
  ViewerHandle viewer = this->viewer_weak_handle_.lock();
  if ( !viewer )
  {
    viewer = ViewerManager::Instance()->get_viewer( this->viewer_name_.value() );
    if ( !viewer )
    {
      context->report_error( std::string( "Viewer '" ) + this->viewer_name_.value()
        + "' does not exist" );
      return false;
    }
    this->viewer_weak_handle_ = viewer;
  }

  return true;
}

bool ActionAutoView::run( ActionContextHandle& context, ActionResultHandle& result )
{
  ViewerHandle viewer = this->viewer_weak_handle_.lock();
  if ( viewer )
  {
    viewer->auto_view();
    return true;
  }
  return false;
}

void ActionAutoView::Dispatch(  ViewerHandle& viewer )
{
  ActionAutoView* action = new ActionAutoView;
  action->viewer_name_ = viewer->get_stateid();
  action->viewer_weak_handle_ = viewer;

  Interface::PostAction( ActionHandle( action ) );
}

} // end namespace Seg3D