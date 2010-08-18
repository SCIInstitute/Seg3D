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
#include <Application/Tool/ToolFactory.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>

// StateEnigne of the tool
#include <Application/Tools/CurvatureAnisotropicDiffusionFilter.h>

// Action associated with tool
#include <Application/Tools/Actions/ActionCurvatureAnisotropicDiffusionFilter.h>

SCI_REGISTER_TOOL( Seg3D, CurvatureAnisotropicDiffusionFilter )

namespace Seg3D
{

// Register the tool into the tool factory

CurvatureAnisotropicDiffusionFilter::CurvatureAnisotropicDiffusionFilter( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::DATA_E, toolid )
{
  // Need to set ranges and default values for all parameters 
  this->add_state( "iterations", this->iterations_state_, 5, 1, 100, 1 );
  this->add_state( "integration_step", this->integration_step_state_, 0.0625, 0.0 , 0.1, 0.005 );
  this->add_state( "conductance", this->conductance_state_, .10, .10, 10.0, .01 );
  this->add_state( "replace", this->replace_state_, true );

}

CurvatureAnisotropicDiffusionFilter::~CurvatureAnisotropicDiffusionFilter()
{
  disconnect_all();
} 

void CurvatureAnisotropicDiffusionFilter::execute( Core::ActionContextHandle context )
{ 
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  ActionCurvatureAnisotropicDiffusionFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->iterations_state_->get(),
    this->integration_step_state_->get(),
    this->conductance_state_->get(),
    this->replace_state_->get() );
}

} // end namespace Seg3D
