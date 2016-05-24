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
#include <Core/Action/ActionFactory.h>
#include <Core/Action/ActionDispatcher.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionMoveLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, MoveLayer )

namespace Seg3D
{
  
class ActionMoveLayerPrivate
{
  // -- Action parameters --
public:
  std::string src_layerid_;
  std::string dst_layerid_;

  // -- Cached handles --
  LayerHandle src_layer_;
  LayerHandle dst_layer_;
};

ActionMoveLayer::ActionMoveLayer() :
  private_( new ActionMoveLayerPrivate )
{
  this->add_parameter( this->private_->src_layerid_ );
  this->add_parameter( this->private_->dst_layerid_ );
}

bool ActionMoveLayer::validate( Core::ActionContextHandle& context )
{
  // Get the source layer
  this->private_->src_layer_ = LayerManager::Instance()->find_layer_by_id( this->private_->src_layerid_ );
  if ( !this->private_->src_layer_ )
  {
    context->report_error( std::string( "Layer '") + this->private_->src_layerid_ + "' doesn't exist." );
    return false;
  }
  
  // Get the destination layer if it's given
  if ( !this->private_->dst_layerid_.empty() )
  {
    this->private_->dst_layer_ = LayerManager::Instance()->find_layer_by_id( this->private_->dst_layerid_ );
    if ( !this->private_->dst_layer_ )
    {
      context->report_error( std::string( "Layer '") + this->private_->dst_layerid_ + "' doesn't exist." );
      return false;
    }
  }

  if ( this->private_->src_layer_ == this->private_->dst_layer_ )
  {
    context->report_error( "Can't move a layer onto itself." );
    return false;
  }
  
  if ( this->private_->dst_layer_ && 
    this->private_->src_layer_->get_layer_group() != this->private_->dst_layer_->get_layer_group() )
  {
    context->report_error( "Source and destination layers must belong to the same group." );
    return false;
  }

  return true;
}

bool ActionMoveLayer::run( Core::ActionContextHandle& context, 
                 Core::ActionResultHandle& result )
{
  return LayerManager::Instance()->move_layer( this->private_->src_layer_,
    this->private_->dst_layer_ );
}

void ActionMoveLayer::clear_cache()
{
  this->private_->src_layer_.reset();
  this->private_->dst_layer_.reset();
}

void ActionMoveLayer::Dispatch( Core::ActionContextHandle context, 
  const std::string& src_layerid, const std::string& dst_layerid /*= "" */ )
{
  ActionMoveLayer* action = new ActionMoveLayer;
  action->private_->src_layerid_ = src_layerid;
  action->private_->dst_layerid_ = dst_layerid;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
