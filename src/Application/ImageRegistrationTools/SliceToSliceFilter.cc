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

// StateEngine of the tool
#include <Application/ImageRegistrationTools/SliceToSliceFilter.h>

// Application includes
#include <Application/Tool/ToolFactory.h>

#include <Application/ImageRegistrationTools/Actions/ActionSliceToSliceFilter.h>

#include <iostream>

SCI_REGISTER_TOOL(Seg3D, SliceToSliceFilter)

namespace Seg3D
{

SliceToSliceFilter::SliceToSliceFilter(const std::string& toolid)
: SingleTargetTool( Core::VolumeType::DATA_E, toolid)
{
  // TODO: temporary hack
  this->valid_target_state_->set( true );
}

SliceToSliceFilter::~SliceToSliceFilter()
{
}

void
SliceToSliceFilter::execute( Core::ActionContextHandle context)
{
  std::cout << "SliceToSliceFilter::execute!" << std::endl;
//  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
//  
//  ActionSliceToSliceFilter::Dispatch( context, this->target_layer_state_->get() );
}

}
