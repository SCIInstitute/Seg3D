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

#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionActivateLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ActivateLayer )

namespace Seg3D
{

bool ActionActivateLayer::validate( Core::ActionContextHandle& context )
{
  if ( ! this->cache_layer_handle( context, this->layer_id_, this->layer_ ) 
    || !this->check_availability( context, this->layer_ ) ) return false;

  return true; // validated
}

bool ActionActivateLayer::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  LayerManager::Instance()->set_active_layer( layer_.handle() );
  return true;
}

Core::ActionHandle ActionActivateLayer::Create( LayerHandle layer )
{
  ActionActivateLayer* action = new ActionActivateLayer;
  
  action->layer_.handle() = layer;
  action->layer_id_.value() = layer->get_layer_id();
  
  return Core::ActionHandle( action );
}

void ActionActivateLayer::Dispatch( LayerHandle layer )
{
  Core::Interface::PostAction( Create( layer ) );
}

} // end namespace Seg3D
