/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Application/Filters/Actions/ActionThreshold.h>

#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ThresholdFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Threshold )

namespace Seg3D
{

using namespace Filter;

ActionThreshold::ActionThreshold()
{
  this->add_layer_id( this->target_layer_ );
  this->add_parameter( this->upper_threshold_ );
  this->add_parameter( this->lower_threshold_ );
  this->add_parameter( this->sandbox_ );
}

bool ActionThreshold::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::DATA_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailabilityForUse( this->target_layer_, 
    context, this->sandbox_ ) ) return false;
  
  if ( this->lower_threshold_ > this->upper_threshold_ )
  {
    std::swap( this->lower_threshold_, this->upper_threshold_ );
  }
  
  DataLayerHandle data_layer = LayerManager::FindDataLayer( this->target_layer_, this->sandbox_ );
  double min_val = data_layer->get_data_volume()->get_data_block()->get_min();
  double max_val = data_layer->get_data_volume()->get_data_block()->get_max();

  if( this->lower_threshold_ > max_val ||
    this->upper_threshold_ < min_val )
  {
    context->report_error( "The threshold is out of data range." );
    return false;
  }
  
  // Validation successful
  return true;
}

bool ActionThreshold::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< ThresholdFilter > algo(
    new ThresholdFilter( this->lower_threshold_, this->upper_threshold_) );
  algo->set_sandbox( this->sandbox_ );

  // Find the handle to the layer
  LayerHandle src_layer;
  algo->find_layer( this->target_layer_, src_layer );
  algo->set_data_layer( boost::dynamic_pointer_cast< DataLayer >( src_layer ) );

  // Create the destination layer, which will show progress
  LayerHandle dst_layer;
  algo->create_and_lock_mask_layer_from_layer( algo->data_layer(), dst_layer );
  //  algo->set_mask_layer( boost::dynamic_pointer_cast< MaskLayer >( dst_layer ) );
  algo->set_mask_layer( boost::dynamic_pointer_cast< MaskLayer >( dst_layer ) );

  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( src_layer );

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->data_layer()->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );
  
  // Start the filter on a separate thread.
  Core::Runnable::Start( algo );

  return true;
}

void ActionThreshold::Dispatch( Core::ActionContextHandle context, 
                 std::string target_layer, double lower_threshold,
                 double upper_threshold )
{ 
  // Create a new action
  ActionThreshold* action = new ActionThreshold;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->lower_threshold_ = lower_threshold;
  action->upper_threshold_ = upper_threshold;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
