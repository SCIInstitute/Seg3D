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

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/Actions/ActionGetLayerGroup.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, GetLayerGroup )

namespace Seg3D
{

bool ActionGetLayerGroup::validate( Core::ActionContextHandle& context )
{
  if ( LayerManager::Instance()->find_layer_by_id( this->target_layer_ ) ) return true;

  LayerHandle layer = LayerManager::Instance()->find_layer_by_name( this->target_layer_ );
  if ( layer )
  {
    // If they passed the name instead, then we'll take the opportunity to get the id instead.
    this->target_layer_ = layer->get_layer_id();
    return true;
  }

  return false; // validated
}

bool ActionGetLayerGroup::run( Core::ActionContextHandle& context,
                               Core::ActionResultHandle& result )
{
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( this->target_layer_ );
  LayerGroupHandle group = layer->get_layer_group();
  std::string group_id = group->get_group_id();
  result.reset( new Core::ActionResult( group_id ) );

  return true;
}

void ActionGetLayerGroup::Dispatch( Core::ActionContextHandle context, const std::string& target_layer )
{
  ActionGetLayerGroup* action = new ActionGetLayerGroup;
  action->target_layer_ = target_layer;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
