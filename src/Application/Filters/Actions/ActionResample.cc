/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

// teem includes
#include <teem/nrrd.h>
#include <privateNrrd.h>

// Core includes
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/Actions/ActionResample.h>
#include <Application/Filters/LayerFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Resample )

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

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class ActionResamplePrivate
//////////////////////////////////////////////////////////////////////////

class ActionResamplePrivate
{
  // -- action parameters --
public:
  std::vector< std::string > layer_ids_;
  int x_;
  int y_;
  int z_;
  bool crop_;
  Core::Point range_min_;
  Core::Point range_max_;
  std::string padding_;
  std::string kernel_;
  double param1_;
  double param2_;
  bool replace_;
  SandboxID sandbox_;

  // -- internal variables --
public:
  bool match_grid_transform_;
  Core::GridTransform grid_transform_;
};

//////////////////////////////////////////////////////////////////////////
// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.
//////////////////////////////////////////////////////////////////////////

class ResampleAlgo : public LayerFilter
{

public:
  std::vector< LayerHandle > src_layers_;
  std::vector< LayerHandle > dst_layers_;
  bool replace_;
  unsigned int dimesions_[ 3 ];
  bool crop_; // Whether to use a different dataset boundary than the original
  Core::Point range_min_; // resample range in index space of the input data
  Core::Point range_max_; // resample range in index space of the input data
  std::string padding_;

  bool padding_only_;
  int mapped_x_start_;
  int mapped_y_start_;
  int mapped_z_start_;
  int overlap_x_start_;
  int overlap_y_start_;
  int overlap_z_start_;
  int overlap_nx_;
  int overlap_ny_;
  int overlap_nz_;

  NrrdKernelSpec* mask_kernel_;
  NrrdKernelSpec* data_kernel_;

  std::vector< NrrdResampleContext* > resample_contexts_; // Per layer
  std::vector< Core::GridTransform > output_transforms_; // Per layer
  NrrdResampleContext* current_resample_context_;
  Core::GridTransform current_output_transform_;

public:
  ResampleAlgo( const std::string& kernel, double param1, double param2 );
  virtual ~ResampleAlgo();

  // COMPUTE_OUTPUT_GRID_TRANSFORM:
  // Compute the output grid transform of the input layer.
  bool compute_output_grid_transform( LayerHandle layer, NrrdResampleContext* resample_context,
    Core::GridTransform& grid_transform );

  // DETECT_PADDING_ONLY:
  // Detect cases where sample positions are not changed so we only need to do padding/cropping.
  void detect_padding_only();

  // NRRD_RESAMPLE:
  // Resample the nrrd data.
  bool nrrd_resample( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk );

  // RESAMPLE_DATA_LAYER:
  // Resample a  data layer.
  void resample_data_layer( DataLayerHandle input, DataLayerHandle output );

  // RESAMPLE_MASK_LAYER:
  // Resample a mask layer.
  void resample_mask_layer( MaskLayerHandle input, MaskLayerHandle output );

  // PAD_AND_CROP_DATA_LAYER:
  // Pad/crop data layer for cases where sample positions are not changed.
  void pad_and_crop_data_layer( DataLayerHandle input, DataLayerHandle output );
  
  // PAD_AND_CROP_TYPED_DATA:
  // Templated implementation for pad_and_crop_data_layer.
  template< class T >
  void pad_and_crop_typed_data( Core::DataBlockHandle src, Core::DataBlockHandle dst,
                 DataLayerHandle output_layer );
  
  // PAD_AND_CROP_MASK_LAYER:
  // Pad/crop mask layer for cases where sample positions are not changed.
  void pad_and_crop_mask_layer( MaskLayerHandle input, MaskLayerHandle output );
  
  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run_filter();

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Resample Tool";
  }
  
  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Resample";  
  } 
};

