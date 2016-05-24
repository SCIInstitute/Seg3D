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

// Teem includes
#include <teem/nrrd.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/NrrdFilter.h>
#include <Application/Filters/Actions/ActionHistogramEqualizationFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, HistogramEqualizationFilter )

namespace Seg3D
{

bool ActionHistogramEqualizationFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::DATA_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, 
    this->replace_, context, this->sandbox_ ) ) return false;
    
  // Check amount
  if( this->amount_ < 0.0 || this->amount_ > 1.0 )
  {
    context->report_error( "Equalization amount needs to be between 0.0 and 1.0." );
    return false;
  }

  // Check bins
  if( this->bins_ < 2 )
  {
    context->report_error( "Bins needs to be bigger than 1." );
    return false;
  }

  // Check how many bins to ignore
  if( this->ignore_bins_ < 0 )
  {
    context->report_error( "Number of bins to ignore needs to be bigger than or equal to 0." );
    return false;
  }

  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class HistogramEqualizationFilterAlgo : public NrrdFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;

  double amount_;
  int bins_;
  int ignore_bins_;

public:
  // RUN_FILTER:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  virtual void run_filter() 
  {
    Core::NrrdDataHandle input_nrrd;
    Core::NrrdDataHandle output_nrrd;
    
    this->get_nrrd_from_layer( this->src_layer_, input_nrrd );
    this->create_nrrd( output_nrrd );

    // Indicate that we started.
    this->update_progress( this->dst_layer_, 0.05 );

    // This teem filter should be thread safe
    // TODO: need to check this --JS
    if ( nrrdHistoEq( output_nrrd->nrrd(), input_nrrd->nrrd(), 0, this->bins_, 
      this->ignore_bins_, static_cast< float >( this->amount_ ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return; 
    }

    // Indicate that we are almost done.
    this->update_progress( this->dst_layer_, 0.75 );

    // Can't abort teem, so if abort was pressed check it now.
    if ( this->check_abort() ) return;
    
    this->insert_nrrd_into_layer( this->dst_layer_, output_nrrd );  
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Histogram Equalization Filter";
  }
  
  
  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "HistEqual"; 
  }
};


bool ActionHistogramEqualizationFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<HistogramEqualizationFilterAlgo> algo( new HistogramEqualizationFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->set_sandbox( this->sandbox_ );
  algo->amount_ = this->amount_;
  algo->bins_ = this->bins_;
  algo->ignore_bins_ = this->ignore_bins_;

  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
  {
    return false;
  }

  if ( this->replace_ )
  {
    // Copy the handles as destination and source will be the same
    algo->dst_layer_ = algo->src_layer_;
    // Mark the layer for processing.
    algo->lock_for_processing( algo->dst_layer_ );  
  }
  else
  {
    // Lock the src layer, so it cannot be used else where
    algo->lock_for_use( algo->src_layer_ );
    
    // Create the destination layer, which will show progress
    algo->create_and_lock_data_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
  }

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );
  
  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionHistogramEqualizationFilter::Dispatch( Core::ActionContextHandle context, 
  std::string target_layer, bool replace, double amount, int bins, int ignore_bins )
{ 
  // Create a new action
  ActionHistogramEqualizationFilter* action = new ActionHistogramEqualizationFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->replace_ = replace;
  action->amount_ = amount;
  action->bins_ = bins;
  action->ignore_bins_ = ignore_bins;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
