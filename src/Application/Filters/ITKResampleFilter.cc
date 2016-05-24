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

#include <Application/Filters/ITKResampleFilter.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkIdentityTransform.h>
#include <itkResampleImageFilter.h>

using namespace Filter;
using namespace Seg3D;
using namespace Core;

const std::string ITKResampleFilter::LINEAR_C( "linear" );
const std::string ITKResampleFilter::B_SPLINE_C( "bspline" );
const std::string ITKResampleFilter::NEAREST_NEIGHBOR_C( "nn" );

ITKResampleFilter::ITKResampleFilter( const std::string& interpolator, int spline_order, bool replace, bool crop, const std::string& padding, Point range_min, Point range_max, SandboxID sandbox )
: replace_(replace),
  crop_(crop),
  padding_(padding),
  padding_only_(false),
  interpolator_(interpolator),
  spline_order_(spline_order),
  range_min_(range_min),
  range_max_(range_max),
  data_type_(DataType::FLOAT_E)
{
  this->set_sandbox( sandbox );
}

bool ITKResampleFilter::setup_layers( const std::vector< std::string >& layer_ids,
                                      bool match_grid_transform, const GridTransform& grid_transform,
                                      unsigned int dimX, unsigned int dimY, unsigned int dimZ )
{
  this->dims_[ 0 ] = dimX;
  this->dims_[ 1 ] = dimY;
  this->dims_[ 2 ] = dimZ;

  size_t num_layers = layer_ids.size();
  this->src_layers_.resize( num_layers );
  this->dst_layers_.resize( num_layers );
  this->output_transforms_.resize( num_layers );
  this->dst_layer_ids_.resize( num_layers );

  for ( size_t i = 0; i < num_layers; ++i )
  {
    this->find_layer( layer_ids[ i ], this->src_layers_[ i ] );
    if ( this->replace_ )
    {
      this->lock_for_deletion( this->src_layers_[ i ] );
    }
    else
    {
      this->lock_for_use( this->src_layers_[ i ] );
    }

    if ( match_grid_transform )
    {
      this->output_transforms_[ i ] = grid_transform;
      this->output_transforms_[ i ].set_originally_node_centered( this->src_layers_[ i ]->get_grid_transform().get_originally_node_centered() );
    }
    else
    {
      GridTransform grid_transform = this->src_layers_[ i ]->get_grid_transform();
      Transform new_transform;

      double spacing_x = grid_transform.spacing_x() * grid_transform.get_nx() / static_cast<double>( this->dims_[0] );
      double spacing_y = grid_transform.spacing_y() * grid_transform.get_ny() / static_cast<double>( this->dims_[1] );
      double spacing_z = grid_transform.spacing_z() * grid_transform.get_nz() / static_cast<double>( this->dims_[2] );

      new_transform = Transform( grid_transform.get_origin(),
                                 Core::Vector( spacing_x, 0.0 , 0.0 ),
                                 Core::Vector( 0.0, spacing_y, 0.0 ),
                                 Core::Vector( 0.0, 0.0, spacing_z ) );
      this->output_transforms_[ i ] = GridTransform( this->dims_[ 0 ], this->dims_[ 1 ], this->dims_[ 2 ], new_transform );
      this->output_transforms_[ i ].set_originally_node_centered( this->src_layers_[ i ]->get_grid_transform().get_originally_node_centered() );
    }

    switch ( this->src_layers_[ i ]->get_type() )
    {
      case Core::VolumeType::DATA_E:
        this->create_and_lock_data_layer( this->output_transforms_[ i ], this->src_layers_[ i ], this->dst_layers_[ i ] );
        break;
      case Core::VolumeType::MASK_E:
        this->create_and_lock_mask_layer( this->output_transforms_[ i ], this->src_layers_[ i ], this->dst_layers_[ i ] );
        static_cast< MaskLayer* >( this->dst_layers_[ i ].get() )->color_state_->set( static_cast< MaskLayer* >( this->src_layers_[ i ].get() )->color_state_->get() );
        break;
      default:
        CORE_LOG_ERROR( "Unsupported volume type." );
        return false;
    }

    if ( ! this->dst_layers_[ i ] )
    {
      CORE_LOG_ERROR( "Could not allocate enough memory." );
      return false;
    }

    dst_layer_ids_[ i ] = this->dst_layers_[ i ]->get_layer_id();
  }
  this->data_type_ = this->src_layers_[ 0 ]->get_data_type();

  return true;
}

