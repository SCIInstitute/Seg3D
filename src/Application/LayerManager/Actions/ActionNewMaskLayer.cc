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

// Application includes
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>

#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionNewMaskLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, NewMaskLayer )

namespace Seg3D
{

bool ActionNewMaskLayer::validate( Core::ActionContextHandle& context )
{
  if ( !this->cache_group_handle( context, this->group_id_, this->group_ ) ) return false;
  
  return true; // validated
}

bool ActionNewMaskLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  LayerHandle new_mask_layer = LayerHandle( new MaskLayer( 
    "MaskLayer", group_.handle()->get_grid_transform() ) );

  LayerManager::Instance()->insert_layer( new_mask_layer );
  
  return true;
}

Core::ActionHandle ActionNewMaskLayer::Create( LayerGroupHandle group )
{
  ActionNewMaskLayer* action = new ActionNewMaskLayer;
  
  action->group_.handle() = group;
  action->group_id_.value() = group->get_group_id();
  
  return Core::ActionHandle( action );
}

void ActionNewMaskLayer::Dispatch( Core::ActionContextHandle context, LayerGroupHandle group )
{
  Core::ActionDispatcher::PostAction( Create( group ), context );
}

} // end namespace Seg3D
