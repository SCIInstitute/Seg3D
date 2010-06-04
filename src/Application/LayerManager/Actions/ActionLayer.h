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
 
#ifndef APPLICATION_LAYERMANAGER_ACTIONS_ACTIONLAYER_H
#define APPLICATION_LAYERMANAGER_ACTIONS_ACTIONLAYER_H 
 
#include <Core/Action/Action.h> 
#include <Application/Layer/LayerFWD.h> 

namespace Seg3D
{

// CLASS ACTIONLAYER:
// Additional functionality for layer functions that allow for checking the availability for a 
// a layer and for creating a short cut to a layer.

class ActionLayer : public Core::Action
{
  // -- helper functions for validating actions of layers --
protected:

  // CACHE_LAYER_HANDLE:
  // This function caches a layer inside the cachedhandle, if it is not already cached.
  // NOTE: if layer already contains a handle, the function returns immediately and returns
  // true, if not it will find the layer in the layer manager.
  bool cache_layer_handle( Core::ActionContextHandle& context,
    Core::ActionParameter< std::string >& layer_id, 
    Core::ActionCachedHandle< LayerHandle >& layer );

  // CACHE_GROUPLAYER_HANDLE:
  // The same as for Layers but now for the group handles
  bool cache_group_handle( Core::ActionContextHandle& context,
    Core::ActionParameter< std::string >& group_id, 
    Core::ActionCachedHandle< LayerGroupHandle >& group );
  
    
  // CHECK_AVAILABILITY:
  // Check whether a layer is available, if it is locked by a filter, this function will return
  // false.
  // NOTE: This function needs to be run on the Application thread within the validate state of 
  // the action.  
  bool check_availability( Core::ActionContextHandle& context,
    Core::ActionCachedHandle< LayerHandle >& layer );
};

} // end namespace Seg3D

#endif
