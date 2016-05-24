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

#ifndef APPLICATION_LAYER_ACTIONS_ACTIONCOMPUTEISOSURFACE_H
#define APPLICATION_LAYER_ACTIONS_ACTIONCOMPUTEISOSURFACE_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/Layer/LayerFWD.h>

namespace Seg3D
{

class ActionComputeIsosurface : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ComputeIsosurface", "Compute isosurface for the selected layer.")
  CORE_ACTION_ARGUMENT( "layerid", "The layerid of the layer for which the isosurface needs to be computed." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "quality_factor", "1.0", "The quality factor for mask downsampling prior to isosurface computation." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "capping", "false", "Whether isosurfaces will be capped." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "show", "true", "Whether isosurfaces will automatically made visible." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)
  
  // -- Constructor/Destructor --
public:
  ActionComputeIsosurface()
  {
    this->add_parameter( this->layer_id_ );
    this->add_parameter( this->quality_factor_ );
    this->add_parameter( this->capping_enabled_ );
    this->add_parameter( this->show_ );
  }
  
// -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  /// This parameter contains the id of the layer group
  std::string layer_id_;
  
  /// This parameter describes the quality factor of the isosurface
  double quality_factor_;

  /// This parameter describes whether capping is enabled for the isosurface
  bool capping_enabled_;

  /// THis parameter describes whether the isosurface will be shown at the end of the computation
  bool show_;
  
  // -- Dispatch this action from the interface --
public:

  /// DISPATCH
  /// Create and dispatch action that computes the isosurface for the selected layer
  static void Dispatch( Core::ActionContextHandle context, MaskLayerHandle mask_layer, 
    double quality_factor, bool capping_enabled, bool show = false );

  /// DISPATCH:
  /// Create and dispatch action that computes the isosurface for the active layer.
  static void Dispatch( Core::ActionContextHandle context );
};
  
} // end namespace Seg3D

#endif
