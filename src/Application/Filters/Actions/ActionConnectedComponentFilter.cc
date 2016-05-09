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
#include <Application/Filters/Actions/ActionConnectedComponentFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, ConnectedComponentFilter )

namespace Seg3D
{

bool ActionConnectedComponentFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailabilityForProcessing( this->target_layer_, 
    context, this->sandbox_ ) ) return false;

  // Check for layer existence and type information
  bool use_mask = false;
  if ( this->mask_.size() > 0 && this->mask_ != "<none>" )
  {
    // Check whether the layer actually exists
    if ( ! LayerManager::CheckLayerExistenceAndType( this->mask_, 
      Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;
    
    // Check whether the size matches the main target layer
    if ( ! LayerManager::CheckLayerSize( this->mask_, this->target_layer_,
      context, this->sandbox_ ) ) return false;

    // Check for layer availability 
    if ( ! LayerManager::CheckLayerAvailabilityForProcessing( this->mask_, 
      context, this->sandbox_ ) ) return false;
    
    use_mask = true;
  }

  // Check whether there are enough seed points
  if ( this->seeds_.size() == 0 && use_mask == false )
  {
    context->report_error( "There needs to be at least one seed point." );
    return false;
  }
  
  // Validation successful
  return true;
}


// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class ConnectedComponentFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;

  std::vector< Core::Point > seeds_;
  bool invert_mask_;
  
public:
  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.
  SCI_BEGIN_ITK_RUN()
  {
    // Define the type of filter that we use.
    typedef itk::ConnectedComponentImageFilter< 
      UCHAR_IMAGE_TYPE, USHORT_IMAGE_TYPE > filter_type;

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

    bool need_32bits = false;
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
      if (  filter->GetObjectCount() > static_cast<unsigned long int>(
        itk::NumericTraits<unsigned short>::max() ) )
      {
        need_32bits = true;
      }
      else
      {
        this->report_error( "ITK filter failed to complete." );
        return;
      }
    }

    // As ITK filters generate an inconsistent abort behavior, we record our own abort flag
    // This one is set when the abort button is pressed and an abort is sent to ITK.
    if ( this->check_abort() ) return;

    Core::DataBlockHandle output_datablock;

    if ( need_32bits )
    {
      filter = 0;
    
      // Define the type of filter that we use.
      typedef itk::ConnectedComponentImageFilter< 
        UCHAR_IMAGE_TYPE, UINT_IMAGE_TYPE > filter32_type;

      // Retrieve the image as an itk image from the underlying data structure
      // NOTE: This only does wrapping and does not regenerate the data.
      Core::ITKImageDataT<unsigned char>::Handle input_image; 
      this->get_itk_image_from_layer<unsigned char>( this->src_layer_, input_image );
          
      // Create a new ITK filter instantiation. 
      filter32_type::Pointer filter32 = filter32_type::New();

      // Relay abort and progress information to the layer that is executing the filter.
      this->forward_abort_to_filter( filter32, this->dst_layer_ );
      this->observe_itk_progress( filter32, this->dst_layer_, 0.0, 0.75 );
      
      // Setup the filter parameters that we do not want to change.
      filter32->SetInput( input_image->get_image() );

      // Ensure we will have some threads left for doing something else
      this->limit_number_of_itk_threads( filter32 );

      // Run the actual ITK filter.
      // This needs to be in a try/catch statement as certain filters throw exceptions when they
      // are aborted. In that case we will relay a message to the status bar for information.     
      
      try 
      { 
        filter32->Update(); 
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

      output_datablock = Core::ITKDataBlock::New( filter32->GetOutput() );

      if ( ! output_datablock )
      {
        this->report_error("Could not allocate enough memory.");
        return;
      }
      
      unsigned int max_label = filter32->GetObjectCount();
      std::vector<unsigned int> lut( max_label + 1, 0 );
      
      Core::GridTransform grid = input_image->get_grid_transform();
      Core::Transform trans = grid.get_inverse();
      int nx = static_cast<int>( grid.get_nx() ); 
      int ny = static_cast<int>( grid.get_ny() ); 
      int nz = static_cast<int>( grid.get_nz() ); 
      
      unsigned int* data = reinterpret_cast<unsigned int*>( output_datablock->get_data() );
      for ( size_t i = 0; i < this->seeds_.size(); ++i )
      {   
        Core::Point location = trans * seeds_[ i ];
        int x = static_cast<int>( Core::Round( location.x() ) );
        int y = static_cast<int>( Core::Round( location.y() ) );
        int z = static_cast<int>( Core::Round( location.z() ) );
        
        if ( x >= 0 && y >= 0 && z >= 0 && x < nx && y < ny && z < nz )
        {
          unsigned int val = data[ output_datablock->to_index( static_cast<size_t>( x ), 
            static_cast<size_t>( y ), static_cast<size_t>( z ) ) ];
          if ( val ) lut[ val ] = 1;
        }
      }
      
      if ( this->mask_layer_ )
      {
        Core::MaskDataBlockHandle mask_handle = 
          dynamic_cast<MaskLayer*>( this->mask_layer_.get() )->
          get_mask_volume()->get_mask_data_block();
          
        unsigned char mask_value = mask_handle->get_mask_value();
        size_t size = mask_handle->get_size();
        unsigned char* mask_data = mask_handle->get_mask_data();
        
        Core::DataBlock::shared_lock_type lock( mask_handle->get_mutex() );
        if ( this->invert_mask_ )
        {
          for ( size_t j = 0; j < size; j++ )
          {
            if ( ! ( mask_data[ j ] & mask_value ) && data[ j ] ) lut[ data[ j ] ] = 1;
          }     
        }
        else
        {
          for ( size_t j = 0; j < size; j++ )
          {
            if ( ( mask_data[ j ] & mask_value ) && data[ j ] ) lut[ data[ j ] ] = 1;
          }           
        
        }
      }

      this->dst_layer_->update_progress_signal_( 0.80 );
      if ( this->check_abort() )
      {
        return;
      }

      size_t size = output_datablock->get_size();
      for ( size_t j = 0; j < size; j++ )
      {
        data[ j ] = lut[ data[ j ] ];
      }
    }
    else
    {
      // As ITK filters generate an inconsistent abort behavior, we record our own abort flag
      // This one is set when the abort button is pressed and an abort is sent to ITK.
      if ( this->check_abort() ) return;

      output_datablock = Core::ITKDataBlock::New( filter->GetOutput() );    
    
      if ( ! output_datablock )
      {
        this->report_error("Could not allocate enough memory.");
        return;
      }
      
      unsigned int max_label = filter->GetObjectCount();
      std::vector<unsigned short> lut( max_label + 1, 0 );
      
      Core::GridTransform grid = input_image->get_grid_transform();
      Core::Transform trans = grid.get_inverse();
      int nx = static_cast<int>( grid.get_nx() ); 
      int ny = static_cast<int>( grid.get_ny() ); 
      int nz = static_cast<int>( grid.get_nz() ); 
      
      unsigned short* data = reinterpret_cast<unsigned short*>( output_datablock->get_data() );
      for ( size_t i = 0; i < this->seeds_.size(); ++i )
      {   
        Core::Point location = trans * seeds_[ i ];
        int x = static_cast<int>( Core::Round( location.x() ) );
        int y = static_cast<int>( Core::Round( location.y() ) );
        int z = static_cast<int>( Core::Round( location.z() ) );
        
        if ( x >= 0 && y >= 0 && z >= 0 && x < nx && y < ny && z < nz )
        {
          unsigned short val = data[ output_datablock->to_index( static_cast<size_t>( x ), 
            static_cast<size_t>( y ), static_cast<size_t>( z ) ) ];
          if ( val ) lut[ val ] = 1;
        }
      }
      
      if ( this->mask_layer_ )
      {
        Core::MaskDataBlockHandle mask_handle = 
          dynamic_cast<MaskLayer*>( this->mask_layer_.get() )->
          get_mask_volume()->get_mask_data_block();
          
        unsigned char mask_value = mask_handle->get_mask_value();
        size_t size = mask_handle->get_size();
        unsigned char* mask_data = mask_handle->get_mask_data();
        
        Core::DataBlock::shared_lock_type lock( mask_handle->get_mutex() );
        if ( this->invert_mask_ )
        {
          for ( size_t j = 0; j < size; j++ )
          {
            if ( ! ( mask_data[ j ] & mask_value && data[ j ] ) ) lut[ data[ j ] ] = 1;
          }     
        }
        else
        {
          for ( size_t j = 0; j < size; j++ )
          {
            if ( ( mask_data[ j ] & mask_value && data[ j ] ) ) lut[ data[ j ] ] = 1;
          }           
        
        }
      }

      this->dst_layer_->update_progress_signal_( 0.80 );
      if ( this->check_abort() )
      {
        return;
      }

      size_t size = output_datablock->get_size();
      for ( size_t j = 0; j < size; j++ )
      {
        data[ j ] = lut[ data[ j ] ];
      }
    }

    this->dst_layer_->update_progress_signal_( .90 );
    if ( this->check_abort() )
    {
      return;
    }
    
    Core::MaskDataBlockHandle mask_datablock;
    if ( ! ( Core::MaskDataBlockManager::Convert( output_datablock, 
      this->dst_layer_->get_grid_transform(), mask_datablock ) ) )  
    {
      this->report_error("Could not allocate enough memory.");
      return;
    }
      
    this->dst_layer_->update_progress_signal_( 1.0 );
    
    this->dispatch_insert_mask_volume_into_layer( this->dst_layer_,
      Core::MaskVolumeHandle( new Core::MaskVolume(
      this->dst_layer_->get_grid_transform(), mask_datablock ) ) );
      
  }
  SCI_END_ITK_RUN()

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "ConnectedComponent Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "ConnectedComponent";  
  }
};


