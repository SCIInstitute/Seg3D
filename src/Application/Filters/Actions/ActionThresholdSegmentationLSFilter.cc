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

// Boost
#include <boost/timer.hpp>

// ITK includes
#include <itkThresholdSegmentationLevelSetImageFilter.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Filters/Actions/ActionThresholdSegmentationLSFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, ThresholdSegmentationLSFilter )

namespace Seg3D
{

bool ActionThresholdSegmentationLSFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->layer_id_, 
    Core::VolumeType::DATA_E, context, this->sandbox_ ) ) return false;
  
  if ( ! LayerManager::CheckLayerExistenceAndType( this->mask_, 
    Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;

  if ( ! LayerManager::CheckLayerSize( this->layer_id_, this->mask_, 
    context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->layer_id_, 
    false, context, this->sandbox_ ) ) return false;
  
  if ( ! LayerManager::CheckLayerAvailability( this->mask_, 
    false, context, this->sandbox_ ) ) return false;
        
  // If the number of iterations is lower than one, we cannot run the filter
  if( this->iterations_ < 1 )
  {
    context->report_error( "The number of iterations needs to be larger than zero." );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class ThresholdSegmentationLSFilterAlgo : public ITKFilter
{

public:
  LayerHandle data_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;
  
  int iterations_;
  
  double threshold_range_;
  double curvature_;
  double propagation_;
  double edge_;
  
  Core::ActionHandle action_;

public:
  typedef itk::ThresholdSegmentationLevelSetImageFilter< 
      FLOAT_IMAGE_TYPE, FLOAT_IMAGE_TYPE > filter_type;
      
  // RUN:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.
  SCI_BEGIN_ITK_RUN()
  {
    // Define the type of filter that we use.

    // Retrieve the image as an itk image from the underlying data structure
    // NOTE: This only does wrapping and does not regenerate the data.
    Core::ITKFloatImageDataHandle feature_image; 
    this->get_itk_image_from_layer<float>( this->data_layer_, feature_image );

    Core::ITKFloatImageDataHandle seed_image; 
    this->get_itk_image_from_mask_layer<float>( this->mask_layer_, seed_image, 1000.0 );
    
        
    // Calculate mean and std deviation
    Core::DataBlockHandle seed_data_block = Core::ITKDataBlock::New( seed_image );    
    Core::DataBlockHandle feature_data_block = Core::ITKDataBlock::New( feature_image );
    
    size_t size = seed_data_block->get_size();
    float* seed_data = reinterpret_cast< float* >( seed_data_block->get_data() );
    float* feature_data = reinterpret_cast< float* >( feature_data_block->get_data() );

    float x = 0.0;
    float x2 = 0.0;
    size_t n = 0;

    for ( size_t j = 0; j < size; j++ )
    {
      if ( seed_data[ j ] )
      {
        float val = feature_data[ j ];
        x += val;
        x2 += val * val;
        n++;
      }
    }
        
    float mean = 0.0;   
    float var = 0.0;
    
    if ( n > 1 )
    {
      float fn = static_cast<float>( n );
      mean = x / fn;
      var = ( x2 - 2.0f * x * mean + fn * mean * mean ) / ( fn - 1.0f );
    }
        
    // Create a new ITK filter instantiation. 
    filter_type::Pointer filter = filter_type::New();
    
    // Relay abort and progress information to the layer that is executing the filter.
    this->forward_abort_to_filter( filter, this->dst_layer_ );
    this->observe_itk_progress( filter, this->dst_layer_ );
    this->observe_itk_iterations( filter, boost::bind( 
      &ThresholdSegmentationLSFilterAlgo::update_iteration, this, _1, 
      this->dst_layer_, this->action_ ) );

    // Setup the filter parameters that we do not want to change.
    filter->SetInput( seed_image->get_image() );
    filter->SetFeatureImage( feature_image->get_image() );
    
    filter->SetNumberOfIterations( this->iterations_ );
    
    filter->SetUpperThreshold( mean + this->threshold_range_ * sqrt( var ) );
    filter->SetLowerThreshold( mean - this->threshold_range_ * sqrt( var ) );
    filter->SetCurvatureScaling( this->curvature_ );
    filter->SetPropagationScaling( this->propagation_ );
    filter->SetEdgeWeight( this->edge_ );
    filter->SetSmoothingIterations( 0 );
    filter->SetSmoothingTimeStep( 0.0 );
    filter->SetSmoothingConductance( 0.0 );
    filter->SetIsoSurfaceValue( 0.5 );
    filter->SetMaximumRMSError( 0.0 );
    filter->ReverseExpansionDirectionOn();
    
    // Ensure we will have some threads left for doing something else
    this->limit_number_of_itk_threads( filter );

    // Run the actual ITK filter.
    // This needs to be in a try/catch statement as certain filters throw exceptions when they
    // are aborted. In that case we will relay a message to the status bar for information.
    try 
    { 
      filter->Update(); 
    } 
    catch ( ... ) 
    {
      if ( this->check_stop() )
      {
        return;
      }
      
      if ( this->check_abort() )
      {
        this->report_error( "Filter was aborted." );
        return;
      }
      
      this->report_error( "ITK filter failed to complete." );
      return;
    }

    // As ITK filters generate an inconsistent abort behavior, we record our own abort flag
    // This one is set when the abort button is pressed and an abort is sent to ITK.
    if ( this->check_abort() ) 
    {
      // ADD function that makes mask invalid
      return;
    }
    
    // If we want to preserve the data type we convert the data before inserting it back.
    // NOTE: Conversion is done on the filter thread and insertion is done on the application
    // thread.

    this->insert_itk_positive_labels_into_mask_layer( this->dst_layer_, filter->GetOutput() );  

    // Update provenance information
    this->update_provenance_action_string( this->action_ );
  }
  SCI_END_ITK_RUN()
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "ThresholdSegmentationLevelSet Filter";
  }
  
  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "TSLevelSet";  
  } 

private:

  // UPDATE_ITERATION:
  // At regular intervals update the results to the user
  void update_iteration( itk::Object* itk_object, LayerHandle layer, Core::ActionHandle action )
  {
    if ( update_timer_.elapsed() > 1.0 )
    {
      update_timer_.restart();
      filter_type* filter = dynamic_cast<filter_type* >( itk_object );
      if ( filter )
      {
        this->insert_itk_positive_labels_into_mask_layer( layer, filter->GetOutput() );
        ActionThresholdSegmentationLSFilter* action_ptr =
          dynamic_cast< ActionThresholdSegmentationLSFilter* >( action.get() );
        action_ptr->set_iterations( filter->GetElapsedIterations() );
      }
    }
  }

  boost::timer update_timer_;
};


bool ActionThresholdSegmentationLSFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<ThresholdSegmentationLSFilterAlgo> algo(
    new ThresholdSegmentationLSFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->set_sandbox( this->sandbox_ );
  algo->iterations_ = this->iterations_;
  algo->threshold_range_ = this->threshold_range_;
  algo->curvature_ = this->curvature_;
  algo->propagation_ = this->propagation_;
  algo->edge_ = this->edge_;
  
  algo->action_ = this->shared_from_this();

  // Find the handle to the layer
  if ( !( algo->find_layer( this->layer_id_, algo->data_layer_ ) ) )
  {
    return false;
  }

  if ( !( algo->find_layer( this->mask_, algo->mask_layer_ ) ) )
  {
    return false;
  }

  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( algo->data_layer_ );
  algo->lock_for_use( algo->mask_layer_ );
  
  // Create the destination layer, which will show progress
  algo->create_and_lock_mask_layer_from_layer( algo->mask_layer_, algo->dst_layer_ );
  // Tell the new layer to enable the stop button which can trigger a stop signal and stop
  // the filter.
  algo->dst_layer_->set_allow_stop();

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

void ActionThresholdSegmentationLSFilter::Dispatch(  Core::ActionContextHandle context, 
    std::string layer_id, std::string mask, int iterations, double threshold_range, 
    double curvature, double propagation, double edge  )
{ 
  // Create a new action
  ActionThresholdSegmentationLSFilter* action = 
    new ActionThresholdSegmentationLSFilter;

  // Setup the parameters
  action->layer_id_ = layer_id;
  action->mask_ = mask;
  action->iterations_ = iterations;
  action->threshold_range_ = threshold_range;
  action->curvature_ = curvature;
  action->propagation_ = propagation;
  action->edge_ = edge;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
