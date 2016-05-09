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
#include <Application/Filters/Actions/ActionFillHolesFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, FillHolesFilter )

namespace Seg3D
{

bool ActionFillHolesFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;
  
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

class FillHolesFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;

  std::vector< Core::Point > seeds_;
  
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
    // NOTE: Get the inverted version f the mask
    this->get_itk_image_from_layer<unsigned char>( this->src_layer_, input_image, true );
        
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

    Core::DataBlockHandle output_datablock;

    // As ITK filters generate an inconsistent abort behavior, we record our own abort flag
    // This one is set when the abort button is pressed and an abort is sent to ITK.
    if ( this->check_abort() ) return;

    if ( need_32bits )
    {
      filter = 0;
      // Define the type of filter that we use.
      typedef itk::ConnectedComponentImageFilter< 
        UCHAR_IMAGE_TYPE, UINT_IMAGE_TYPE > filter32_type;

      // Retrieve the image as an itk image from the underlying data structure
      // NOTE: This only does wrapping and does not regenerate the data.
      Core::ITKImageDataT<unsigned char>::Handle input_image; 
      // NOTE: Get the invertedd version f the mask
      this->get_itk_image_from_layer<unsigned char>( this->src_layer_, input_image, true );
          
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
      
      unsigned int max_label = filter32->GetObjectCount();
      std::vector<unsigned int> lut;

      try
      {
        lut.resize( max_label + 1, 1 );
      }
      catch( ... )
      {
        this->report_error( "Could not allocate enough memory." );
        return;   
      }
      
      Core::GridTransform grid = input_image->get_grid_transform();
      Core::Transform trans = grid.get_inverse();
      int nx = static_cast<int>( grid.get_nx() ); 
      int ny = static_cast<int>( grid.get_ny() ); 
      int nz = static_cast<int>( grid.get_nz() ); 
      
      unsigned int* data = reinterpret_cast<unsigned int*>( output_datablock->get_data() );
      unsigned int val;
      for ( size_t i = 0; i < this->seeds_.size(); ++i )
      {   
        Core::Point location = trans * seeds_[ i ];
        int x = static_cast<int>( Core::Round( location.x() ) );
        int y = static_cast<int>( Core::Round( location.y() ) );
        int z = static_cast<int>( Core::Round( location.z() ) );
        
        if ( x >= 0 && y >= 0 && z >= 0 && x < nx && y < ny && z < nz )
        {
          val = data[ output_datablock->to_index( static_cast<size_t>( x ), 
            static_cast<size_t>( y ), static_cast<size_t>( z ) ) ];
          if ( val ) lut[ val ] = 0;
        }
      }

      // Ensure that anything connected to the corners is not removed
      val = data[ output_datablock->to_index( 0, 0, 0 ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 0, 0 ) ];
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( 0, static_cast<size_t>( ny -1 ), 0 ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 
        static_cast<size_t>( ny -1 ), 0 ) ];
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( 0, 0, static_cast<size_t>( nz -1 ) ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 0, 
        static_cast<size_t>( nz -1 ) ) ];
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( 0, static_cast<size_t>( ny -1 ), 
        static_cast<size_t>( nz -1 ) ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 
        static_cast<size_t>( ny -1 ), static_cast<size_t>( nz -1 ) ) ];
      if ( val ) lut[ val ] = 0;
      
      lut[ 0 ] = 1;
      
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
      output_datablock = Core::ITKDataBlock::New( filter->GetOutput() );
      
      unsigned int max_label = filter->GetObjectCount();
      std::vector<unsigned short> lut;

      try
      {
        lut.resize( max_label + 1, 1 );
      }
      catch( ... )
      {
        this->report_error( "Could not allocate enough memory." );
        return;   
      }
      
      Core::GridTransform grid = input_image->get_grid_transform();
      Core::Transform trans = grid.get_inverse();
      int nx = static_cast<int>( grid.get_nx() ); 
      int ny = static_cast<int>( grid.get_ny() ); 
      int nz = static_cast<int>( grid.get_nz() ); 
      
      unsigned short* data = reinterpret_cast<unsigned short*>( output_datablock->get_data() );
      unsigned short val;
      for ( size_t i = 0; i < this->seeds_.size(); ++i )
      {   
        Core::Point location = trans * seeds_[ i ];
        int x = static_cast<int>( Core::Round( location.x() ) );
        int y = static_cast<int>( Core::Round( location.y() ) );
        int z = static_cast<int>( Core::Round( location.z() ) );
        
        if ( x >= 0 && y >= 0 && z >= 0 && x < nx && y < ny && z < nz )
        {
          val = data[ output_datablock->to_index( static_cast<size_t>( x ), 
            static_cast<size_t>( y ), static_cast<size_t>( z ) ) ];
          if ( val ) lut[ val ] = 0;
        }
      }

      // Ensure that anything connected to the corners is not removed
      val = data[ output_datablock->to_index( 0, 0, 0 ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 0, 0 ) ];
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( 0, static_cast<size_t>( ny -1 ), 0 ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 
        static_cast<size_t>( ny -1 ), 0 ) ];
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( 0, 0, static_cast<size_t>( nz -1 ) ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 0, 
        static_cast<size_t>( nz -1 ) ) ];
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( 0, static_cast<size_t>( ny -1 ), 
        static_cast<size_t>( nz -1 ) ) ]; 
      if ( val ) lut[ val ] = 0;
      val = data[ output_datablock->to_index( static_cast<size_t>( nx -1 ), 
        static_cast<size_t>( ny -1 ), static_cast<size_t>( nz -1 ) ) ];
      if ( val ) lut[ val ] = 0;
      
      lut[ 0 ] = 1;
      
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
    if ( !( Core::MaskDataBlockManager::Convert( output_datablock, 
      this->dst_layer_->get_grid_transform(), mask_datablock ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
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
    return "FillHoles Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "FillHoles"; 
  }
};


bool ActionFillHolesFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<FillHolesFilterAlgo> algo( new FillHolesFilterAlgo );
  algo->set_sandbox( this->sandbox_ );

  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
  {
    return false;
  }
  
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

void ActionFillHolesFilter::Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, const std::vector< Core::Point >& seeds, bool replace )
{ 
  // Create a new action
  ActionFillHolesFilter* action = new ActionFillHolesFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->seeds_ = seeds;
  action->replace_ = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
