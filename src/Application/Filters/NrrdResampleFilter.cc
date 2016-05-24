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

// Core includes
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>

#include <Application/Filters/NrrdResampleFilter.h>

// NOTE: The following functions are defined in resampleContext.c of teem library
extern "C" {
  extern int _nrrdResampleInputDimensionUpdate( NrrdResampleContext *rsmc );
  extern int _nrrdResampleInputCentersUpdate( NrrdResampleContext *rsmc );
  extern int _nrrdResampleInputSizesUpdate( NrrdResampleContext *rsmc );
  extern int _nrrdResampleLineAllocateUpdate( NrrdResampleContext *rsmc );
  extern int _nrrdResampleVectorAllocateUpdate( NrrdResampleContext *rsmc );
  extern int _nrrdResampleLineFillUpdate( NrrdResampleContext *rsmc );
  extern int _nrrdResampleVectorFillUpdate( NrrdResampleContext *rsmc );
  extern int _nrrdResamplePermutationUpdate( NrrdResampleContext *rsmc );
  extern void _nrrdResampleMinMaxFull( double *minP, double *maxP, int center, size_t size );
};

using namespace Filter;
using namespace Seg3D;
using namespace Core;

const std::string NrrdResampleFilter::BOX_C( "box" );
const std::string NrrdResampleFilter::TENT_C( "tent" );
const std::string NrrdResampleFilter::CUBIC_CR_C( "cubic_cr" );
const std::string NrrdResampleFilter::CUBIC_BS_C( "cubic_bs" );
const std::string NrrdResampleFilter::QUARTIC_C( "quartic" );
const std::string NrrdResampleFilter::GAUSSIAN_C( "gauss" );


NrrdResampleFilter::NrrdResampleFilter( const std::string& kernel, double gauss_sigma, double gauss_cutoff, bool replace, bool crop, const std::string& padding, Core::Point range_min, Core::Point range_max, SandboxID sandbox )
  : replace_(replace),
    crop_(crop),
    padding_(padding),
    padding_only_(false),
    range_min_(range_min),
    range_max_(range_max)
{
  this->data_kernel_ = nrrdKernelSpecNew();
  this->mask_kernel_ = nrrdKernelSpecNew();

  this->mask_kernel_->kernel = nrrdKernelCheap;
  this->mask_kernel_->parm[ 0 ] = 1.0;

  this->data_kernel_->parm[ 0 ] = 1.0;
  if ( kernel == NrrdResampleFilter::BOX_C )
  {
    this->data_kernel_->kernel = nrrdKernelBox;
  }
  else if ( kernel == NrrdResampleFilter::TENT_C )
  {
    this->data_kernel_->kernel = nrrdKernelTent;
  }
  else if ( kernel == NrrdResampleFilter::CUBIC_BS_C )
  {
    this->data_kernel_->kernel = nrrdKernelBCCubic;
    this->data_kernel_->parm[ 1 ] = 1.0;
    this->data_kernel_->parm[ 2 ] = 0.0;
  }
  else if ( kernel == NrrdResampleFilter::CUBIC_CR_C )
  {
    this->data_kernel_->kernel = nrrdKernelBCCubic;
    this->data_kernel_->parm[ 1 ] = 0.0;
    this->data_kernel_->parm[ 2 ] = 0.5;
  }
  else if ( kernel == NrrdResampleFilter::QUARTIC_C )
  {
    this->data_kernel_->kernel = nrrdKernelAQuartic;
    this->data_kernel_->parm[ 1 ] = 0.0834;
  }
  else
  {
    this->data_kernel_->kernel = nrrdKernelGaussian;
    this->data_kernel_->parm[ 0 ] = gauss_sigma;
    this->data_kernel_->parm[ 1 ] = gauss_cutoff;
  }

  this->set_sandbox( sandbox );
}

NrrdResampleFilter::~NrrdResampleFilter()
{
  nrrdKernelSpecNix( this->data_kernel_ );
  nrrdKernelSpecNix( this->mask_kernel_ );
  for( size_t i = 0; i < this->resample_contexts_.size(); i++ )
  {
    nrrdResampleContextNix( this->resample_contexts_[ i ] );
  }
}

