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

// ITK includes
#include <itkDiscreteGaussianImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkCastImageFilter.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Filters/Actions/ActionSpeedlineImageFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, SpeedlineImageFilter )

namespace Seg3D
{

bool ActionSpeedlineImageFilter::validate( Core::ActionContextHandle& context )
{
  // Check for layer existance and type information
  if ( ! LayerManager::CheckLayerExistanceAndType( this->target_layer_, 
    Core::VolumeType::DATA_E, context ) ) return false;
    
  // Validation successful
  return true;
}




// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class SpeedlineImageFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;

  bool is_smoothing_;
  bool is_rescale_;
  
public:
  // RUN:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.
  
  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.
  SCI_BEGIN_TYPED_ITK_RUN( this->src_layer_->get_data_type() )
  {
    // Retrieve the image as an itk image from the underlying data structure
    // NOTE: This only does wrapping and does not regenerate the data.
    typename Core::ITKImageDataT<VALUE_TYPE>::Handle input_image; 
    this->get_itk_image_from_layer<VALUE_TYPE>( this->src_layer_, input_image );

    typedef itk::CastImageFilter<
      TYPED_IMAGE_TYPE,
      FLOAT_IMAGE_TYPE > CastFilterType;

    typename CastFilterType::Pointer caster = CastFilterType::New();
    caster->SetInput( input_image->get_image() );

    float progress_amount;

    if ( this->is_smoothing_ && this->is_rescale_ )
    {
      progress_amount = 1.0 / 3.0;
    } 
    else if ( !this->is_smoothing_ && !this->is_rescale_ )
    {
      progress_amount = 1.0;
    }
    else
    {
      progress_amount = 1.0 / 2.0;
    }
    
    float start_progress = 0.0;
    
    FLOAT_IMAGE_TYPE::Pointer output_image = caster->GetOutput(); 

    if ( this->is_smoothing_ )
    {
      // Define the type of filter that we use.
      typedef itk::DiscreteGaussianImageFilter< 
        FLOAT_IMAGE_TYPE, FLOAT_IMAGE_TYPE > smoothing_filter_type;

      // Create a new ITK filter instantiation. 
      typename smoothing_filter_type::Pointer smoothing_filter = smoothing_filter_type::New();

      smoothing_filter->SetInput( output_image );
      smoothing_filter->SetUseImageSpacingOff();
      smoothing_filter->SetVariance( 4.0 );

      // Ensure we will have some threads left for doing something else
      this->limit_number_of_itk_threads( smoothing_filter );
      this->forward_abort_to_filter( smoothing_filter, this->dst_layer_ );
      this->observe_itk_progress( smoothing_filter, this->dst_layer_, start_progress, progress_amount * 8.0 );


      // Run the actual ITK filter.
      // This needs to be in a try/catch statement as certain filters throw exceptions when they
      // are aborted. In that case we will relay a message to the status bar for information.
      try 
      { 
        smoothing_filter->Update(); 
      } 
      catch ( ... ) 
      {
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
      if ( this->check_abort() ) return;

      output_image = smoothing_filter->GetOutput();
      start_progress = start_progress + progress_amount;
    } 
        

    // Define the type of filter that we use.
    typedef itk::GradientMagnitudeImageFilter< 
      FLOAT_IMAGE_TYPE, FLOAT_IMAGE_TYPE > gradient_magnititude_filter_type;

    // Create a new ITK filter instantiation.   
    typename gradient_magnititude_filter_type::Pointer gm_filter = gradient_magnititude_filter_type::New();

    // Relay abort and progress information to the layer that is executing the filter.
    this->forward_abort_to_filter( gm_filter, this->dst_layer_ );
    this->observe_itk_progress( gm_filter, this->dst_layer_, start_progress, progress_amount );
    
    // Setup the filter parameters that we do not want to change.
    gm_filter->SetInput( output_image );
    gm_filter->SetUseImageSpacingOff();
    
    // Ensure we will have some threads left for doing something else
    this->limit_number_of_itk_threads( gm_filter );

    // Run the actual ITK filter.
    // This needs to be in a try/catch statement as certain filters throw exceptions when they
    // are aborted. In that case we will relay a message to the status bar for information.
    try 
    { 
      gm_filter->Update(); 
    } 
    catch ( ... ) 
    {
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
    if ( this->check_abort() ) return;

    output_image = gm_filter->GetOutput();
    start_progress = start_progress + progress_amount;
    
    if ( this->is_rescale_ )
    {
      double epsilon = 1.0e-7;
      double max_value = 1.0;
      typedef itk::RescaleIntensityImageFilter < FLOAT_IMAGE_TYPE, FLOAT_IMAGE_TYPE > rescale_filter_type;

      rescale_filter_type::Pointer rescale_filter = rescale_filter_type::New();
      this->forward_abort_to_filter( rescale_filter, this->dst_layer_ );
      this->observe_itk_progress( rescale_filter, this->dst_layer_, start_progress, progress_amount );

      rescale_filter->SetInput( output_image );
      rescale_filter->SetOutputMinimum( epsilon );
      rescale_filter->SetOutputMaximum( max_value );
      rescale_filter->Update();
      //output_image = rescale_filter->GetOutput();
      this->insert_itk_image_into_layer( this->dst_layer_, rescale_filter->GetOutput() ); 
    }   

    else
    {
      this->insert_itk_image_into_layer( this->dst_layer_, gm_filter->GetOutput() );
    }
      

  }
  SCI_END_TYPED_ITK_RUN()
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "SpeedlineImage Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "SpeedlineImage";  
  }
};


bool ActionSpeedlineImageFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<SpeedlineImageFilterAlgo> algo( new SpeedlineImageFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->is_smoothing_ = this->is_smoothing_;
  algo->is_rescale_ = this->is_rescale_;

  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
  {
    return false;
  }


  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( algo->src_layer_ );
  
  // Create the destination layer, which will show progress
  algo->create_and_lock_data_layer_from_layer( algo->src_layer_, algo->dst_layer_ );


  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );
  
  // Start the filter on a separate thread.
  Core::Runnable::Start( algo );

  return true;
}

void ActionSpeedlineImageFilter::Dispatch( Core::ActionContextHandle context, 
  std::string target_layer, bool is_smoothing, bool is_rescale )
{ 
  // Create a new action
  ActionSpeedlineImageFilter* action = new ActionSpeedlineImageFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->is_smoothing_ = is_smoothing;
  action->is_rescale_ = is_rescale;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
