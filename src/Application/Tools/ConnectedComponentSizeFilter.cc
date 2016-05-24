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
#include <Application/Tool/ToolFactory.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>

// StateEngine of the tool
#include <Application/Tools/ConnectedComponentSizeFilter.h>

// Action associated with tool
#include <Application/Filters/Actions/ActionConnectedComponentSizeFilter.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ConnectedComponentSizeFilter )

namespace Seg3D 
{


ConnectedComponentSizeFilter::ConnectedComponentSizeFilter( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::MASK_E, toolid )
{ 
  this->add_state( "log_scale", this->log_scale_state_, true );
}

ConnectedComponentSizeFilter::~ConnectedComponentSizeFilter()
{ 
  this->disconnect_all();
}

void ConnectedComponentSizeFilter::execute( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  ActionConnectedComponentSizeFilter::Dispatch( context,
    this->target_layer_state_->get(), 
    this->log_scale_state_->get() );    
}

} // end namespace Seg3D
