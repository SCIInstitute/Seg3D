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

// ITK includes
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>

// Core includes
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ITKFilter.h> // ITKFilter inherits LayerFilter
#include <Application/Filters/Actions/ActionSmoothErodeFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, SmoothErodeFilter )

namespace Seg3D
{

bool ActionSmoothErodeFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, 
    this->replace_, context, this->sandbox_ ) ) return false;
    
  // Check for layer existence and type information
  if ( ( this->mask_layer_ != "" ) && ( this->mask_layer_ != "<none>" ) )
  {
    std::string error;
    if ( ! LayerManager::CheckLayerExistenceAndType( this->mask_layer_, 
      Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;
    
    if ( ! LayerManager::CheckLayerSize( this->mask_layer_, this->target_layer_,
      context, this->sandbox_ ) ) return false;
    
    // Check for layer availability 
    if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_, false, 
      context, this->sandbox_ ) ) return false;
  }

  // If the number of iterations is lower than one, we cannot run the filter
  if( this->radius_ < 1 )
  {
    context->report_error( "The radius needs to be larger than or equal to one." );
    return false;
  }

  if( this->radius_ > 254 )
  {
    context->report_error( "The radius is too large." );
    return false;
  }
  
  if ( this->slice_type_ != Core::SliceType::AXIAL_E &&
    this->slice_type_ != Core::SliceType::CORONAL_E &&
    this->slice_type_ != Core::SliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
    
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class SmoothErodeFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;

  int radius_;
  bool invert_mask_;

  bool only2d_;
  int slice_type_;
  
public:
  // RUN_FILTER:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.
  SCI_BEGIN_TYPED_ITK_RUN( this->src_layer_->get_data_type() )
  {
    // Define the type of filter that we use.
    // OutputImageType same as InputImageType (mask)
    typedef itk::BinaryBallStructuringElement< float, 3 > structuring_element_type;
    typedef itk::BinaryErodeImageFilter
      < TYPED_IMAGE_TYPE, TYPED_IMAGE_TYPE, structuring_element_type > filter_type;
  
    // Retrieve the image as an itk image from the underlying data structure
    // NOTE: This only does wrapping and does not regenerate the data.
    typename Core::ITKImageDataT<VALUE_TYPE>::Handle input_image; 
    this->get_itk_image_from_layer<VALUE_TYPE>( this->src_layer_, input_image );

    //
    // Erode
    //

    // Create a new ITK filter instantiation. 
    typename filter_type::Pointer filter = filter_type::New();

    // Relay abort and progress information to the layer that is executing the filter.
    this->forward_abort_to_filter( filter, this->dst_layer_ );
    this->observe_itk_progress( filter, this->dst_layer_ );

    // Setup the filter parameters that we do not want to change.
    filter->SetInput( input_image->get_image() );

    structuring_element_type structuring_element;
    structuring_element.SetRadius( this->radius_ );
    structuring_element.CreateStructuringElement();
    filter->SetKernel( structuring_element );

    filter->SetErodeValue( 1 );

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

    this->insert_itk_image_into_layer( this->dst_layer_, filter->GetOutput() ); 
  }
  SCI_END_TYPED_ITK_RUN()
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "SmoothErode Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "SmoothErode"; 
  }
};


bool ActionSmoothErodeFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<SmoothErodeFilterAlgo> algo( new SmoothErodeFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->set_sandbox( this->sandbox_ );
  algo->radius_ = this->radius_;
  algo->only2d_ = this->only2d_;
  algo->slice_type_ = this->slice_type_;
  
  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
  {
    return false;   
  }

  if ( this->mask_layer_.size() > 0 && this->mask_layer_ != "<none>" )
  {
    if ( !( algo->find_layer( this->mask_layer_, algo->mask_layer_ ) ) )
    {
      return false;
    }   
    algo->lock_for_use( algo->mask_layer_ );
  }
  
  algo->invert_mask_ = this->mask_invert_;  

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
    algo->create_and_lock_mask_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
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

void ActionSmoothErodeFilter::Dispatch( Core::ActionContextHandle context, 
  std::string target_layer, bool replace, int radius, std::string mask_layer,
  bool mask_invert, bool only2d, int slice_type )
{ 
  // Create a new action
  ActionSmoothErodeFilter* action = new ActionSmoothErodeFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->replace_ = replace;
  action->radius_ = radius;
  action->mask_layer_ = mask_layer;
  action->mask_invert_ = mask_invert;
  action->only2d_ = only2d;
  action->slice_type_ = slice_type;
    
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
