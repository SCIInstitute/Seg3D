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

#ifndef APPLICATION_LAYERMANAGER_ACTIONS_ACTIONCOMPUTEISOSURFACE_H
#define APPLICATION_LAYERMANAGER_ACTIONS_ACTIONCOMPUTEISOSURFACE_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/Layer/LayerFWD.h>
#include <Application/LayerManager/Actions/ActionLayer.h>

namespace Seg3D
{

class ActionComputeIsosurface : public ActionLayer
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ComputeIsosurface", "Compute isosurface for the selected layer.")
  CORE_ACTION_ARGUMENT( "layerid", "The layerid of the layer for which the isosurface needs to be computed." )
  CORE_ACTION_ARGUMENT( "quality_factor", "The quality factor for mask downsampling prior to isosurface computation." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)
  
  // -- Constructor/Destructor --
public:
  ActionComputeIsosurface()
  {
    this->add_argument( this->mask_layer_id_ );
    this->add_argument( this->quality_factor_ );
    this->add_cachedhandle( this->mask_layer_ );
  }
  
  virtual ~ActionComputeIsosurface()
  {
  }
  
// -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  // This parameter contains the id of the layer group
  Core::ActionParameter< std::string > mask_layer_id_;
  
  // This cached handle contains a short cut to the layer for which the isosurface needs to be
  // generated
  Core::ActionCachedHandle< MaskLayerHandle > mask_layer_;

  Core::ActionParameter< double > quality_factor_;

  // -- Dispatch this action from the interface --
public:

  // CREATE:
  // Create an action that computes the isosurface for the selected layer
  static Core::ActionHandle Create( MaskLayerHandle mask_layer, double quality_factor );

  // DISPATCH
  // Create and dispatch action that computes the isosurface for the selected layer
  static void Dispatch( Core::ActionContextHandle context, MaskLayerHandle mask_layer, 
    double quality_factor );  
};
  
} // end namespace Seg3D

#endif
