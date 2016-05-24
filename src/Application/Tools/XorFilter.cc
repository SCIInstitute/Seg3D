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
#include <Application/Tools/XorFilter.h>

// Action associated with tool
#include <Application/Filters/Actions/ActionXorFilter.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, XorFilter )

namespace Seg3D
{

XorFilter::XorFilter( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::MASK_E, toolid )
{
  // Create an empty list of label options
  std::vector< LayerIDNamePair > empty_list( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  // Need to set ranges and default values for all parameters
  this->add_state( "replace", this->replace_state_, false );

  // Whether we use a mask to find which components to use
  this->add_state( "mask", this->mask_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_extra_layer_input( this->mask_state_, Core::VolumeType::MASK_E, true );
}
  
XorFilter::~XorFilter()
{
  this->disconnect_all();
}

void XorFilter::execute( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionXorFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->mask_state_->get(),
    this->replace_state_->get() );    
}

} // end namespace Seg3D


