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

#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionNewMaskLayer.h>
#include <Application/Layer/MaskLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, NewMaskLayer )

namespace Seg3D
{

bool ActionNewMaskLayer::validate( Core::ActionContextHandle& context )
{
  if ( !( Core::StateEngine::Instance()->is_stateid( this->group_name_.value() ) ) )
  {
    context->report_error( std::string( "GroupID '" ) + this->group_name_.value() + "' is invalid" );
    return false;
  }
  return true; // validated
}

bool ActionNewMaskLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  if ( !this->group_handle_ )
  {
    this->group_handle_ = LayerManager::Instance()->get_LayerGroupHandle_from_group_id( 
      this->group_name_.value() );
    
    if ( !this->group_handle_ )
    {
      context->report_error( std::string( "GroupID '" ) + this->group_name_.value() + "' is invalid" );
      return false;
    }
  }
  
  LayerHandle new_mask_layer = LayerHandle( new MaskLayer( "MaskLayer", group_handle_->get_grid_transform() ));
  LayerManager::Instance()->insert_layer( new_mask_layer );
  
  return true;
}

Core::ActionHandle ActionNewMaskLayer::Create( LayerGroupHandle group )
{
  ActionNewMaskLayer* action = new ActionNewMaskLayer;
  action->group_handle_ = group;
  action->group_name_.value() = group->get_group_id();
  
  return Core::ActionHandle( action );
}

Core::ActionHandle ActionNewMaskLayer::Create( const std::string& group_name )
{
  ActionNewMaskLayer* action = new ActionNewMaskLayer;
  
  SCI_LOG_DEBUG( "trying to create an action for adding a new mask layer to " + group_name );
  
  action->group_handle_ = LayerManager::Instance()->
    get_LayerGroupHandle_from_group_id( group_name );
  action->group_name_.value() = group_name;
  
  return Core::ActionHandle( action );
}

void ActionNewMaskLayer::Dispatch( LayerGroupHandle group )
{
  Core::Interface::PostAction( Create( group ) );
}

void ActionNewMaskLayer::Dispatch( const std::string& group_name )
{
  Core::Interface::PostAction( Create( group_name ) );
}

} // end namespace Seg3D