void ITKResampleFilter::pad_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  DataBlockHandle output_datablock = this->pad_internals_->pad_and_crop_data_layer( input, output, this->shared_from_this() );

  if ( ! this->check_abort() )
  {
    GridTransform output_grid_transform = output->get_grid_transform();
    output_grid_transform.set_originally_node_centered( input->get_grid_transform().get_originally_node_centered() );

    this->dispatch_insert_data_volume_into_layer( output, DataVolumeHandle( new DataVolume( output_grid_transform, output_datablock ) ), true );
    this->dispatch_unlock_layer( output );
    if ( this->replace_ )
    {
      this->dispatch_delete_layer( input );
    }
    else
    {
      this->dispatch_unlock_layer( input );
    }
  }

  return;
}

void ITKResampleFilter::pad_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
{
  DataBlockHandle output_mask = this->pad_internals_->pad_and_crop_mask_layer( input, output, this->shared_from_this() );

  if ( ! this->check_abort() )
  {
    MaskDataBlockHandle dst_mask_data_block;
    if ( ! MaskDataBlockManager::Convert( output_mask, output->get_grid_transform(), dst_mask_data_block ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
    GridTransform output_grid_transform = output->get_grid_transform();
    output_grid_transform.set_originally_node_centered( input->get_grid_transform().get_originally_node_centered() );

    MaskVolumeHandle mask_volume( new MaskVolume( output_grid_transform, dst_mask_data_block ) );
    this->dispatch_insert_mask_volume_into_layer( output, mask_volume );
    this->dispatch_unlock_layer( output );
    if ( this->replace_ )
    {
      this->dispatch_delete_layer( input );
    }
    else
    {
      this->dispatch_unlock_layer( input );
    }
  }

  return;
}

template< class VALUE_TYPE >
void ITKResampleFilter::typed_run_filter()
{
  typedef double SCALAR_TYPE;

  this->pad_internals_.reset( new PadFilterInternals( this->src_layers_[0], this->dst_layers_[0], this->padding_ ) );
  if ( this->crop_ )
  {
    this->padding_only_ = this->pad_internals_->detect_padding_only();
  }

  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    this->current_output_transform_ = this->output_transforms_[ i ];

    if ( this->src_layers_[ i ]->get_type() == VolumeType::DATA_E )
    {
      DataLayerHandle src_data_layer = boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] );
      if ( ! src_data_layer )
      {
        this->report_error("Error obtaining source data layer.");
        return;
      }

      if ( this->padding_only_ )
      {
        DataLayerHandle dst_data_layer = boost::dynamic_pointer_cast< DataLayer >( this->dst_layers_[ i ] );
        if ( ! dst_data_layer )
        {
          this->report_error("Error obtaining destination data layer.");
          return;
        }

        this->pad_data_layer( src_data_layer, dst_data_layer );
        return;
      }

      typedef itk::Image< VALUE_TYPE, 3 > TYPED_IMAGE_TYPE;
      typedef ITKImageDataT< VALUE_TYPE > TYPED_CONTAINER_TYPE;

      // Define the type of filter that we use.
      typedef itk::ResampleImageFilter< TYPED_IMAGE_TYPE, TYPED_IMAGE_TYPE > filter_type;

      // Retrieve the image as an itk image from the underlying data structure
      // NOTE: This only does wrapping and does not regenerate the data.
      typename ITKImageDataT< VALUE_TYPE >::Handle input_image;
      this->get_itk_image_from_layer< VALUE_TYPE >( this->src_layers_[ i ], input_image );

      const typename TYPED_IMAGE_TYPE::PointType& origin = input_image->get_image()->GetOrigin();
      typename TYPED_IMAGE_TYPE::DirectionType direction;
      direction.SetIdentity();

      typename TYPED_IMAGE_TYPE::SizeType size;
      size[0] = dims_[0];
      size[1] = dims_[1];
      size[2] = dims_[2];

      typename TYPED_IMAGE_TYPE::SpacingType spacing;
      spacing[0] = this->current_output_transform_.spacing_x();
      spacing[1] = this->current_output_transform_.spacing_y();
      spacing[2] = this->current_output_transform_.spacing_z();

      // Create a new ITK filter instantiation.
      typename filter_type::Pointer filter = filter_type::New();

      // Relay abort and progress information to the layer that is executing the filter.
      this->forward_abort_to_filter( filter, this->dst_layers_[ i ] );
      this->observe_itk_progress( filter, this->dst_layers_[ i ], 0.0, 0.75 );

      // Setup the filter parameters that we do not want to change.
      filter->SetOutputOrigin( origin );
      filter->SetOutputSpacing( spacing );
      filter->SetSize( size );
      filter->SetOutputDirection( direction );

      VALUE_TYPE defaultPixelValue = 0;
      if ( this->padding_ == PadValues::MIN_C )
      {
        defaultPixelValue = static_cast< VALUE_TYPE >(src_data_layer->get_data_volume()->get_data_block()->get_min());
      }
      else if ( this->padding_ == PadValues::MAX_C )
      {
        defaultPixelValue = static_cast< VALUE_TYPE >(src_data_layer->get_data_volume()->get_data_block()->get_max());
      }
      filter->SetDefaultPixelValue(defaultPixelValue);

      typedef itk::LinearInterpolateImageFunction< TYPED_IMAGE_TYPE, SCALAR_TYPE > LinearInterpolatorType;
      typedef itk::BSplineInterpolateImageFunction< TYPED_IMAGE_TYPE, SCALAR_TYPE > BSplineInterpolatorType;
      typedef itk::NearestNeighborInterpolateImageFunction< TYPED_IMAGE_TYPE, SCALAR_TYPE > NearestNeighborInterpolatorType;

      if ( this->interpolator_ == ITKResampleFilter::LINEAR_C )
      {
        typename LinearInterpolatorType::Pointer linear_interp = LinearInterpolatorType::New();
        filter->SetInterpolator( linear_interp );
      }
      else if ( this->interpolator_ == ITKResampleFilter::B_SPLINE_C )
      {
        typename BSplineInterpolatorType::Pointer bspline_interp = BSplineInterpolatorType::New();
        bspline_interp->SetSplineOrder( this->spline_order_ );
        filter->SetInterpolator( bspline_interp );
      }
      else // NEAREST_NEIGHBOR_C
      {
        typename NearestNeighborInterpolatorType::Pointer nn_interp = NearestNeighborInterpolatorType::New();
        filter->SetInterpolator( nn_interp );
      }
      filter->SetInput( input_image->get_image() );
      filter->UpdateLargestPossibleRegion();

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

      this->insert_itk_image_into_layer< VALUE_TYPE >( this->dst_layers_[ i ], filter->GetOutput() );
      this->dispatch_unlock_layer( this->dst_layers_[ i ] );
      if ( this->replace_ )
      {
        this->dispatch_delete_layer( this->src_layers_[ i ] );
      }
      else
      {
        this->dispatch_unlock_layer( this->src_layers_[ i ] );
      }
    }
    else if ( this->src_layers_[ i ]->get_type() == VolumeType::MASK_E )
    {
      if ( this->padding_only_ )
      {
        MaskLayerHandle src_mask_layer = boost::dynamic_pointer_cast< MaskLayer >( this->src_layers_[ i ] );
        if ( ! src_mask_layer )
        {
          this->report_error("Error obtaining source mask layer.");
          return;
        }
        
        MaskLayerHandle dst_mask_layer = boost::dynamic_pointer_cast< MaskLayer >( this->dst_layers_[ i ] );
        if ( ! dst_mask_layer )
        {
          this->report_error("Error obtaining destination mask layer.");
          return;
        }
        
        this->pad_mask_layer( src_mask_layer, dst_mask_layer );
        return;
      }

      typedef ITKImageDataT< unsigned char > TYPED_CONTAINER_TYPE;

      // Define the type of filter that we use.
      typedef itk::ResampleImageFilter< UCHAR_IMAGE_TYPE, UCHAR_IMAGE_TYPE > filter_type;

      // Retrieve the image as an itk image from the underlying data structure
      // NOTE: This only does wrapping and does not regenerate the data.
      typename TYPED_CONTAINER_TYPE::Handle input_image;
      this->get_itk_image_from_layer< unsigned char >( this->src_layers_[ i ], input_image );

      const typename UCHAR_IMAGE_TYPE::PointType& origin = input_image->get_image()->GetOrigin();
      typename UCHAR_IMAGE_TYPE::DirectionType direction;
      direction.SetIdentity();

      typename UCHAR_IMAGE_TYPE::SizeType size;
      size[0] = dims_[0];
      size[1] = dims_[1];
      size[2] = dims_[2];

      typename UCHAR_IMAGE_TYPE::SpacingType spacing;
      spacing[0] = this->current_output_transform_.spacing_x();
      spacing[1] = this->current_output_transform_.spacing_y();
      spacing[2] = this->current_output_transform_.spacing_z();

      // Create a new ITK filter instantiation.
      typename filter_type::Pointer filter = filter_type::New();

      // Relay abort and progress information to the layer that is executing the filter.
      this->forward_abort_to_filter( filter, this->dst_layers_[ i ] );
      this->observe_itk_progress( filter, this->dst_layers_[ i ], 0.0, 0.75 );

      // Setup the filter parameters that we do not want to change.
      filter->SetOutputOrigin( origin );
      filter->SetOutputSpacing( spacing );
      filter->SetSize( size );
      filter->SetOutputDirection( direction );

      typedef itk::LinearInterpolateImageFunction< UCHAR_IMAGE_TYPE, SCALAR_TYPE > LinearInterpolatorType;
      typedef itk::BSplineInterpolateImageFunction< UCHAR_IMAGE_TYPE, SCALAR_TYPE > BSplineInterpolatorType;
      typedef itk::NearestNeighborInterpolateImageFunction< UCHAR_IMAGE_TYPE, SCALAR_TYPE > NearestNeighborInterpolatorType;

      if ( this->interpolator_ == ITKResampleFilter::LINEAR_C )
      {
        typename LinearInterpolatorType::Pointer linear_interp = LinearInterpolatorType::New();
        filter->SetInterpolator( linear_interp );
      }
      else if ( this->interpolator_ == ITKResampleFilter::B_SPLINE_C )
      {
        typename BSplineInterpolatorType::Pointer bspline_interp = BSplineInterpolatorType::New();
        bspline_interp->SetSplineOrder( this->spline_order_ );
        filter->SetInterpolator( bspline_interp );
      }
      else // NEAREST_NEIGHBOR_C
      {
        typename NearestNeighborInterpolatorType::Pointer nn_interp = NearestNeighborInterpolatorType::New();
        filter->SetInterpolator( nn_interp );
      }
      filter->SetInput( input_image->get_image() );
      filter->UpdateLargestPossibleRegion();

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

      this->insert_itk_image_into_layer< unsigned char >( this->dst_layers_[ i ], filter->GetOutput() );
      if ( this->replace_ )
      {
        this->dispatch_delete_layer( this->src_layers_[ i ] );
      }
      else
      {
        this->dispatch_unlock_layer( this->src_layers_[ i ] );
      }
    }
    else
    {
      CORE_LOG_WARNING("Attempting to resample unsupported layer type.");
    }

    if ( ! this->dst_layers_[ i ] )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }

    if ( this->check_abort() ) break;
  }
}
