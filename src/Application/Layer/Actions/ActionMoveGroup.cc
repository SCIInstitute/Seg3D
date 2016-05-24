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

// Core includes
#include <Core/Action/Actions.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionMoveGroup.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, MoveGroup )

namespace Seg3D
{
  
class ActionMoveGroupPrivate
{
  // -- Action parameters --
public:
  std::string src_groupid_;
  std::string dst_groupid_;

  // -- Cached handles --
  LayerGroupHandle src_group_;
  LayerGroupHandle dst_group_;
};

ActionMoveGroup::ActionMoveGroup() :
  private_( new ActionMoveGroupPrivate )
{
  this->add_parameter( this->private_->src_groupid_ );
  this->add_parameter( this->private_->dst_groupid_ );
}

bool ActionMoveGroup::validate( Core::ActionContextHandle& context )
{
  this->private_->src_group_ = LayerManager::Instance()->find_group( this->private_->src_groupid_ );
  if ( !this->private_->src_group_ )
  {
    context->report_error( "'" + this->private_->src_groupid_ + "' is not a valid layer group." );
    return false;
  }
  
  this->private_->dst_group_ = LayerManager::Instance()->find_group( this->private_->dst_groupid_ );
  if ( !this->private_->dst_group_ )
  {
    context->report_error( "'" + this->private_->dst_groupid_ + "' is not a valid layer group." );
    return false;
  }

  if ( this->private_->src_group_ == this->private_->dst_group_ )
  {
    context->report_error( "Can't move a group onto itself." );
    return false;
  }

  return true;
}

bool ActionMoveGroup::run( Core::ActionContextHandle& context, 
                 Core::ActionResultHandle& result )
{
  return LayerManager::Instance()->move_group( this->private_->src_group_, 
    this->private_->dst_group_ );
}

void ActionMoveGroup::clear_cache()
{
  this->private_->src_group_.reset();
  this->private_->dst_group_.reset();
}

void ActionMoveGroup::Dispatch( Core::ActionContextHandle context, 
  const std::string& src_groupid, const std::string& dst_groupid )
{
  ActionMoveGroup* action = new ActionMoveGroup;
  action->private_->src_groupid_ = src_groupid;
  action->private_->dst_groupid_ = dst_groupid;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