bool NrrdResampleFilter::setup_layers(const std::vector< std::string >& layer_ids,
                                      bool match_grid_transform, const Core::GridTransform& grid_transform,
                                      unsigned int dimX, unsigned int dimY, unsigned int dimZ)
{
  this->dims_[ 0 ] = dimX;
  this->dims_[ 1 ] = dimY;
  this->dims_[ 2 ] = dimZ;

  size_t num_layers = layer_ids.size();
  this->src_layers_.resize( num_layers );
  this->dst_layers_.resize( num_layers );
  this->resample_contexts_.resize( num_layers );
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

    // Compute grid transform for the output layers
    this->resample_contexts_[ i ] = nrrdResampleContextNew();
    this->resample_contexts_[ i ]->verbose = 0;

    int error = AIR_FALSE;
    error |= nrrdResampleDefaultCenterSet( this->resample_contexts_[ i ], nrrdCenterCell );
    if (error)
    {
      CORE_LOG_ERROR( "Setting resampled center fail" );
      return false;
    }

    if ( match_grid_transform )
    {
      this->output_transforms_[ i ] = grid_transform;
      this->output_transforms_[ i ].set_originally_node_centered( this->src_layers_[ i ]->get_grid_transform().get_originally_node_centered() );
    }
    else
    {
      if (! this->compute_output_grid_transform( this->src_layers_[ i ], this->resample_contexts_[ i ], this->output_transforms_[ i ] ) )
      {
        CORE_LOG_ERROR( "Computing grid transform failed." );
        return false;
      }
    }

    switch ( this->src_layers_[ i ]->get_type() )
    {
      case Core::VolumeType::DATA_E:
        this->create_and_lock_data_layer( this->output_transforms_[ i ], this->src_layers_[ i ], this->dst_layers_[ i ] );
        break;
      case Core::VolumeType::MASK_E:
        this->create_and_lock_mask_layer( this->output_transforms_[ i ], this->src_layers_[ i ], this->dst_layers_[ i ] );
        static_cast< MaskLayer* >( this->dst_layers_[ i ].get() )->color_state_->set(
          static_cast< MaskLayer* >( this->src_layers_[ i ].get() )->color_state_->get() );
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
  
  return true;
}

// NOTE: This function is copied from the _nrrdResampleOutputUpdate function
static void UpdateNrrdAxisInfo( Nrrd* nout, NrrdResampleContext* rsmc )
{
  nrrdSpaceVecCopy( nout->spaceOrigin, rsmc->nin->spaceOrigin );
  for ( unsigned int axIdx = 0; axIdx < rsmc->dim; axIdx++ )
  {
    double minIdxFull, maxIdxFull, zeroPos;
    _nrrdAxisInfoCopy( nout->axis + axIdx, rsmc->nin->axis + axIdx,
                      ( NRRD_AXIS_INFO_SIZE_BIT
                       | NRRD_AXIS_INFO_SPACING_BIT
                       | NRRD_AXIS_INFO_THICKNESS_BIT
                       | NRRD_AXIS_INFO_MIN_BIT
                       | NRRD_AXIS_INFO_MAX_BIT
                       | NRRD_AXIS_INFO_SPACEDIRECTION_BIT
                       | NRRD_AXIS_INFO_CENTER_BIT
                       | NRRD_AXIS_INFO_KIND_BIT ) );
    /* now set all the per-axis fields we just abstained from copying */
    /* size was already set */
    nout->axis[ axIdx ].spacing =
    ( rsmc->nin->axis[ axIdx ].spacing / rsmc->axis[ axIdx ].ratio );
    /* for now, we don't attempt to modify thickness */
    nout->axis[ axIdx ].thickness = AIR_NAN;
    /* We had to assume a specific centering when doing resampling */
    nout->axis[ axIdx ].center = rsmc->axis[ axIdx ].center;
    _nrrdResampleMinMaxFull( &minIdxFull, &maxIdxFull,
                            rsmc->axis[ axIdx ].center,
                            rsmc->nin->axis[ axIdx ].size );
    nout->axis[ axIdx ].min = AIR_AFFINE( minIdxFull,
                                         rsmc->axis[axIdx].min,
                                         maxIdxFull,
                                         rsmc->nin->axis[ axIdx ].min,
                                         rsmc->nin->axis[ axIdx ].max );
    nout->axis[ axIdx ].max = AIR_AFFINE( minIdxFull,
                                         rsmc->axis[ axIdx ].max,
                                         maxIdxFull,
                                         rsmc->nin->axis[ axIdx ].min,
                                         rsmc->nin->axis[ axIdx ].max );
    nrrdSpaceVecScale( nout->axis[ axIdx ].spaceDirection,
                      1.0 / rsmc->axis[ axIdx ].ratio,
                      rsmc->nin->axis[ axIdx ].spaceDirection );
    nout->axis[ axIdx ].kind = _nrrdKindAltered( rsmc->nin->axis[ axIdx ].kind, AIR_TRUE );
    /* space origin may have translated along this axis;
     only do this if the axis was already spatial */
    if ( AIR_EXISTS( rsmc->nin->axis[ axIdx ].spaceDirection[ 0 ] ) )
    {
      zeroPos = NRRD_POS( nout->axis[ axIdx ].center,
                         rsmc->axis[ axIdx ].min,
                         rsmc->axis[ axIdx ].max,
                         rsmc->axis[ axIdx ].samples,
                         0 );
      nrrdSpaceVecScaleAdd2( nout->spaceOrigin,
                            1.0, nout->spaceOrigin,
                            zeroPos,
                            rsmc->nin->axis[ axIdx ].spaceDirection );
    }
  }

  nrrdBasicInfoCopy( nout, rsmc->nin,
                    NRRD_BASIC_INFO_DATA_BIT
                    | NRRD_BASIC_INFO_TYPE_BIT
                    | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                    | NRRD_BASIC_INFO_DIMENSION_BIT
                    | NRRD_BASIC_INFO_SPACEORIGIN_BIT
                    | NRRD_BASIC_INFO_CONTENT_BIT
                    | NRRD_BASIC_INFO_COMMENTS_BIT
                    | ( nrrdStateKeyValuePairsPropagate
                       ? 0
                       : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT ) );
}

bool NrrdResampleFilter::compute_output_grid_transform( LayerHandle layer,
                                                        NrrdResampleContext* resample_context,
                                                        GridTransform& grid_transform )
{
  DataBlockHandle input_datablock;
  switch ( layer->get_type() )
  {
    case VolumeType::DATA_E:
      input_datablock = static_cast< DataLayer* >( layer.get() )->
        get_data_volume()->get_data_block();
      break;
    case VolumeType::MASK_E:
      // NOTE: The data block won't be used for any real processing, so it's
      // fine to use the whole underlying data block of the mask data
      input_datablock = static_cast< MaskLayer* >( layer.get() )->
        get_mask_volume()->get_mask_data_block()->get_data_block();
      break;
  }

  NrrdDataHandle nrrd_in( new NrrdData( input_datablock, layer->get_grid_transform() ) );
  for ( unsigned int axis = 0; axis < nrrd_in->nrrd()->dim; ++axis )
  {
    // Make sure both min and max are set since UpdateNrrdAxisInfo relies on this.  NrrdData
    // creates nrrds with only min and spacing.
    // Is there a Teem function for doing this?  nrrdAxisInfoMinMaxSet always sets min to 0.
    NrrdAxisInfo nrrd_axis_info = nrrd_in->nrrd()->axis[ axis ];
    if( nrrd_axis_info.center == nrrdCenterCell )
    {
      nrrd_in->nrrd()->axis[ axis ].max = nrrd_axis_info.min + ( nrrd_axis_info.size * nrrd_axis_info.spacing );
    }
    else // Assume node-centering
    {
      nrrd_in->nrrd()->axis[ axis ].max = nrrd_axis_info.min + ( (nrrd_axis_info.size - 1 ) * nrrd_axis_info.spacing );
    }
  }

  Nrrd* nout = nrrdNew();
  int error = AIR_FALSE;

  error |= nrrdResampleNrrdSet( resample_context, nrrd_in->nrrd() );
  for ( unsigned int axis = 0; axis < nrrd_in->nrrd()->dim && !error; ++axis )
  {
    error |= nrrdResampleKernelSet( resample_context, axis,
                                   this->mask_kernel_->kernel, this->mask_kernel_->parm );
    error |= nrrdResampleSamplesSet( resample_context, axis, this->dims_[ axis ] );
    if ( this->crop_ )
    {
      error |= nrrdResampleRangeSet( resample_context, axis,
                                    this->range_min_[ axis ], this->range_max_[ axis ] );
    }
    else
    {
      error |= nrrdResampleRangeFullSet( resample_context, axis );
    }
  }

  if ( error ||
      _nrrdResampleInputDimensionUpdate( resample_context ) ||
      _nrrdResampleInputCentersUpdate( resample_context ) ||
      _nrrdResampleInputSizesUpdate( resample_context ) ||
      _nrrdResampleLineAllocateUpdate( resample_context ) ||
      _nrrdResampleVectorAllocateUpdate( resample_context ) ||
      _nrrdResampleLineFillUpdate( resample_context ) ||
      _nrrdResampleVectorFillUpdate( resample_context ) ||
      _nrrdResamplePermutationUpdate( resample_context ) )
  {
    nrrdNuke( nout );
    return false;
  }

  nout->axis[ 0 ].size = this->dims_[ 0 ];
  nout->axis[ 1 ].size = this->dims_[ 1 ];
  nout->axis[ 2 ].size = this->dims_[ 2 ];
  nout->dim = 3;
  UpdateNrrdAxisInfo( nout, resample_context );
  NrrdDataHandle nrrd_out( new NrrdData( nout ) );
  grid_transform = nrrd_out->get_grid_transform();

  return true;
}

bool NrrdResampleFilter::nrrd_resample( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk )
{
  int error = AIR_FALSE;
  error |= nrrdResampleNrrdSet( this->current_resample_context_, nin );
  for ( unsigned int axis = 0; axis < nin->dim && !error; ++axis )
  {
    error |= nrrdResampleKernelSet( this->current_resample_context_, axis, unuk->kernel,
                                   unuk->parm );
    error |= nrrdResampleSamplesSet( this->current_resample_context_, axis,
                                    this->dims_[ axis ] );
    if ( this->crop_ )
    {
      error |= nrrdResampleRangeSet( this->current_resample_context_, axis,
                                    this->range_min_[ axis ], this->range_max_[ axis ] );
    }
    else
    {
      error |= nrrdResampleRangeFullSet( this->current_resample_context_, axis );
    }
  }
  if ( ! error )
  {
    // For some reason the grid transform of nout isn't set (no min, max, or origin info)
    // unu resample works, so maybe we're doing something wrong?  Working around this for now.
    error |= nrrdResampleExecute( this->current_resample_context_, nout );
  }

  return !error;
}

void NrrdResampleFilter::resample_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  if ( this->padding_only_ )
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

  DataBlock::shared_lock_type data_lock( input->get_data_volume()->get_data_block()->get_mutex() );

  NrrdDataHandle nrrd_in( new NrrdData(input->get_data_volume()->get_data_block(),
                                       input->get_grid_transform() ) );

  Nrrd* nrrd_out = nrrdNew();
  int error = AIR_FALSE;

  if ( this->crop_ )
  {
    if ( this->padding_ == PadValues::ZERO_C )
    {
      error |= nrrdResamplePadValueSet( this->current_resample_context_, 0.0 );
    }
    else if ( this->padding_ == PadValues::MIN_C )
    {
      error |= nrrdResamplePadValueSet( this->current_resample_context_, input->get_data_volume()->
                                       get_data_block()->get_min() );
    }
    else
    {
      error |= nrrdResamplePadValueSet( this->current_resample_context_, input->get_data_volume()->
                                       get_data_block()->get_max() );
    }
    if (error)
    {
      nrrdNuke( nrrd_out );
      CORE_LOG_ERROR( "Failed to set up padding." );
      return;
    }
  }
  output->update_progress_signal_( 0.1 );
  if ( ! nrrd_resample( nrrd_in->nrrd(), nrrd_out, this->data_kernel_ ) )
  {
    nrrdNuke( nrrd_out );
    CORE_LOG_ERROR( "Failed to resample layer '" + input->get_layer_id() +"'" );
  }
  else if ( ! this->check_abort() )
  {
    NrrdDataHandle nrrd_data( new NrrdData( nrrd_out ) );
    DataBlockHandle data_block = NrrdDataBlock::New( nrrd_data );
    DataVolumeHandle data_volume( new DataVolume( this->current_output_transform_, data_block ) );
    this->dispatch_insert_data_volume_into_layer( output, data_volume, true );
    output->update_progress_signal_( 1.0 );
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
}

void NrrdResampleFilter::resample_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
{
  if ( this->padding_only_ )
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

  DataBlockHandle input_data_block;
  MaskDataBlockManager::Convert( input->get_mask_volume()->get_mask_data_block(), input_data_block, DataType::UCHAR_E );
  NrrdDataHandle nrrd_in( new NrrdData( input_data_block, input->get_grid_transform() ) );

  Nrrd* nrrd_out = nrrdNew();

  if ( this->crop_ )
  {
    int error = AIR_FALSE;

    // Pad the mask layer with 0
    error |= nrrdResamplePadValueSet( this->current_resample_context_, 0.0 );
    if (error)
    {
      nrrdNuke( nrrd_out );
      CORE_LOG_ERROR( "Failed to set up padding." );
      return;
    }
  }
  output->update_progress_signal_( 0.1 );
  if ( ! nrrd_resample( nrrd_in->nrrd(), nrrd_out, this->mask_kernel_ ) )
  {
    nrrdNuke( nrrd_out );
    CORE_LOG_ERROR( "Failed to resample layer '" + input->get_layer_id() +"'" );
  }
  else if ( !this->check_abort() )
  {
    NrrdDataHandle nrrd_data( new NrrdData( nrrd_out ) );
    DataBlockHandle data_block = NrrdDataBlock::New( nrrd_data );
    MaskDataBlockHandle mask_data_block;
    if ( ! MaskDataBlockManager::Convert( data_block,
                                          this->current_output_transform_, mask_data_block ) )
    {
      return;
    }

    MaskVolumeHandle mask_volume( new MaskVolume( this->current_output_transform_, mask_data_block ) );
    this->dispatch_insert_mask_volume_into_layer( output, mask_volume );
    output->update_progress_signal_( 1.0 );
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
}

void NrrdResampleFilter::run_filter()
{
  this->pad_internals_.reset( new PadFilterInternals( this->src_layers_[0], this->dst_layers_[0], this->padding_ ) );

  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    this->current_output_transform_ = this->output_transforms_[ i ];
    this->current_resample_context_ = this->resample_contexts_[ i ];

    int error = AIR_FALSE;
    if ( this->crop_ )
    {
      this->padding_only_ = this->pad_internals_->detect_padding_only();
      error |= nrrdResampleBoundarySet( this->current_resample_context_, nrrdBoundaryPad );
    }
    else
    {
      error |= nrrdResampleBoundarySet( this->current_resample_context_, nrrdBoundaryBleed );
    }
    if (error)
    {
      this->report_error( "Resample boundary failed." );
      return;
    }

    if ( this->src_layers_[ i ]->get_type() == VolumeType::DATA_E )
    {
      DataLayerHandle src_data_layer = boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] );
      if (! src_data_layer)
      {
        this->report_error("Error obtaining source data layer.");
        return;
      }
      DataLayerHandle dst_data_layer = boost::dynamic_pointer_cast< DataLayer >( this->dst_layers_[ i ] );
      if (! dst_data_layer)
      {
        this->report_error("Error obtaining destination data layer.");
        return;
      }
      this->resample_data_layer( src_data_layer, dst_data_layer );
    }
    else if ( this->src_layers_[ i ]->get_type() == VolumeType::MASK_E )
    {
      MaskLayerHandle src_mask_layer = boost::dynamic_pointer_cast< MaskLayer >( this->src_layers_[ i ] );
      if (! src_mask_layer)
      {
        this->report_error("Error obtaining source mask layer.");
        return;
      }
      MaskLayerHandle dst_mask_layer = boost::dynamic_pointer_cast< MaskLayer >( this->dst_layers_[ i ] );
      if (! dst_mask_layer)
      {
        this->report_error("Error obtaining destination mask layer.");
        return;
      }
      this->resample_mask_layer( src_mask_layer, dst_mask_layer );
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