ResampleAlgo::ResampleAlgo( const std::string& kernel, double param1, double param2 )
{
  this->padding_only_ = false;
  this->data_kernel_ = nrrdKernelSpecNew();
  this->mask_kernel_ = nrrdKernelSpecNew();

  this->mask_kernel_->kernel = nrrdKernelCheap;
  this->mask_kernel_->parm[ 0 ] = 1.0;

  this->data_kernel_->parm[ 0 ] = 1.0;
  if ( kernel == ActionResample::BOX_C )
  {
    this->data_kernel_->kernel = nrrdKernelBox;
  }
  else if ( kernel == ActionResample::TENT_C )
  {
    this->data_kernel_->kernel = nrrdKernelTent;
  }
  else if ( kernel == ActionResample::CUBIC_BS_C )
  {
    this->data_kernel_->kernel = nrrdKernelBCCubic;
    this->data_kernel_->parm[ 1 ] = 1.0;
    this->data_kernel_->parm[ 2 ] = 0.0;
  }
  else if ( kernel == ActionResample::CUBIC_CR_C )
  {
    this->data_kernel_->kernel = nrrdKernelBCCubic;
    this->data_kernel_->parm[ 1 ] = 0.0;
    this->data_kernel_->parm[ 2 ] = 0.5;
  }
  else if ( kernel == ActionResample::QUARTIC_C )
  {
    this->data_kernel_->kernel = nrrdKernelAQuartic;
    this->data_kernel_->parm[ 1 ] = 0.0834;
  }
  else
  {
    this->data_kernel_->kernel = nrrdKernelGaussian;
    this->data_kernel_->parm[ 0 ] = param1;
    this->data_kernel_->parm[ 1 ] = param2;
  }
}

