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
#include <itkConnectedComponentImageFilter.h>

#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Filters/Actions/ActionConnectedComponentSizeFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, ConnectedComponentSizeFilter )

namespace Seg3D
{

bool ActionConnectedComponentSizeFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, Core::VolumeType::MASK_E, 
    context, this->sandbox_ ) ) return false;

  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailabilityForProcessing( this->target_layer_, 
    context, this->sandbox_ ) ) return false;

  // Validation successful
  return true;
}


// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class ConnectedComponentSizeFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;
  
  bool log_scale_;

public:
  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.
  SCI_BEGIN_ITK_RUN()
  {
    // Define the type of filter that we use.
    typedef itk::ConnectedComponentImageFilter< 
      UCHAR_IMAGE_TYPE, UINT_IMAGE_TYPE > filter_type;

    // Retrieve the image as an itk image from the underlying data structure
    // NOTE: This only does wrapping and does not regenerate the data.
    Core::ITKImageDataT<unsigned char>::Handle input_image; 
    this->get_itk_image_from_layer<unsigned char>( this->src_layer_, input_image );
        
    // Create a new ITK filter instantiation. 
    filter_type::Pointer filter = filter_type::New();

    // Relay abort and progress information to the layer that is executing the filter.
    this->forward_abort_to_filter( filter, this->dst_layer_ );
    this->observe_itk_progress( filter, this->dst_layer_, 0.0, 0.75 );
    
    // Setup the filter parameters that we do not want to change.
    filter->SetInput( input_image->get_image() );

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
      this->report_error( "ITK filter failed to complete." );
      return;
    }

    // As ITK filters generate an inconsistent abort behavior, we record our own abort flag
    // This one is set when the abort button is pressed and an abort is sent to ITK.
    if ( this->check_abort() ) return;

    Core::DataBlockHandle output_datablock = Core::ITKDataBlock::New( filter->GetOutput() );
    
    if ( ! output_datablock )
    {
      this->report_error("Could not allocate enough memory.");
      return;
    }   
    
    unsigned int max_label = filter->GetObjectCount();
    
    std::vector<unsigned int> hist;
    try
    {
      hist.resize( max_label + 1, 0 );
    }
    catch( ... )
    {
      this->report_error( "Could not allocate enough memory." );
      return;   
    }
    
    size_t size = output_datablock->get_size();
    unsigned int* data = reinterpret_cast<unsigned int*>( output_datablock->get_data() );
  
    for ( size_t j = 0; j < size; j++ )
    {
      hist[ data[ j ] ]++;
    }
  
    hist[ 0 ] = 0;
  
    this->dst_layer_->update_progress_signal_( 0.85 );
    if ( this->check_abort() ) return;

    if ( log_scale_ )
    {
      float* ldata = reinterpret_cast<float*>( output_datablock->get_data() );
      for ( size_t j = 0; j < size; j++ )
      {
        ldata[ j ] = logf( static_cast<float>( hist[ data[ j ] ] + 1) );
      }
      output_datablock->update_data_type( Core::DataType::FLOAT_E );
    }
    else
    {
      for ( size_t j = 0; j < size; j++ )
      {
        data[ j ] = hist[ data[ j ] ];
      }   
    }
    
    this->dst_layer_->update_progress_signal_( 0.95 );
    if ( this->check_abort() ) return;
        
    this->dispatch_insert_data_volume_into_layer( this->dst_layer_,
      Core::DataVolumeHandle( new Core::DataVolume(
      this->dst_layer_->get_grid_transform(), output_datablock ) ), true );
  }
  SCI_END_ITK_RUN()

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "ConnectedComponentSize Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "ComponentSize"; 
  }
};


bool ActionConnectedComponentSizeFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<ConnectedComponentSizeFilterAlgo> algo( new ConnectedComponentSizeFilterAlgo );

  // Find the handle to the layer
  algo->set_sandbox( this->sandbox_ );
  algo->src_layer_ = LayerManager::FindLayer( this->target_layer_, this->sandbox_ );

  // In case the layer did not exist, fail gracefully
  if ( !algo->src_layer_ ) return false;
  
  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( algo->src_layer_ );
  
  // Create the destination layer, which will show progress
  algo->create_and_lock_data_layer_from_layer( algo->src_layer_, algo->dst_layer_ );

  // Copy the parameters
  algo->log_scale_ = this->log_scale_;

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
  
  // Start the filter on a separate thread.
  Core::Runnable::Start( algo );

  return true;
}

void ActionConnectedComponentSizeFilter::Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, bool log_scale )
{ 
  // Create a new action
  ActionConnectedComponentSizeFilter* action = new ActionConnectedComponentSizeFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->log_scale_ = log_scale;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