bool ActionConnectedComponentFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<ConnectedComponentFilterAlgo> algo( new ConnectedComponentFilterAlgo );

  // Find the handle to the layer
  algo->set_sandbox( this->sandbox_ );
  algo->src_layer_ = LayerManager::FindLayer( this->target_layer_, this->sandbox_ );
  algo->mask_layer_ = LayerManager::FindLayer( this->mask_, this->sandbox_ );
  
  // We definitely need a source layer, so make sure it exists
  if ( !algo->src_layer_ ) return false;
  
  // If there is a mask layer it needs to be locked for use
  if ( algo->mask_layer_ )
  {
    algo->lock_for_use( algo->mask_layer_ );
  }
  
  // Copy parameters to the algorithm
  algo->invert_mask_ = this->invert_mask_;
  algo->seeds_ = this->seeds_;
  
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

  // Start the filter on a separate thread.
  Core::Runnable::Start( algo );

  return true;
}

void ActionConnectedComponentFilter::Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, const std::vector< Core::Point >& seeds, bool replace,
    std::string mask, bool invert_mask )
{ 
  // Create a new action
  ActionConnectedComponentFilter* action = new ActionConnectedComponentFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->seeds_ = seeds;
  action->replace_ = replace;
  action->mask_ = mask;
  action->invert_mask_ = invert_mask;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