ResampleAlgo::~ResampleAlgo()
{
  nrrdKernelSpecNix( this->data_kernel_ );
  nrrdKernelSpecNix( this->mask_kernel_ );
  for( size_t i = 0; i < this->resample_contexts_.size(); i++ )
  {
    nrrdResampleContextNix( this->resample_contexts_[ i ] );
  }
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

bool ResampleAlgo::compute_output_grid_transform( LayerHandle layer, 
  NrrdResampleContext* resample_context, Core::GridTransform& grid_transform )
{
  Core::DataBlockHandle input_datablock;
  switch ( layer->get_type() )
  {
  case Core::VolumeType::DATA_E:
    input_datablock = static_cast< DataLayer* >( layer.get() )->
      get_data_volume()->get_data_block();
    break;
  case Core::VolumeType::MASK_E:
    // NOTE: The data block won't be used for any real processing, so it's 
    // fine to use the whole underlying data block of the mask data
    input_datablock = static_cast< MaskLayer* >( layer.get() )->
      get_mask_volume()->get_mask_data_block()->get_data_block();
    break;
  }
  
  Core::NrrdDataHandle nrrd_in( new Core::NrrdData( input_datablock, 
    layer->get_grid_transform() ) );
  for ( unsigned int axis = 0; axis < nrrd_in->nrrd()->dim; ++axis )
  {
    // Make sure both min and max are set since UpdateNrrdAxisInfo relies on this.  NrrdData
    // creates nrrds with only min and spacing.
    // Is there a Teem function for doing this?  nrrdAxisInfoMinMaxSet always sets min to 0.
    NrrdAxisInfo nrrd_axis_info = nrrd_in->nrrd()->axis[ axis ];
    if( nrrd_axis_info.center == nrrdCenterCell )
    {
      nrrd_in->nrrd()->axis[ axis ].max = nrrd_axis_info.min + 
        ( nrrd_axis_info.size * nrrd_axis_info.spacing );
    }
    else // Assume node-centering
    {
      nrrd_in->nrrd()->axis[ axis ].max = nrrd_axis_info.min + 
        ( (nrrd_axis_info.size - 1 ) * nrrd_axis_info.spacing );
    }
  }

  Nrrd* nout = nrrdNew();
  int error = AIR_FALSE;

  error |= nrrdResampleNrrdSet( resample_context, nrrd_in->nrrd() );
  for ( unsigned int axis = 0; axis < nrrd_in->nrrd()->dim && !error; ++axis )
  {
    error |= nrrdResampleKernelSet( resample_context, axis, 
      this->mask_kernel_->kernel, this->mask_kernel_->parm );
    error |= nrrdResampleSamplesSet( resample_context, axis, this->dimesions_[ axis ] );
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

  nout->axis[ 0 ].size = this->dimesions_[ 0 ];
  nout->axis[ 1 ].size = this->dimesions_[ 1 ];
  nout->axis[ 2 ].size = this->dimesions_[ 2 ];
  nout->dim = 3;
  UpdateNrrdAxisInfo( nout, resample_context );
  Core::NrrdDataHandle nrrd_out( new Core::NrrdData( nout ) );
  grid_transform = nrrd_out->get_grid_transform();

  return true;
}

bool ResampleAlgo::nrrd_resample( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk )
{
  int error = AIR_FALSE;
  error |= nrrdResampleNrrdSet( this->current_resample_context_, nin );
  for ( unsigned int axis = 0; axis < nin->dim && !error; ++axis )
  {
    error |= nrrdResampleKernelSet( this->current_resample_context_, axis, unuk->kernel, 
      unuk->parm );
    error |= nrrdResampleSamplesSet( this->current_resample_context_, axis, 
      this->dimesions_[ axis ] );
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

void ResampleAlgo::resample_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  if ( this->padding_only_ )
  {
    this->pad_and_crop_data_layer( input, output );
    return;
  }
  
  Core::DataBlock::shared_lock_type data_lock( input->get_data_volume()->
    get_data_block()->get_mutex() );

  Core::NrrdDataHandle nrrd_in( new Core::NrrdData( 
    input->get_data_volume()->get_data_block(),
    input->get_grid_transform() ) );

  Nrrd* nrrd_out = nrrdNew();
  int error = AIR_FALSE;

  if ( this->crop_ )
  {
    if ( this->padding_ == ActionResample::ZERO_C )
    {
      error |= nrrdResamplePadValueSet( this->current_resample_context_, 0.0 );
    }
    else if ( this->padding_ == ActionResample::MIN_C )
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
  else if ( !this->check_abort() )
  {
    Core::NrrdDataHandle nrrd_data( new Core::NrrdData( nrrd_out ) );
    Core::DataBlockHandle data_block = Core::NrrdDataBlock::New( nrrd_data );
    Core::DataVolumeHandle data_volume( new Core::DataVolume( 
      this->current_output_transform_, data_block ) );
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

void ResampleAlgo::resample_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
{
  if ( this->padding_only_ )
  {
    this->pad_and_crop_mask_layer( input, output );
    return;
  }
  
  Core::DataBlockHandle input_data_block;
  Core::MaskDataBlockManager::Convert( input->get_mask_volume()->get_mask_data_block(),
    input_data_block, Core::DataType::UCHAR_E );
  Core::NrrdDataHandle nrrd_in( new Core::NrrdData( input_data_block, input->get_grid_transform() ) );

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
    Core::NrrdDataHandle nrrd_data( new Core::NrrdData( nrrd_out ) );
    Core::DataBlockHandle data_block = Core::NrrdDataBlock::New( nrrd_data );
    Core::MaskDataBlockHandle mask_data_block;
    if ( ! Core::MaskDataBlockManager::Convert( data_block,
      this->current_output_transform_, mask_data_block ) )
    {
      return;
    }

    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
      this->current_output_transform_, mask_data_block ) );
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

void ResampleAlgo::run_filter()
{
  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    this->current_output_transform_ = this->output_transforms_[ i ];
    this->current_resample_context_ = this->resample_contexts_[ i ];

    int error = AIR_FALSE;
    if ( this->crop_ )
    {
      this->detect_padding_only();
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

    switch ( this->src_layers_[ i ]->get_type() )
    {
      case Core::VolumeType::DATA_E:
        this->resample_data_layer(
          boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] ),
          boost::dynamic_pointer_cast< DataLayer >( this->dst_layers_[ i ] ) );
        break;
      case Core::VolumeType::MASK_E:
        this->resample_mask_layer(
          boost::dynamic_pointer_cast< MaskLayer >( this->src_layers_[ i ] ),
          boost::dynamic_pointer_cast< MaskLayer >( this->dst_layers_[ i ] ) );
        break;
    }

    if ( ! this->dst_layers_[ i ] )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
    
    if ( this->check_abort() ) break;
  }
}

void ResampleAlgo::detect_padding_only()
{
  this->padding_only_ = false;
  Core::GridTransform src_trans = this->src_layers_[ 0 ]->get_grid_transform();
  Core::GridTransform dst_trans = this->dst_layers_[ 0 ]->get_grid_transform();
  double epsilon = 1e-2;

  // Compare spacing
  Core::Vector src_spacing = src_trans * Core::Vector( 1.0, 1.0, 1.0 );
  Core::Vector dst_spacing = dst_trans * Core::Vector( 1.0, 1.0, 1.0 );
  if ( Core::Abs( src_spacing[ 0 ] - dst_spacing[ 0 ] ) > epsilon * src_spacing[ 0 ] ||
    Core::Abs( src_spacing[ 1 ] - dst_spacing[ 1 ] ) > epsilon * src_spacing[ 1 ] ||
    Core::Abs( src_spacing[ 2 ] - dst_spacing[ 2 ] ) > epsilon * src_spacing[ 2 ] )
  {
    return;
  }
  
  // Check if source grid aligns with destination grid
  Core::Point src_origin = src_trans * Core::Point( 0.0, 0.0, 0.0 );
  Core::Transform inv_dst_trans = dst_trans.get_inverse();
  Core::Point src_origin_to_dst_index = inv_dst_trans * src_origin;
  if ( Core::Abs( Core::Fraction( src_origin_to_dst_index[ 0 ] ) ) > epsilon ||
    Core::Abs( Core::Fraction( src_origin_to_dst_index[ 1 ] ) ) > epsilon ||
    Core::Abs( Core::Fraction( src_origin_to_dst_index[ 2 ] ) ) > epsilon )
  {
    return;
  }
  
  CORE_LOG_DEBUG( "Resample tool: padding only situation detected." );
  this->padding_only_ = true;

  // Compute the range of the source volume mapped to the destination volume in index space
  this->mapped_x_start_ = static_cast< int >( src_origin_to_dst_index[ 0 ] );
  this->mapped_y_start_ = static_cast< int >( src_origin_to_dst_index[ 1 ] );
  this->mapped_z_start_ = static_cast< int >( src_origin_to_dst_index[ 2 ] );
  this->overlap_x_start_ = Core::Max( 0, this->mapped_x_start_ );
  this->overlap_y_start_ = Core::Max( 0, this->mapped_y_start_ );
  this->overlap_z_start_ = Core::Max( 0, this->mapped_z_start_ );
  this->overlap_nx_ = Core::Max( Core::Min( this->mapped_x_start_ + 
    static_cast< int >( src_trans.get_nx() ), 
    static_cast< int >( dst_trans.get_nx() ) ) - 
    this->overlap_x_start_, 0 );
  this->overlap_ny_ = Core::Max( Core::Min( this->mapped_y_start_ + 
    static_cast< int >( src_trans.get_ny() ),
    static_cast< int >( dst_trans.get_ny() ) ) - 
    this->overlap_y_start_, 0 );
  this->overlap_nz_ = Core::Max( Core::Min( this->mapped_z_start_ + 
    static_cast< int >( src_trans.get_nz() ),
    static_cast< int >( dst_trans.get_nz() ) ) - 
    this->overlap_z_start_, 0 );
}

template< class T >
void ResampleAlgo::pad_and_crop_typed_data( Core::DataBlockHandle src, Core::DataBlockHandle dst,
                       DataLayerHandle output_layer )
{
  Core::GridTransform src_trans = this->src_layers_[ 0 ]->get_grid_transform();
  int src_nx = static_cast< int >( src_trans.get_nx() );
  int src_ny = static_cast< int >( src_trans.get_ny() );
  int src_nxy = src_nx * src_ny;

  Core::GridTransform dst_trans = this->dst_layers_[ 0 ]->get_grid_transform();
  int dst_nx = static_cast< int >( dst_trans.get_nx() );
  int dst_ny = static_cast< int >( dst_trans.get_ny() );
  int dst_nz = static_cast< int >( dst_trans.get_nz() );
  int dst_nxy = dst_nx * dst_ny;
  
  const T* src_data = reinterpret_cast< T* >( src->get_data() );
  T* dst_data = reinterpret_cast< T* >( dst->get_data() );
  
  T padding_val;
  if ( this->padding_ == ActionResample::ZERO_C )
  {
    padding_val = T( 0 );
  }
  else if ( this->padding_ == ActionResample::MIN_C )
  {
    padding_val = static_cast< T >( src->get_min() );
  }
  else
  {
    padding_val = static_cast< T >( dst->get_max() );
  }
  
  // Pad the non-overlapped part in Z-direction
  for ( int z = 0; z < this->overlap_z_start_; ++z )
  {
    if ( this->check_abort() )  return;
    
    int start_index = z * dst_nxy;
    for ( int i = 0; i < dst_nxy; ++i )
    {
      dst_data[ start_index + i ] = padding_val;
    }
  }
  
  output_layer->update_progress_signal_( this->overlap_z_start_ * 1.0 / dst_nz );
  
  // Process the overlapped part in Z-direction
  for ( int z = this->overlap_z_start_; z < this->overlap_z_start_ + this->overlap_nz_; ++z )
  {
    if ( this->check_abort() )  return;

    int offset_z = z * dst_nxy;
    
    // Pad the non-overlapped part in Y-direction
    for ( int y = 0; y < this->overlap_y_start_; ++y )
    {
      int offset_y = y * dst_nx;
      for ( int x = 0; x < dst_nx; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = padding_val;
      }
    }
    
    // Process the overlapped part in Y-direction
    for ( int y = this->overlap_y_start_; y < this->overlap_y_start_ + this->overlap_ny_; ++y )
    {
      int offset_y = y * dst_nx;
      // Pad the non-overlapped part in X-direction
      for ( int x = 0; x < this->overlap_x_start_; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = padding_val;
      }
      // Copy over the overlapped part in X-direction
      if ( this->overlap_nx_ > 0 )
      {
        memcpy( dst_data + offset_z + offset_y + this->overlap_x_start_,
             src_data + ( z - this->mapped_z_start_ ) * src_nxy + 
             ( y - this->mapped_y_start_ ) * src_nx + 
             this->overlap_x_start_ - this->mapped_x_start_,
             this->overlap_nx_ * sizeof( T ) );       
      }
      // Pad the non-overlapped part in X-direction
      for ( int x = this->overlap_x_start_ + this->overlap_nx_; x < dst_nx; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = padding_val;
      }
    }
    
    // Pad the non-overlapped part in Y-direction
    for ( int y = this->overlap_y_start_ + this->overlap_ny_; y < dst_ny; ++y )
    {
      int offset_y = y * dst_nx;
      for ( int x = 0; x < dst_nx; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = padding_val;
      }
    }
    
    output_layer->update_progress_signal_( ( z + 1.0 ) / dst_nz );
  }
  
  // Pad the non-overlapped part in Z-direction
  for ( int z = this->overlap_z_start_ + this->overlap_nz_; z < dst_nz; ++z )
  {
    if ( this->check_abort() )  return;

    int start_index = z * dst_nxy;
    for ( int i = 0; i < dst_nxy; ++i )
    {
      dst_data[ start_index + i ] = padding_val;
    }
  }
  
  output_layer->update_progress_signal_( 1.0 );
}

void ResampleAlgo::pad_and_crop_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  Core::DataBlockHandle input_datablock = input->get_data_volume()->get_data_block();
  Core::DataBlockHandle output_datablock = Core::StdDataBlock::New( 
    output->get_grid_transform(), input_datablock->get_data_type() );
  if ( !output_datablock ) 
  {
    this->report_error( "Could not allocate enough memory" );
    return;
  }
  
  Core::DataBlock::shared_lock_type data_lock( input_datablock->get_mutex() );
  switch ( input_datablock->get_data_type() )
  {
  case Core::DataType::CHAR_E:
    this->pad_and_crop_typed_data< signed char >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::UCHAR_E:
    this->pad_and_crop_typed_data< unsigned char >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::SHORT_E:
    this->pad_and_crop_typed_data< short >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::USHORT_E:
    this->pad_and_crop_typed_data< unsigned short >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::INT_E:
    this->pad_and_crop_typed_data< int >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::UINT_E:
    this->pad_and_crop_typed_data< unsigned int >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::LONGLONG_E:
    this->pad_and_crop_typed_data< long long >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::ULONGLONG_E:
    this->pad_and_crop_typed_data< unsigned long long >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::FLOAT_E:
    this->pad_and_crop_typed_data< float >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::DOUBLE_E:
    this->pad_and_crop_typed_data< double >( input_datablock, output_datablock, output );
    break;
  default:
    this->report_error("Unsupported data type");
    return;
  }
  
  data_lock.unlock();
  
  if ( !this->check_abort() )
  {
    Core::GridTransform output_grid_transform = output->get_grid_transform();
    output_grid_transform.set_originally_node_centered( 
      input->get_grid_transform().get_originally_node_centered() );
    this->dispatch_insert_data_volume_into_layer( output, Core::DataVolumeHandle(
      new Core::DataVolume( output_grid_transform, output_datablock ) ), true );
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

void ResampleAlgo::pad_and_crop_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
{
  Core::MaskDataBlockHandle input_mask = input->get_mask_volume()->get_mask_data_block();
  Core::DataBlockHandle output_mask = Core::StdDataBlock::New(
    output->get_grid_transform(), Core::DataType::UCHAR_E );
  if ( !output_mask ) 
  {
    this->report_error( "Could not allocate enough memory" );
    return;
  }
  
  Core::MaskDataBlock::shared_lock_type data_lock( input_mask->get_mutex() );
  const unsigned char* src_data = input_mask->get_mask_data();
  unsigned char* dst_data = reinterpret_cast< unsigned char* >( output_mask->get_data() );
  unsigned char mask_value = input_mask->get_mask_value();
  
  Core::GridTransform src_trans = input->get_grid_transform();
  int src_nx = static_cast< int >( src_trans.get_nx() );
  int src_ny = static_cast< int >( src_trans.get_ny() );
  int src_nxy = src_nx * src_ny;
  
  Core::GridTransform dst_trans = output->get_grid_transform();
  int dst_nx = static_cast< int >( dst_trans.get_nx() );
  int dst_ny = static_cast< int >( dst_trans.get_ny() );
  int dst_nz = static_cast< int >( dst_trans.get_nz() );
  int dst_nxy = dst_nx * dst_ny;
  
  // Pad the non-overlapped part in Z-direction
  for ( int z = 0; z < this->overlap_z_start_; ++z )
  {
    if ( this->check_abort() )  return;
    
    int start_index = z * dst_nxy;
    for ( int i = 0; i < dst_nxy; ++i )
    {
      dst_data[ start_index + i ] = 0;
    }
  }
  
  output->update_progress_signal_( this->overlap_z_start_ * 1.0 / dst_nz );
  
  // Process the overlapped part in Z-direction
  for ( int z = this->overlap_z_start_; z < this->overlap_z_start_ + this->overlap_nz_; ++z )
  {
    if ( this->check_abort() )  return;
    
    int offset_z = z * dst_nxy;
    
    // Pad the non-overlapped part in Y-direction
    for ( int y = 0; y < this->overlap_y_start_; ++y )
    {
      int offset_y = y * dst_nx;
      for ( int x = 0; x < dst_nx; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = 0;
      }
    }
    
    // Process the overlapped part in Y-direction
    for ( int y = this->overlap_y_start_; y < this->overlap_y_start_ + this->overlap_ny_; ++y )
    {
      int offset_y = y * dst_nx;
      // Pad the non-overlapped part in X-direction
      for ( int x = 0; x < this->overlap_x_start_; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = 0;
      }
      // Copy over the overlapped part in X-direction
      int src_z_offset = ( z - this->mapped_z_start_ ) * src_nxy;
      int src_y_offset = ( y - this->mapped_y_start_ ) * src_nx;
      for ( int x = this->overlap_x_start_; x < this->overlap_x_start_ + this->overlap_nx_; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = src_data[ src_z_offset + src_y_offset + 
          x - this->mapped_x_start_ ] & mask_value;
      }
      // Pad the non-overlapped part in X-direction
      for ( int x = this->overlap_x_start_ + this->overlap_nx_; x < dst_nx; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = 0;
      }
    }
    
    // Pad the non-overlapped part in Y-direction
    for ( int y = this->overlap_y_start_ + this->overlap_ny_; y < dst_ny; ++y )
    {
      int offset_y = y * dst_nx;
      for ( int x = 0; x < dst_nx; ++x )
      {
        dst_data[ offset_z + offset_y + x ] = 0;
      }
    }
    
    output->update_progress_signal_( ( z + 1.0 ) / dst_nz );
  }
  
  // Pad the non-overlapped part in Z-direction
  for ( int z = this->overlap_z_start_ + this->overlap_nz_; z < dst_nz; ++z )
  {
    if ( this->check_abort() )  return;
    
    int start_index = z * dst_nxy;
    for ( int i = 0; i < dst_nxy; ++i )
    {
      dst_data[ start_index + i ] = 0;
    }
  }
  
  output->update_progress_signal_( 1.0 );
  
  data_lock.unlock();
  
  if ( !this->check_abort() )
  {
    Core::MaskDataBlockHandle dst_mask_data_block;
    if ( !Core::MaskDataBlockManager::Convert( output_mask, output->get_grid_transform(),
                          dst_mask_data_block ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
    Core::GridTransform output_grid_transform = output->get_grid_transform();
    output_grid_transform.set_originally_node_centered( 
      input->get_grid_transform().get_originally_node_centered() );
    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume(
      output_grid_transform, dst_mask_data_block ) );
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
}

//////////////////////////////////////////////////////////////////////////
// Class ActionResample
//////////////////////////////////////////////////////////////////////////

const std::string ActionResample::ZERO_C( "0" );
const std::string ActionResample::MIN_C( "min" );
const std::string ActionResample::MAX_C( "max" );

const std::string ActionResample::BOX_C( "box" );
const std::string ActionResample::TENT_C( "tent" );
const std::string ActionResample::CUBIC_CR_C( "cubic_cr" );
const std::string ActionResample::CUBIC_BS_C( "cubic_bs" );
const std::string ActionResample::QUARTIC_C( "quartic" );
const std::string ActionResample::GAUSSIAN_C( "gauss" );

ActionResample::ActionResample() :
  private_( new ActionResamplePrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->x_ );
  this->add_parameter( this->private_->y_ );
  this->add_parameter( this->private_->z_ );
  this->add_parameter( this->private_->crop_ );
  this->add_parameter( this->private_->range_min_ );
  this->add_parameter( this->private_->range_max_ );
  this->add_parameter( this->private_->padding_ );
  this->add_parameter( this->private_->kernel_ );
  this->add_parameter( this->private_->param1_ );
  this->add_parameter( this->private_->param2_ );
  this->add_parameter( this->private_->replace_ );
  this->add_parameter( this->private_->sandbox_ );

  this->private_->match_grid_transform_ = false;
}

bool ActionResample::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) )
  {
    return false;
  }

  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;
  if ( layer_ids.size() == 0 )
  {
    context->report_error( "No input layers specified" );
    return false;
  }
  
  for ( size_t i = 0; i < layer_ids.size(); ++i )
  {
    // Check for layer existence and type information
    if ( !LayerManager::CheckLayerExistence( layer_ids[ i ], context, this->private_->sandbox_ ) )
      return false;
    
    // Check for layer availability 
    if ( !LayerManager::CheckLayerAvailability( layer_ids[ i ], 
      this->private_->replace_, context, this->private_->sandbox_ ) ) return false;
  }

  if ( this->private_->x_ < 1 ||
    this->private_->y_ < 1 ||
    this->private_->z_ < 1 )
  {
    context->report_error( "Invalid resample size" );
    return false;
  }

  if ( this->private_->match_grid_transform_ )
  {
    this->private_->crop_ = true;

    // Compute the boundary of the new grid transform in world space
    // NOTE: If the destination grid is cell centered, the actual range of the grid should be
    // extended by half a voxel in each direction
    double offset = this->private_->grid_transform_.get_originally_node_centered() ? 0.0 : 0.5;
    Core::Point start( -offset, -offset, -offset );
    Core::Point end( this->private_->x_ - 1 + offset, this->private_->y_ - 1 + offset, this->private_->z_ - 1 + offset );
    this->private_->range_min_ = this->private_->grid_transform_ * start;
    this->private_->range_max_ = this->private_->grid_transform_ * end;

    // Compute the resample range relative to the input in index space
    LayerHandle layer = LayerManager::FindLayer( layer_ids[ 0 ] );
    Core::Transform inverse_src_trans = layer->get_grid_transform().get_inverse();
    this->private_->range_min_ = inverse_src_trans * this->private_->range_min_;
    this->private_->range_max_ = inverse_src_trans * this->private_->range_max_;
  }
  
  if ( this->private_->crop_ )
  {
    if ( this->private_->range_max_[ 0 ] <= this->private_->range_min_[ 0 ] ||
         this->private_->range_max_[ 1 ] <= this->private_->range_min_[ 1 ] ||
         this->private_->range_max_[ 2 ] <= this->private_->range_min_[ 2 ] )
    {
      context->report_error( "Invalid resample range." );
      return false;
    }
    
    if ( this->private_->padding_ != ZERO_C &&
         this->private_->padding_ != MIN_C &&
         this->private_->padding_ != MAX_C )
    {
      context->report_error( "Unknown padding option" );
      return false;
    }
  }
  
  if ( this->private_->kernel_ != BOX_C &&
       this->private_->kernel_ != TENT_C &&
       this->private_->kernel_ != CUBIC_BS_C &&
       this->private_->kernel_ != CUBIC_CR_C &&
       this->private_->kernel_ != QUARTIC_C &&
       this->private_->kernel_ != GAUSSIAN_C )
  {
    context->report_error( "Unknown kernel type" );
    return false;
  }
  
  // Validation successful
  return true;
}

bool ActionResample::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< ResampleAlgo > algo( new ResampleAlgo( this->private_->kernel_, 
    this->private_->param1_, this->private_->param2_ ) );

  // Set up parameters
  algo->set_sandbox( this->private_->sandbox_ );
  algo->replace_ = this->private_->replace_;
  algo->dimesions_[ 0 ] = static_cast< unsigned int >( this->private_->x_ );
  algo->dimesions_[ 1 ] = static_cast< unsigned int >( this->private_->y_ );
  algo->dimesions_[ 2 ] = static_cast< unsigned int >( this->private_->z_ );
  algo->crop_ = this->private_->crop_;
  algo->range_min_ = this->private_->range_min_;
  algo->range_max_ = this->private_->range_max_;
  algo->padding_ = this->private_->padding_;

  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;

  // Set up input and output layers
  size_t num_of_layers = layer_ids.size();
  algo->src_layers_.resize( num_of_layers );
  algo->dst_layers_.resize( num_of_layers );
  algo->resample_contexts_.resize( num_of_layers );
  algo->output_transforms_.resize( num_of_layers );
  std::vector< std::string > dst_layer_ids( num_of_layers );
  for ( size_t i = 0; i < num_of_layers; ++i )
  {
    algo->find_layer( layer_ids[ i ], algo->src_layers_[ i ] );
    if ( algo->replace_ )
    {
      algo->lock_for_deletion( algo->src_layers_[ i ] );
    }
    else
    {
      algo->lock_for_use( algo->src_layers_[ i ] );
    }

    // Compute grid transform for the output layers
    algo->resample_contexts_[ i ] = nrrdResampleContextNew();
    algo->resample_contexts_[ i ]->verbose = 0;

    int error = AIR_FALSE;
    error |= nrrdResampleDefaultCenterSet( algo->resample_contexts_[ i ], nrrdCenterCell );
    if (error)
    {
      context->report_error( "Setting resampled center fail" );
      return false;
    }

    if ( this->private_->match_grid_transform_ )
    {
      algo->output_transforms_[ i ] = this->private_->grid_transform_;
      algo->output_transforms_[ i ].set_originally_node_centered( algo->src_layers_[ i ]->
        get_grid_transform().get_originally_node_centered() );
    }
    else
    {
      if (! algo->compute_output_grid_transform( algo->src_layers_[ i ],
        algo->resample_contexts_[ i ], algo->output_transforms_[ i ] ) )
      {
        context->report_error( "Computing grid transform failed." );
        return false;
      }
    }

    switch ( algo->src_layers_[ i ]->get_type() )
    {
      case Core::VolumeType::DATA_E:
        algo->create_and_lock_data_layer( algo->output_transforms_[ i ], 
          algo->src_layers_[ i ], algo->dst_layers_[ i ] );
        break;
      case Core::VolumeType::MASK_E:
        algo->create_and_lock_mask_layer( algo->output_transforms_[ i ],
          algo->src_layers_[ i ], algo->dst_layers_[ i ] );
        static_cast< MaskLayer* >( algo->dst_layers_[ i ].get() )->color_state_->set(
          static_cast< MaskLayer* >( algo->src_layers_[ i ].get() )->color_state_->get() );
        break;
      default:
        context->report_error( "Unknown volume type." );
        return false;
    }

    if ( ! algo->dst_layers_[ i ] )
    {
      context->report_error( "Could not allocate enough memory." );
      return false;
    }
    
    dst_layer_ids[ i ] = algo->dst_layers_[ i ]->get_layer_id();
  }

  // Return the ids of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( dst_layer_ids ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }
  
  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this(), true );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionResample::Dispatch( Core::ActionContextHandle context, 
                const std::vector< std::string >& layer_ids, 
                int x, int y, int z, const std::string& kernel, 
                double param1, double param2, bool replace )
{
  ActionResample* action = new ActionResample;
  action->private_->layer_ids_ = layer_ids;
  action->private_->x_ = x;
  action->private_->y_ = y;
  action->private_->z_ = z;
  action->private_->kernel_ = kernel;
  action->private_->param1_ = param1;
  action->private_->param2_ = param2;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionResample::Dispatch( Core::ActionContextHandle context, 
                const std::vector< std::string >& layer_ids, 
                const Core::GridTransform& grid_trans, 
                const std::string& padding, const std::string& kernel, 
                double param1, double param2, bool replace )
{
  ActionResample* action = new ActionResample;

  action->private_->layer_ids_ = layer_ids;
  int nx = static_cast< int >( grid_trans.get_nx() );
  int ny = static_cast< int >( grid_trans.get_ny() );
  int nz = static_cast< int >( grid_trans.get_nz() );
  action->private_->x_ = nx;
  action->private_->y_ = ny;
  action->private_->z_ = nz;
  action->private_->crop_ = true;
  action->private_->padding_ = padding;

  action->private_->match_grid_transform_ = true;
  action->private_->grid_transform_ = grid_trans;

  action->private_->kernel_ = kernel;
  action->private_->param1_ = param1;
  action->private_->param2_ = param2;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionResample::Dispatch( Core::ActionContextHandle context, 
                const std::string& src_layer, const std::string& dst_layer, 
                const std::string& padding, const std::string& kernel, 
                double param1, double param2, bool replace )
{
  LayerHandle layer = LayerManager::FindLayer( dst_layer );
  if ( layer )
  {
    std::vector< std::string > layer_ids( 1, src_layer );
    ActionResample::Dispatch( context, layer_ids, layer->get_grid_transform(),
      padding, kernel, param1, param2, replace );
  }
}

} // end namespace Seg3D
