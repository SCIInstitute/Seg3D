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
#include <Application/ImageRegistrationTools/FFTFilter.h>

// Application includes
#include <Application/Tool/ToolFactory.h>


#include <Application/ImageRegistrationTools/Actions/ActionFFTFilter.h>

#include <iostream>

SCI_REGISTER_TOOL(Seg3D, FFTFilter)

namespace Seg3D
{

FFTFilter::FFTFilter(const std::string& toolid)
  : SingleTargetTool( Core::VolumeType::DATA_E, toolid)
{
  this->add_state( "shrink_factor", this->highest_resolution_level_shrink_factor_state_, 8, 0, 20, 1 );
  this->add_state( "overlap_min", this->overlap_min_state_, 0.2, 0.0, 1.0, 0.05 );
  this->add_state( "overlap_max", this->overlap_max_state_, 1.0, 0.0, 1.0, 0.05 );
  this->add_state( "pixel_spacing", this->pixel_spacing_state_, 1.0, 0.0, 5.0, 0.1 );
  this->add_state( "files", this->input_files_state_ );
  this->add_state( "directory", this->directory_state_, "" );
  this->add_state( "output_mosaic_file", this->output_mosaic_file_state_, "" );

  // TODO: temporary hack
  this->valid_target_state_->set( true );
}

FFTFilter::~FFTFilter()
{
}

void
FFTFilter::execute( Core::ActionContextHandle context)
{
  std::cout << "FFTFilter::execute!" << std::endl;
//  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
//  
//  ActionFFTFilter::Dispatch( context, this->target_layer_state_->get(),
//                            highest_resolution_level_shrink_factor_state_->get(),
//                            overlap_min_state_->get(),
//                            overlap_max_state_->get(),
//                            pixel_spacing_state_->get(),
//                            input_files_state_->get(),
//                            directory_state_->get(),
//                            output_mosaic_file_state_->get() );
}

}
