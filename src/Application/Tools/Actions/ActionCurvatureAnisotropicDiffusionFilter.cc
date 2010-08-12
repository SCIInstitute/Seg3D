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

#include <itkCurvatureAnisotropicDiffusionImageFilter.h>

#include <Application/LayerManager/LayerManager.h>

#include <Application/Tool/ITKFilterAlgo.h>
#include <Application/Tools/Actions/ActionCurvatureAnisotropicDiffusionFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, CurvatureAnisotropicDiffusionFilter )

namespace Seg3D
{


// ALGORITHM CLASS
// This class the actual work and is created by run

class CurvatureAnisotropicDiffusionFilterAlgo : public ITKFilterAlgo
{

public:
  bool run( DataLayerHandle target_layer, bool replace, int iterations,
    double steps, double conductance );
};

bool CurvatureAnisotropicDiffusionFilterAlgo::run( DataLayerHandle target_layer, bool replace, 
  int iterations, double steps, double conductance )
{

  target_layer->data_state_->set( Layer::PROCESSING_C );

  // For each datatype implement the filter 
  FOREACH_DATATYPE( target_layer->get_data_type(),
  
    typedef itk::CurvatureAnisotropicDiffusionImageFilter< 
      TYPED_IMAGE_TYPE, FLOAT_IMAGE_TYPE > filter_type;

    Core::ITKImageDataT<VALUE_TYPE>::Handle input_image; 
    Core::ITKImageDataT<float>::Handle output_image;


    this->retrieve_typed_itk_image<VALUE_TYPE>( target_layer, input_image );
        
    filter_type::Pointer filter = filter_type::New();
    this->forward_progress( filter, target_layer );

    filter->SetInput( input_image->get_image() );
    filter->SetInPlace( false );

    filter->SetNumberOfIterations( iterations );
    filter->SetTimeStep( steps );
    filter->SetConductanceParameter( conductance );

    filter->Update();

    // this->insert_data_into_layer( filter->GetOutput(), target_layer ); 
  );
    

  target_layer->data_state_->set( Layer::AVAILABLE_C );

  return true;
}

bool ActionCurvatureAnisotropicDiffusionFilter::validate( Core::ActionContextHandle& context )
{
  // Check whether the input layer is a valid layer id
  if ( ! LayerManager::Instance()->get_data_layer_by_id( this->layer_id_.value() ) )
  {
    context->report_error( std::string( "LayerID '" ) + this->layer_id_.value() +
      std::string( "' is not valid." ) );
    return false;   
  }
  
  // If the number of iterations is lower than one, we cannot run the filter
  if( this->iterations_.value() < 1 )
  {
    context->report_error( "The number of iterations needs to be larger than zero." );
    return false;
  }
  
  // If the number of integration steps is smaller than one we cannot run the filter
  if( this->steps_.value() < 1 )
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
  return true;
}

bool ActionCurvatureAnisotropicDiffusionFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  CurvatureAnisotropicDiffusionFilterAlgo algo;
  
  // Run the algorithm
  return ( algo.run( 
    LayerManager::Instance()->get_data_layer_by_id( this->layer_id_.value() ),
    this->replace_.value(),
    this->iterations_.value(),
    this->steps_.value(),
    this->conductance_.value() ) );
}

void ActionCurvatureAnisotropicDiffusionFilter::Dispatch( Core::ActionContextHandle context, 
  std::string layer_id, int iterations, int steps, double conductance, bool replace )
{ 
  // Create a new action
  ActionCurvatureAnisotropicDiffusionFilter* action = 
    new ActionCurvatureAnisotropicDiffusionFilter;

  // Setup the parameters
  action->layer_id_.value() = layer_id;
  action->iterations_.value() = iterations;
  action->steps_.value() = steps;
  action->conductance_.value() = conductance;
  action->replace_.value() = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
