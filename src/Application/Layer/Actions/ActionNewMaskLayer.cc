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

/*
 *****************************************************************************
 *
 *   ActionNewMaskLayer.cc
 *
 *   Also see: Layer, LayerManager
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/19/2009
 *
 *    
 *****************************************************************************
 */

// STL includes

// Boost includes 

// ITK includes

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/Actions/ActionNewMaskLayer.h>

namespace Seg3D {

SCI_REGISTER_ACTION(NewMaskLayer);

bool
ActionNewMaskLayer::check_layer_availability()
{
  bool is_available = false;
  LayerHandle layer = LayerManager::Instance()->get_layer(provided_layer_.value());
  if (layer && !(layer->is_locked()))
  {
    is_available = true;
  }
  return (is_available);
}

bool
ActionNewMaskLayer::lock_layers() const
{
  LayerManager::Instance()->lock_layer(provided_layer_.value()); 
  return (true);
}

bool
ActionNewMaskLayer::release_layers() const
{
  LayerManager::Instance()->unlock_layer(provided_layer_.value());
  return (true);
}



bool 
ActionNewMaskLayer::do_validate(ActionContextHandle& context)
{
  // no parameter checking necessary for this action
  return (true);
}

bool 
ActionNewMaskLayer::execute(ActionContextHandle& context) const
{
  // Create a zeroed out mask layer of the same size as 
  // provided_layer_
  // Give this new layer to the LayerManager
  return (true);
}

} // end namespace Seg3D
