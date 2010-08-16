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

#ifndef APPLICATION_TOOL_ACTIONS_ACTIONCURVATUREANISOTROPICDIFFUSIONFILTER_H
#define APPLICATION_TOOL_ACTIONS_ACTIONCURVATUREANISOTROPICDIFFUSIONFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>
#include <Application/Layer/Layer.h>

namespace Seg3D
{

  
class ActionCurvatureAnisotropicDiffusionFilter : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "CurvatureAnisotropicDiffusionFilter", "Run the ITK Curvature Anisotropic Diffusion Filter." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_KEY( "iterations", "5", "Number of iterations to perform." )
  CORE_ACTION_KEY( "integration_step", "0.0625", "Number of divisions for each integration." )
  CORE_ACTION_KEY( "conductance", "0.1", "Weight for specifying how closely connected values are." )
  CORE_ACTION_KEY( "replace", "true", "Replace the old layer (true), or add an new layer (false)" )
)
  
  // -- Constructor/Destructor --
public:
  ActionCurvatureAnisotropicDiffusionFilter()
  {
    // Action arguments
    this->add_argument( this->layer_id_ );
    
    // Action options
    this->add_key( this->iterations_ );
    this->add_key( this->integration_step_ );
    this->add_key( this->conductance_ );
    this->add_key( this->replace_ );
  }
  
  virtual ~ActionCurvatureAnisotropicDiffusionFilter()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  Core::ActionParameter< std::string > layer_id_;
  Core::ActionParameter< int > iterations_;
  Core::ActionParameter< double > integration_step_;
  Core::ActionParameter< double > conductance_;
  Core::ActionParameter< bool > replace_;
  
  // -- Dispatch this action from the interface --
public:
        
  // DISPATCH:
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, std::string layer_id, 
    int iterations, double integration_step, 
    double conductance, bool replace );
  
};
  
} // end namespace Seg3D

#endif
