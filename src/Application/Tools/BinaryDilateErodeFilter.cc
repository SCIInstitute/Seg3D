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

// Core includes
#include <Core/Utils/Exception.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/BinaryDilateErodeFilter.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/Actions/ActionDilateFilter.h>
#include <Application/Filters/Actions/ActionErodeFilter.h>
#include <Application/Filters/Actions/ActionDilateErodeFilter.h>
#include <Application/ViewerManager/ViewerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, BinaryDilateErodeFilter )

namespace Seg3D
{

BinaryDilateErodeFilter::BinaryDilateErodeFilter( const std::string& toolid ) :
  SliceTargetTool( Core::VolumeType::MASK_E, toolid )
{
  // Create an empty list of label options
  std::vector< LayerIDNamePair > empty_list( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  // Need to set ranges and default values for all parameters
  this->add_state( "dilate", this->dilate_state_, 1, 1, 20, 1 );
  this->add_state( "erode", this->erode_state_, 1, 1, 20, 1 );
  this->add_state( "replace", this->replace_state_, false );

  // Whether we use a mask to find which components to use
  this->add_state( "mask", this->mask_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_extra_layer_input( this->mask_state_, Core::VolumeType::MASK_E );

  // Whether that mask should be inverted
  this->add_state( "invert_mask", this->mask_invert_state_, false );
  
  // Whether the filters runs 2d or 3d
  this->add_state( "only2d", this->only2d_state_, false );
}

BinaryDilateErodeFilter::~BinaryDilateErodeFilter()
{
  this->disconnect_all();
}

int BinaryDilateErodeFilter::get_slice_type()
{
  std::string slice_type = this->slice_type_state_->get();
  if ( slice_type == SliceTargetTool::SAGITTAL_C ) return Core::SliceType::SAGITTAL_E;
  if ( slice_type == SliceTargetTool::CORONAL_C ) return Core::SliceType::CORONAL_E;
  if ( slice_type == SliceTargetTool::AXIAL_C ) return Core::SliceType::AXIAL_E;

  CORE_THROW_LOGICERROR( "Invalid slice type" );

  return -1;
}

void BinaryDilateErodeFilter::execute_dilateerode( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionDilateErodeFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->replace_state_->get(),
    this->dilate_state_->get(),
    this->erode_state_->get(),
    this->mask_state_->get(),
    this->mask_invert_state_->get(),
    this->only2d_state_->get(), 
    this->get_slice_type() );
}

void BinaryDilateErodeFilter::execute_dilate( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionDilateFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->replace_state_->get(),
    this->dilate_state_->get(),
    this->mask_state_->get(),
    this->mask_invert_state_->get(),
    this->only2d_state_->get(), 
    this->get_slice_type() );
}

void BinaryDilateErodeFilter::execute_erode( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionErodeFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->replace_state_->get(),
    this->erode_state_->get(),
    this->mask_state_->get(),
    this->mask_invert_state_->get(),
    this->only2d_state_->get(), 
    this->get_slice_type() );
}

} // end namespace Seg3D


