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
#include <Application/LayerManager/Actions/ActionDeleteIsosurface.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, DeleteIsosurface )

namespace Seg3D
{

bool ActionDeleteIsosurface::validate( Core::ActionContextHandle& context )
{
  // Check for valid layer
  if ( ! this->cache_mask_layer_handle( context, this->mask_layer_id_, this->mask_layer_ ) ) 
  {
    return false;
  }

  // Check for valid mask data
  if( !this->mask_layer_.handle()->is_valid() ) 
  {
    return false;
  }

  return true; // validated
}

bool ActionDeleteIsosurface::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  this->mask_layer_.handle()->delete_isosurface();

  return true;
}

Core::ActionHandle ActionDeleteIsosurface::Create( MaskLayerHandle mask_layer )
{
  ActionDeleteIsosurface* action = new ActionDeleteIsosurface;

  action->mask_layer_.handle() = mask_layer;
  action->mask_layer_id_.value() = mask_layer->get_layer_id();

  return Core::ActionHandle( action );
}

void ActionDeleteIsosurface::Dispatch( Core::ActionContextHandle context, 
  MaskLayerHandle mask_layer )
{
  Core::ActionDispatcher::PostAction( Create( mask_layer ), context );
}

} // end namespace Seg3D