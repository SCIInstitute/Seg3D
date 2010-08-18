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
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/ITKFilter.h>
#include <Application/Tools/Actions/ActionCurvatureAnisotropicDiffusionFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, CurvatureAnisotropicDiffusionFilter )

namespace Seg3D
{

bool ActionCurvatureAnisotropicDiffusionFilter::validate( Core::ActionContextHandle& context )
{
  // Check for layer existance and type information
  std::string error;
  if ( ! LayerManager::CheckLayerExistanceAndType( this->layer_id_.value(), 
    Core::VolumeType::DATA_E, error ) )
  {
    context->report_error( error );
    return false;
  }
  
  // Check for layer availability 
  Core::NotifierHandle notifier;
  if ( ! LayerManager::CheckLayerAvailability( this->layer_id_.value(), 
    this->replace_.value(), notifier ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
    
  // If the number of iterations is lower than one, we cannot run the filter
  if( this->iterations_.value() < 1 )
  {
    context->report_error( "The number of iterations needs to be larger than zero." );
    return false;
  }
  
  // If the number of integration steps is smaller than one we cannot run the filter
  if( this->integration_step_.value() < 0.0 )
  {
    context->report_error( "The integration step needs to be larger than zero." );
    return false;
  }
  
  // Conductance needs to be a positive number
  if( this->conductance_.value() < 0.0 )
  {
    context->report_error( "The conductance needs to be larger than zero." );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class CurvatureAnisotropicDiffusionFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;
  
  int iterations_;
  double integration_step_;
  double conductance_;

public:
  // RUN:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is run.
  virtual void run()
  {
    // For each datatype implement the filter 
    // NOTE: This macro duplicates the code for each data type that is supported by Seg3D
    // The typedefs TYPED_IMAGE_TYPE and VALUE_TYPE are defined by the macro
    SWITCH_DATATYPE( this->src_layer_->get_data_type(),
    
      typedef itk::CurvatureAnisotropicDiffusionImageFilter< 
        TYPED_IMAGE_TYPE, FLOAT_IMAGE_TYPE > filter_type;

      Core::ITKImageDataT<VALUE_TYPE>::Handle input_image; 
      this->get_itk_image_from_layer<VALUE_TYPE>( this->src_layer_, input_image );
          
      filter_type::Pointer filter = filter_type::New();
      this->forward_progress( filter, this->dst_layer_ );

      filter->SetInput( input_image->get_image() );
      filter->SetInPlace( false );

      filter->SetNumberOfIterations( this->iterations_ );
      filter->SetTimeStep( this->integration_step_ );
      filter->SetConductanceParameter( this->conductance_ );

      filter->Update();

      this->insert_itk_image_into_layer( this->dst_layer_, filter->GetOutput() ); 
    );
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "AnisoDiff";
  }
};


bool ActionCurvatureAnisotropicDiffusionFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<CurvatureAnisotropicDiffusionFilterAlgo> algo(
    new CurvatureAnisotropicDiffusionFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->iterations_ = this->iterations_.value();
  algo->integration_step_ = this->integration_step_.value();
  algo->conductance_ = this->conductance_.value();

  // Find the handle to the layer
  algo->find_layer( this->layer_id_.value(), algo->src_layer_ );

  if ( this->replace_.value() )
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

  // Start the filter
  CurvatureAnisotropicDiffusionFilterAlgo::Start( algo );
}


void ActionCurvatureAnisotropicDiffusionFilter::Dispatch( Core::ActionContextHandle context, 
  std::string layer_id, int iterations, double integration_step, double conductance, bool replace )
{ 
  // Create a new action
  ActionCurvatureAnisotropicDiffusionFilter* action = 
    new ActionCurvatureAnisotropicDiffusionFilter;

  // Setup the parameters
  action->layer_id_.value() = layer_id;
  action->iterations_.value() = iterations;
  action->integration_step_.value() = integration_step;
  action->conductance_.value() = conductance;
  action->replace_.value() = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
