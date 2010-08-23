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
#include <Application/LayerManager/Actions/ActionComputeIsosurface.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ComputeIsosurface )

namespace Seg3D
{

bool ActionComputeIsosurface::validate( Core::ActionContextHandle& context )
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

  // Check for valid quality factor
  double quality_factor = this->quality_factor_.value();
  if( !( quality_factor == 1.0 || quality_factor == 0.5 || 
    quality_factor == 0.25 || quality_factor == 0.125 ) )
  {
    return false;
  }

  return true; // validated
}

bool ActionComputeIsosurface::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  this->mask_layer_.handle()->compute_isosurface( this->quality_factor_.value() );

  return true;
}

Core::ActionHandle ActionComputeIsosurface::Create( MaskLayerHandle mask_layer, 
  double quality_factor )
{
  ActionComputeIsosurface* action = new ActionComputeIsosurface;

  action->mask_layer_.handle() = mask_layer;
  action->mask_layer_id_.value() = mask_layer->get_layer_id();
  action->quality_factor_.value() = quality_factor;

  return Core::ActionHandle( action );
}

void ActionComputeIsosurface::Dispatch( Core::ActionContextHandle context, 
  MaskLayerHandle mask_layer, double quality_factor )
{
  Core::ActionDispatcher::PostAction( Create( mask_layer, quality_factor ), context );
}

} // end namespace Seg3D
