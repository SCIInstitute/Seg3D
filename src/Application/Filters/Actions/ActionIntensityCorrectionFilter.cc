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
#include <itkDiscreteGaussianImageFilter.h>
#include <itkGradientImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>

  
// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Filters/Actions/ActionIntensityCorrectionFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, IntensityCorrectionFilter )

namespace Seg3D
{

bool ActionIntensityCorrectionFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::DATA_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, 
    this->replace_, context, this->sandbox_ ) ) return false;
    
  // If the number of iterations is lower than one, we cannot run the filter
  if( this->order_ < 1  || this->order_ > 4)
  {
    context->report_error( "The polynomial order needs to be between 1 and 4." );
    return false;
  }
  
  // Conductance needs to be a positive number
  if( this->edge_ < 0.0 )
  {
    context->report_error( "The sensitivity needs to be larger than zero." );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class IntensityCorrectionFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;
  
  int order_;
  double edge_;
  bool preserve_data_format_;

  float getAx( float w, float nx, float ny, float nz, int order_index); 
  float getAy( float w, float nx, float ny, float nz, int order_index);
  float getAz( float w, float nx, float ny, float nz, int order_index);

  // RUN:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.
  SCI_BEGIN_TYPED_ITK_RUN( this->src_layer_->get_data_type() )
  {
    // Discrete gaussian blur of the image first to reduce noise.
    // TODO:  Add median filter, no blur options.
    typedef itk::DiscreteGaussianImageFilter< TYPED_IMAGE_TYPE, FLOAT_IMAGE_TYPE > DGFilterType;
    
    // Retrieve the image as an itk image from the underlying data structure
    // NOTE: This only does wrapping and does not regenerate the data.
    typename TYPED_CONTAINER_TYPE::Handle input_image; 
    this->get_itk_image_from_layer<VALUE_TYPE>( this->src_layer_, input_image );
  
    Core::DataBlockHandle input_data_block = Core::ITKDataBlock::New( input_image );
  
    if ( !input_data_block )
    {
      this->report_error( "Could not allocate enough memory." );
      return;   
    }
  
    // Build the Discrete Gaussian filter
    typename DGFilterType::Pointer dgfilter = DGFilterType::New();
    
    dgfilter->SetInput( input_image->get_image() );
    dgfilter->SetVariance(1.0);
    dgfilter->SetMaximumKernelWidth(8);
    dgfilter->SetUseImageSpacingOff();
    
    this->forward_abort_to_filter( dgfilter, this->dst_layer_ );
    this->observe_itk_progress( dgfilter, this->dst_layer_, 0.0, 0.05 );

    try
    {
      dgfilter->Update();
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

    if ( this->check_abort() ) return;

    typename FLOAT_CONTAINER_TYPE::Handle dg_image( new FLOAT_CONTAINER_TYPE( 
      dgfilter->GetOutput() ) );

    if ( !dg_image )
    {
      this->report_error( "Could not allocate enough memory." );
      return;     
    }

    typename Core::DataBlockHandle dg_data_block = Core::ITKDataBlock::New( dg_image );
    float data_min = std::numeric_limits<float>::max();
    float data_max = std::numeric_limits<float>::min();

    {
      float* data = reinterpret_cast<float*>( dg_data_block->get_data() );
      size_t size = dg_data_block->get_size();
      for ( size_t j = 0; j < size; j++ )
      {
        if ( data[ j ] < data_min ) data_min = data[ j ];
        if ( data[ j ] > data_max ) data_max = data[ j ];
      }
      for ( size_t j = 0; j < size; j++ ) data[ j ] = log( data[ j ] + 0.01f - data_min );
    }
    
    if ( this->check_abort() ) return;
   
    // Take the Gradient Magnitude of the log image.
    typedef itk::GradientMagnitudeImageFilter< FLOAT_IMAGE_TYPE, FLOAT_IMAGE_TYPE > GMagFilterType;
    GMagFilterType::Pointer gmag_filter = GMagFilterType::New();
    
    gmag_filter->SetInput( dg_image->get_image() );
    
    this->forward_abort_to_filter( gmag_filter, this->dst_layer_ );
    this->observe_itk_progress( gmag_filter, this->dst_layer_, 0.1, 0.05 );

    try
    {
      gmag_filter->Update();
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

    if ( this->check_abort() ) return;
    
    typename FLOAT_CONTAINER_TYPE::Handle gmag_image( new FLOAT_CONTAINER_TYPE( 
      gmag_filter->GetOutput() ) );

    if ( !gmag_image )
    {
      this->report_error( "Could not allocate enough memory." );
      return;     
    }
  
    Core::DataBlockHandle gmag_data_block = Core::ITKDataBlock::New( gmag_image );

    if ( !gmag_data_block )
    {
      this->report_error( "Could not allocate enough memory." );
      return;     
    }
    
    {
      float* data = reinterpret_cast<float*>( gmag_data_block->get_data() );
      size_t size = gmag_data_block->get_size();
      const float inv_edge_sqr = 1.0 / ( this->edge_ * this->edge_ );
      for ( size_t j = 0; j < size; j++ ) data[ j ] = exp( - data[ j ] * inv_edge_sqr );
    }
    
    if ( this->check_abort() ) return;

    // Take the Gradient Magnitude of the log image.
    typedef itk::GradientImageFilter<FLOAT_IMAGE_TYPE> GradientFilterType;
    typedef GradientFilterType::OutputImageType GradientImageType;
    typedef GradientImageType::PixelType GradientPixelType;

    GradientFilterType::Pointer gradient_filter = GradientFilterType::New();
  
    gradient_filter->SetInput( gmag_image->get_image() );

    this->forward_abort_to_filter( gradient_filter, this->dst_layer_ );
    this->observe_itk_progress( gradient_filter, this->dst_layer_, 0.2, 0.05 );

    try
    {
      gradient_filter->Update();
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

    if ( this->check_abort() ) return;

    GradientImageType::Pointer gradient_image = gradient_filter->GetOutput();
  
    // Create the matrices.

    size_t size = input_image->get_size();
    size_t size_x = input_image->get_nx();
    size_t size_y = input_image->get_ny();
    size_t size_z = input_image->get_nz();
    
    float xscale = static_cast<float>( size_x + size_y + size_z ) / 8.0f;
    float yscale = static_cast<float>( size_x + size_y + size_z ) / 8.0f;
    float zscale = static_cast<float>( size_x + size_y + size_z ) / 8.0f;

    float xcenter = static_cast<float>( size_x ) / 2.0;
    float ycenter = static_cast<float>( size_y ) / 2.0;
    float zcenter = static_cast<float>( size_z ) / 2.0;

    // Build F (U?)
    try
    {
      vnl_matrix< float > U( 3 * size, 1, 0.0f );
      
      typedef itk::ImageRegionIterator< GradientImageType > GradientImageIterator;
      GradientImageIterator grad_iter(gradient_image, gradient_image->GetLargestPossibleRegion());

      float* W = reinterpret_cast<float*>( gmag_data_block->get_data() );
      float* res = reinterpret_cast<float*>( dg_data_block->get_data() );
      VALUE_TYPE* input = reinterpret_cast<VALUE_TYPE*>( input_data_block->get_data() );

      size_t j = 0;
      
      while ( !grad_iter.IsAtEnd() )
      {
        GradientPixelType v = grad_iter.Get();

        U[j         ][0] = W[j] * v[0] * xscale;
        U[j + size  ][0] = W[j] * v[1] * yscale;
        U[j + 2*size][0] = W[j] * v[2] * zscale;

        ++j;
        ++grad_iter;
      }
    
      /// Clear memory for image
      gradient_image = 0;
    
      // Build A
      static int order_counts[ 5 ] = { 0, 3, 9, 19, 34 };
      const int  order = order_counts[ this->order_ ];
      vnl_matrix<float> ATA( order, order, 0 );
      vnl_matrix<float> uT( order, 1, 0 );

      //
      //  loop over all the pixels in the image and calculate ATA in-place.
      //
      for( size_t z = 0; z < size_z; z++ )
      {
        this->dst_layer_->update_progress( 
          static_cast<float>( z )/static_cast<float>( size_z ), 0.25f, 0.35f );
        if ( this->check_abort() ) return;

        for ( size_t y = 0; y < size_y; y++ )
        {
          for ( size_t x = 0; x < size_x; x++ )
          {
            const float nx = ( static_cast<float>( x ) - xcenter ) / xscale;
            const float ny = ( static_cast<float>( y ) - ycenter ) / yscale;
            const float nz = ( static_cast<float>( z ) - zcenter ) / zscale;

            const size_t row_num = ( ( size_x * size_y * z) + ( size_x * y ) + x );
            const float w = W[ row_num ];

            for( int i = 0; i < order; i++ )
            {
              for( int j = 0; j < order; j++ )
              {   
                float val = ATA(i, j);

                const float ax = getAx( w, nx, ny, nz, j);
                const float axt = getAx( w, nx, ny, nz, i);

                val += ax * axt;

                const float ay = getAy( w, nx, ny, nz, j);
                const float ayt = getAy( w, nx, ny, nz, i);

                val += ay * ayt;

                const float az = getAz( w, nx, ny, nz, j);
                const float azt = getAz( w, nx, ny, nz, i);

                val += az * azt;

                if( j == 0 ) // only need to do this once
                {
                  float u_val = uT( i, 0 );
                  const size_t h = size_x * size_y * z + size_x * y + x;
                  u_val += ( axt * U( h, 0 ) ) +
                    ( ayt * U( h + size, 0 ) ) +
                    ( azt * U( h + (2 * size ), 0 ) );
                  uT( i, 0 ) = u_val;

                }

                ATA(i, j) = val;
              }
            }
          }
        }
      }

      if ( this->check_abort() ) return;
      vnl_matrix< float > ATA_inv = vnl_matrix_inverse<float>(ATA);
      this->dst_layer_->update_progress( 0.7f );

      if ( this->check_abort() ) return;
      vnl_matrix< float > alphaM = ATA_inv * uT;

      this->dst_layer_->update_progress( 0.85f );
      if ( this->check_abort() ) return;

      vnl_vector< float > alpha = alphaM.get_column(0);
      if ( this->check_abort() ) return;

      this->dst_layer_->update_progress( 0.9f );

      // Create the source - illumination field.
      size_t i = 0;
      
      float new_data_min = std::numeric_limits<float>::max();
      float new_data_max = std::numeric_limits<float>::min();
      
      for ( size_t z = 0; z < size_z; z++ )
      {
        for ( size_t y = 0; y < size_y; y++ ) 
        {
          for (size_t x = 0; x < size_x; x++)
          {
            const float nx = ( static_cast<float>( x ) - xcenter) / xscale;
            const float ny = ( static_cast<float>( y ) - ycenter) / yscale;
            const float nz = ( static_cast<float>( z ) - zcenter) / zscale;

            float val = alpha[0] * nx + alpha[1] * ny + alpha[2] * nz;

            if ( this->order_ > 1)
            {
              val +=
                alpha[3] * nx * nx +
                alpha[4] * nx * ny +
                alpha[5] * nx * nz +
                alpha[6] * ny * ny +
                alpha[7] * ny * nz +
                alpha[8] * nz * nz;

              if ( this->order_ > 2)
              {
                val +=
                  alpha[9] * nx * nx * nx +
                  alpha[10] * nx * nx * ny +
                  alpha[11] * nx * nx * nz +
                  alpha[12] * nx * ny * ny +
                  alpha[13] * nx * ny * nz +
                  alpha[14] * nx * nz * nz +
                  alpha[15] * ny * ny * ny +
                  alpha[16] * ny * ny * nz +
                  alpha[17] * ny * nz * nz +
                  alpha[18] * nz * nz * nz;
             
                if ( this->order_ > 3 )
                {
                  val +=
                  alpha[19] * nx * nx * nx * nx +
                  alpha[20] * nx * nx * nx * ny +
                  alpha[21] * nx * nx * nx * nz +
                  alpha[22] * nx * nx * ny * ny +
                  alpha[23] * nx * nx * ny * nz +
                  alpha[24] * nx * nx * nz * nz +
                  alpha[25] * nx * ny * ny * ny +
                  alpha[26] * nx * ny * ny * nz +
                  alpha[27] * nx * ny * nz * nz +
                  alpha[28] * nx * nz * nz * nz +
                  alpha[29] * ny * ny * ny * ny +
                  alpha[30] * ny * ny * ny * nz +
                  alpha[31] * ny * ny * nz * nz +
                  alpha[32] * ny * nz * nz * nz +
                  alpha[33] * nz * nz * nz * nz;
                }
              } 
            }
          
            res[ i ] = ( static_cast<float>( input[ i ] ) - data_min ) * 
              exp( -val ) + data_min;
            if ( res[ i ] < new_data_min ) new_data_min = res[ i ];
            if ( res[ i ] > new_data_max ) new_data_max = res[ i ];
            i++;
          }
        }
      } 
    
    
      float factor = ( data_max - data_min ) / ( new_data_max - new_data_min );
      for ( size_t j = 0; j < size; j++ ) 
      {
        res[ j ] = ( factor * ( res[ j ] - new_data_min ) ) + data_min;
      }
    }
    catch ( ... )
    {
      if ( this->check_abort() )
      {
        this->report_error( "Filter was aborted." );
        return;
      }
      this->report_error( "Could not allocate enough memory." );
      return;   
    }
    
    if ( this->check_abort() ) return;
    this->dst_layer_->update_progress( 0.99f );
      
    // If we want to preserve the data type we convert the data before inserting it back.
    // NOTE: Conversion is done on the filter thread and insertion is done on the application
    // thread.
    if ( this->preserve_data_format_ )
    {
      this->convert_and_insert_itk_image_pointer_into_layer<float>( this->dst_layer_, 
        dg_image->get_image(), this->src_layer_->get_data_type() );       
    }
    else
    {
      this->insert_itk_image_pointer_into_layer<float>( this->dst_layer_, 
        dg_image->get_image() );  
    }
  }
  SCI_END_TYPED_ITK_RUN()
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Intensity Correction Filter";
  }
  
  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "IntensCorr";  
  } 
};


float IntensityCorrectionFilterAlgo::getAx( float w, float nx, float ny, float nz, int order_index)
{
   switch( order_index )
   {
    case 0: return w;
    case 3: return w * 2.0f * nx;
    case 4: return w * ny;
    case 5: return w * nz;
    
    case 9: return w * 3.0f * nx * nx;
    case 10: return w * 2.0f * nx * ny;
    case 11: return w * 2.0f * nx * nz;
    case 12: return w * ny * ny;
    case 13: return w * ny * nz;
    case 14: return w * nz * nz;

    case 19: return w * 4.0f * nx * nx * nx;
    case 20: return w * 3.0f * nx * nx * ny;
    case 21: return w * 3.0f * nx * nx * nz;
    case 22: return w * 2.0f * nx * ny * ny;
    case 23: return w * 2.0f * nx * ny * nz;
    case 24: return w * 2.0f * nx * nz * nz;
    case 25: return w * ny * ny * ny;
    case 26: return w * ny * ny * nz;
    case 27: return w * ny * nz * nz;
    case 28: return w * nz * nz * nz;
    default: return 0.0f;
   }
}

float IntensityCorrectionFilterAlgo::getAy( float w, float nx, float ny, float nz, int order_index)
{
   switch( order_index )
   {
    case 1: return w;

    case 4: return w * nx;
    case 6: return w * 2.0f * ny;
    case 7: return w * nz;

    case 10: return w * nx * nx;
    case 12: return w * 2.0f * nx * ny;
    case 13: return w * nx * nz;
    case 15: return w * 3.0f * ny * ny;
    case 16: return w * 2.0f * ny * nz;
    case 17: return w * nz * nz;

    case 20: return w * nx * nx * nx;
    case 22: return w * 2.0f * nx * nx * ny;
    case 23: return w * nx * nx * nz;
    case 25: return w * 3.0f * ny * ny * nx;
    case 26: return w * 2.0f * nx * ny * nz;
    case 27: return w * nx * nz * nz;
    case 29: return w * 4.0f * ny * ny * ny;
    case 30: return w * 3.0f * ny * ny * nz;
    case 31: return w * 2.0f * ny * nz * nz;
    case 32: return w * nz * nz * nz;
    default: return 0.0f;
   }
}

float IntensityCorrectionFilterAlgo::getAz( float w, float nx, float ny, float nz, int order_index)
{
   switch( order_index )
   {
    case 2: return w;

    case 5: return w * nx;
    case 7: return w * ny;
    case 8: return w * 2.0f * nz;

    case 11: return w * nx * nx;
    case 13: return w * nx * ny;
    case 14: return w * 2.0f * nx * nz;
    case 16: return w * ny * ny;
    case 17: return w * 2.0f * ny * nz;
    case 18: return w * 3.0f * nz * nz;

    case 21: return w * nx * nx * nx;
    case 23: return w * nx * nx * ny;
    case 24: return w * 2.0f * nx * nx * nz;
    case 26: return w * nx * ny * ny;
    case 27: return w * 2.0f * nx * ny * nz;
    case 28: return w * 3.0f * nx * nz * nz;
    case 30: return w * ny * ny * ny;
    case 31: return w * 2.0f * ny * ny * nz;
    case 32: return w * 3.0f * ny * nz * nz;
    case 33: return w * 4.0f * nz * nz * nz;
    
    default: return 0.0f;
   }
}


bool ActionIntensityCorrectionFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<IntensityCorrectionFilterAlgo> algo(
    new IntensityCorrectionFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->set_sandbox( this->sandbox_ );
  algo->order_ = this->order_;
  algo->edge_  = this->edge_;
  algo->preserve_data_format_ = this->preserve_data_format_;

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

void ActionIntensityCorrectionFilter::Dispatch( Core::ActionContextHandle context, 
  std::string layer_id, bool preserve_data_format, bool replace, int order, double edge )
{ 
  // Create a new action
  ActionIntensityCorrectionFilter* action = 
    new ActionIntensityCorrectionFilter;

  // Setup the parameters
  action->target_layer_ = layer_id;
  action->order_ = order;
  action->edge_ = edge;
  action->preserve_data_format_ = preserve_data_format;
  action->replace_ = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
