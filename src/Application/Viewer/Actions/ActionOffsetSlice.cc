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

#include <Application/Viewer/Actions/ActionOffsetSlice.h>
#include <Application/ViewerManager/ViewerManager.h>

CORE_REGISTER_ACTION( Seg3D, OffsetSlice )

namespace Seg3D
{

class ActionOffsetSlicePrivate
{
public:
  size_t viewer_id_;
  int offset_;

  ViewerHandle viewer_;

  // Actual offset as a result of the action.
  // This value is used for undoing the action.
  int actual_offset_;
};

ActionOffsetSlice::ActionOffsetSlice() :
  private_( new ActionOffsetSlicePrivate )
{
  this->private_->actual_offset_ = 0;

  this->add_parameter( this->private_->viewer_id_ );
  this->add_parameter( this->private_->offset_ );
}

bool ActionOffsetSlice::validate( Core::ActionContextHandle& context )
{
  if ( this->private_->offset_ == 0 )
  {
    context->report_error( "Offset value can not be 0" );
    this->private_->viewer_.reset();
    return false;
  }
  
  if ( !this->private_->viewer_ )
  {
    this->private_->viewer_ = ViewerManager::Instance()->
      get_viewer( this->private_->viewer_id_ );
    if ( !this->private_->viewer_ )
    {
      context->report_error( std::string( "Invalid viewer ID " ) 
        + Core::ExportToString( this->private_->viewer_id_ ) );
      return false;
    }

    if ( this->private_->viewer_->is_volume_view() )
    {
      context->report_error( "Viewer " + Core::ExportToString( 
        this->private_->viewer_id_ ) + " is in volume mode" );
      this->private_->viewer_.reset();
      return false;
    }
    
    if ( !this->private_->viewer_->get_active_volume_slice() )
    {
      context->report_error( "No layer loaded" );
      this->private_->viewer_.reset();
      return false;
    }
  }
  
  return true;
}

bool ActionOffsetSlice::run( Core::ActionContextHandle& context, 
              Core::ActionResultHandle& result )
{
  if ( this->private_->viewer_ )
  {
    this->private_->actual_offset_ = this->private_->viewer_->
      offset_slice( this->private_->offset_ );
    this->private_->viewer_.reset();
    return true;
  }

  return false;
}

void ActionOffsetSlice::Dispatch( Core::ActionContextHandle context, 
                 const ViewerHandle& viewer, int offset )
{
  ActionOffsetSlice* action = new ActionOffsetSlice;
  action->private_->viewer_ = viewer;
  action->private_->viewer_id_ = viewer->get_viewer_id();
  action->private_->offset_ = offset;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
