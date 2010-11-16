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

#include <teem/nrrd.h>
#include <teem/privateNrrd.h>

#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/LayerManager/LayerManager.h>
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
public:
  Core::ActionParameter< std::vector< std::string > > layer_ids_;
  Core::ActionParameter< int > x_;
  Core::ActionParameter< int > y_;
  Core::ActionParameter< int > z_;
  Core::ActionParameter< std::string > kernel_;
  Core::ActionParameter< double > param1_;
  Core::ActionParameter< double > param2_;
  Core::ActionParameter< bool > replace_;

  Core::GridTransform grid_transform_;
  bool resample_to_grid_;
  bool resample_to_layer_;
  std::string dst_layer_id_;
  std::string padding_;
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
  double range_min_[ 3 ];
  double range_max_[ 3 ];
  bool resample_to_grid_;
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
  NrrdResampleContext* rsmc_;

public:
  ResampleAlgo( const std::string& kernel, double param1, double param2 );
  virtual ~ResampleAlgo();

  bool compute_output_grid_transform( const std::string& input_layerid, 
    Core::GridTransform& grid_transform );

  void detect_padding_only();

  bool nrrd_resmaple( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk );
  void resmaple_data_layer( DataLayerHandle input, DataLayerHandle output );
  void resample_mask_layer( MaskLayerHandle input, MaskLayerHandle output );

  void pad_and_crop_data_layer( DataLayerHandle input, DataLayerHandle output );
  
  template< class T >
  void pad_and_crop_typed_data( Core::DataBlockHandle src, Core::DataBlockHandle dst,
                 DataLayerHandle output_layer );
  
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
  this->rsmc_ = nrrdResampleContextNew();
  this->rsmc_->verbose = 0;
  nrrdResampleDefaultCenterSet( this->rsmc_, nrrdCenterCell );
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
  nrrdResampleContextNix( this->rsmc_ );
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
        nout->axis[ axIdx ].spacing = ( rsmc->nin->axis[ axIdx ].spacing
                                     / rsmc->axis[ axIdx ].ratio );
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
        nout->axis[ axIdx ].kind = _nrrdKindAltered( rsmc->nin->axis[ axIdx ].kind,
                                                  AIR_TRUE );
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

bool ResampleAlgo::compute_output_grid_transform( const std::string& input_layerid, 
                         Core::GridTransform& grid_transform )
{
  LayerHandle layer;
  this->find_layer( input_layerid, layer );
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
    layer->get_grid_transform().transform() ) );
  Nrrd* nout = nrrdNew();

  int error = 0;
  error |= nrrdResampleNrrdSet( this->rsmc_, nrrd_in->nrrd() );
  for ( unsigned int axis = 0; axis < nrrd_in->nrrd()->dim && !error; ++axis )
  {
    error |= nrrdResampleKernelSet( this->rsmc_, axis, 
      this->mask_kernel_->kernel, this->mask_kernel_->parm );
    error |= nrrdResampleSamplesSet( this->rsmc_, axis, this->dimesions_[ axis ] );
    error |= nrrdResampleRangeFullSet( this->rsmc_, axis );
  }
  
  if ( error 
    || _nrrdResampleInputDimensionUpdate( this->rsmc_ )
    || _nrrdResampleInputCentersUpdate( this->rsmc_ )
    || _nrrdResampleInputSizesUpdate( this->rsmc_ )
    || _nrrdResampleLineAllocateUpdate( this->rsmc_ )
    || _nrrdResampleVectorAllocateUpdate( this->rsmc_ )
    || _nrrdResampleLineFillUpdate( this->rsmc_ )
    || _nrrdResampleVectorFillUpdate( this->rsmc_ )
    || _nrrdResamplePermutationUpdate( this->rsmc_ ) )
  {
    nrrdNuke( nout );
    return false;
  }

  nout->axis[ 0 ].size = this->dimesions_[ 0 ];
  nout->axis[ 1 ].size = this->dimesions_[ 1 ];
  nout->axis[ 2 ].size = this->dimesions_[ 2 ];
  nout->dim = 3;
  UpdateNrrdAxisInfo( nout, this->rsmc_ );
  Core::NrrdDataHandle nrrd_out( new Core::NrrdData( nout ) );
  grid_transform = nrrd_out->get_grid_transform();

  return true;
}

bool ResampleAlgo::nrrd_resmaple( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk )
{
  int error = 0;
  error |= nrrdResampleNrrdSet( this->rsmc_, nin );
  for ( unsigned int axis = 0; axis < nin->dim && !error; ++axis )
  {
    error |= nrrdResampleKernelSet( this->rsmc_, axis, unuk->kernel, unuk->parm );
    error |= nrrdResampleSamplesSet( this->rsmc_, axis, this->dimesions_[ axis ] );
    if ( this->resample_to_grid_ )
    {
      error |= nrrdResampleRangeSet( this->rsmc_, axis, this->range_min_[ axis ], 
        this->range_max_[ axis ] );
    }
    else
    {
      error |= nrrdResampleRangeFullSet( this->rsmc_, axis );
    }
  }
  if ( !error )
  {
    error |= nrrdResampleExecute( this->rsmc_, nout );
  }

  return !error;
}

void ResampleAlgo::resmaple_data_layer( DataLayerHandle input, DataLayerHandle output )
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
    input->get_grid_transform().transform() ) );
  Nrrd* nrrd_out = nrrdNew();
  if ( this->resample_to_grid_ )
  {
    if ( this->padding_ == ActionResample::ZERO_C )
    {
      nrrdResamplePadValueSet( this->rsmc_, 0.0 );
    }
    else if ( this->padding_ == ActionResample::MIN_C )
    {
      nrrdResamplePadValueSet( this->rsmc_, input->get_data_volume()->
        get_data_block()->get_min() );
    }
    else
    {
      nrrdResamplePadValueSet( this->rsmc_, input->get_data_volume()->
        get_data_block()->get_max() );
    }
  }
  output->update_progress_signal_( 0.1 );
  if ( !nrrd_resmaple( nrrd_in->nrrd(), nrrd_out, this->data_kernel_ ) )
  {
    nrrdNuke( nrrd_out );
    CORE_LOG_ERROR( "Failed to resample layer '" + input->get_layer_id() +"'" );
  }
  else if ( !this->check_abort() )
  {
    Core::NrrdDataHandle nrrd_data( new Core::NrrdData( nrrd_out ) );
    Core::DataBlockHandle data_block = Core::NrrdDataBlock::New( nrrd_data );
    Core::DataVolumeHandle data_volume( new Core::DataVolume( 
      nrrd_data->get_grid_transform(), data_block ) );
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
  Core::NrrdDataHandle nrrd_in( new Core::NrrdData( input_data_block,
    input->get_grid_transform().transform() ) );
  Nrrd* nrrd_out = nrrdNew();
  if ( this->resample_to_grid_ )
  {
    // Pad the mask layer with 0
    nrrdResamplePadValueSet( this->rsmc_, 0.0 );
  }
  output->update_progress_signal_( 0.1 );
  if ( !nrrd_resmaple( nrrd_in->nrrd(), nrrd_out, this->mask_kernel_ ) )
  {
    nrrdNuke( nrrd_out );
    CORE_LOG_ERROR( "Failed to resample layer '" + input->get_layer_id() +"'" );
  }
  else if ( !this->check_abort() )
  {
    Core::NrrdDataHandle nrrd_data( new Core::NrrdData( nrrd_out ) );
    Core::DataBlockHandle data_block = Core::NrrdDataBlock::New( nrrd_data );
    Core::MaskDataBlockHandle mask_data_block;
    if ( !( Core::MaskDataBlockManager::Convert( data_block, nrrd_data->get_grid_transform(),
      mask_data_block ) ) )
    {
      return;
    }

    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
      nrrd_data->get_grid_transform(), mask_data_block ) );
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
  if ( this->resample_to_grid_ )
  {
    this->detect_padding_only();
    nrrdResampleBoundarySet( this->rsmc_, nrrdBoundaryPad );
  }
  else
  {
    nrrdResampleBoundarySet( this->rsmc_, nrrdBoundaryBleed );
  }
  
  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    switch ( this->src_layers_[ i ]->get_type() )
    {
    case Core::VolumeType::DATA_E:
      this->resmaple_data_layer(
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
  if ( ( dst_spacing - src_spacing ).length2() > epsilon ) return;

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
  case Core::DataType::FLOAT_E:
    this->pad_and_crop_typed_data< float >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::DOUBLE_E:
    this->pad_and_crop_typed_data< double >( input_datablock, output_datablock, output );
    break;
  default:
    assert( false );
  }
  
  data_lock.unlock();
  
  if ( !this->check_abort() )
  {
    this->dispatch_insert_data_volume_into_layer( output, Core::DataVolumeHandle(
      new Core::DataVolume( output->get_grid_transform(), output_datablock ) ), true );
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
    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume(
      output->get_grid_transform(), dst_mask_data_block ) );
    
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
  this->add_argument( this->private_->layer_ids_ );
  this->add_argument( this->private_->x_ );
  this->add_argument( this->private_->y_ );
  this->add_argument( this->private_->z_ );

  // Action options
  this->add_key( this->private_->kernel_ );
  this->add_key( this->private_->param1_ );
  this->add_key( this->private_->param2_ );
  this->add_key( this->private_->replace_ );

  this->private_->resample_to_grid_ = false;
  this->private_->resample_to_layer_ = false;
}

bool ActionResample::validate( Core::ActionContextHandle& context )
{
  const std::vector< std::string >& layer_ids = this->private_->layer_ids_.value();
  if ( layer_ids.size() == 0 )
  {
    context->report_error( "No input layers specified" );
    return false;
  }
  
  for ( size_t i = 0; i < layer_ids.size(); ++i )
  {
    // Check for layer existence and type information
    std::string error;
    if ( !LayerManager::CheckLayerExistance( layer_ids[ i ], error ) )
    {
      context->report_error( error );
      return false;
    }
    
    // Check for layer availability 
    Core::NotifierHandle notifier;
    if ( !LayerManager::CheckLayerAvailability( layer_ids[ i ], 
      this->private_->replace_.value(), notifier ) )
    {
      context->report_need_resource( notifier );
      return false;
    }
  }

  if ( this->private_->resample_to_layer_ )
  {
    LayerHandle layer = LayerManager::Instance()->get_layer_by_id( 
      this->private_->dst_layer_id_ );
    if ( !layer )
    {
      context->report_error( "Invalid drop target" );
      return false;
    }
    Core::GridTransform grid_trans = layer->get_grid_transform();
    this->private_->x_.set_value( static_cast< int >( grid_trans.get_nx() ) );
    this->private_->y_.set_value( static_cast< int >( grid_trans.get_ny() ) );
    this->private_->z_.set_value( static_cast< int >( grid_trans.get_nz() ) );
    this->private_->grid_transform_ = grid_trans;
  }
  
  if ( this->private_->resample_to_grid_ || 
    this->private_->resample_to_layer_ )
  {
    if ( this->private_->padding_ != ZERO_C &&
      this->private_->padding_ != MIN_C &&
      this->private_->padding_ != MAX_C )
    {
      context->report_error( "Unknown padding option" );
      return false;
    }
  }
  else
  {
    if ( this->private_->x_.value() < 1 ||
      this->private_->y_.value() < 1 ||
      this->private_->z_.value() < 1 )
    {
      context->report_error( "Invalid resample size" );
      return false;
    }
  }
  
  if ( this->private_->kernel_.value() != BOX_C &&
    this->private_->kernel_.value() != TENT_C &&
    this->private_->kernel_.value() != CUBIC_BS_C &&
    this->private_->kernel_.value() != CUBIC_CR_C &&
    this->private_->kernel_.value() != QUARTIC_C &&
    this->private_->kernel_.value() != GAUSSIAN_C )
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
  boost::shared_ptr< ResampleAlgo > algo( new ResampleAlgo( this->private_->kernel_.value(), 
    this->private_->param1_.value(), this->private_->param2_.value() ) );

  // Set up parameters
  algo->replace_ = this->private_->replace_.value();
  algo->dimesions_[ 0 ] = static_cast< unsigned int >( this->private_->x_.value() );
  algo->dimesions_[ 1 ] = static_cast< unsigned int >( this->private_->y_.value() );
  algo->dimesions_[ 2 ] = static_cast< unsigned int >( this->private_->z_.value() );
  algo->resample_to_grid_ = this->private_->resample_to_grid_ || 
    this->private_->resample_to_layer_;
  algo->padding_ = this->private_->padding_;

  // Compute grid transform for the output layers
  const std::vector< std::string >& layer_ids = this->private_->layer_ids_.value();
  Core::GridTransform output_transform;
  if ( this->private_->resample_to_grid_ || this->private_->resample_to_layer_ )
  {
    output_transform = this->private_->grid_transform_;
  }
  else
  {
    algo->compute_output_grid_transform( layer_ids[ 0 ], output_transform );
  }

  // Set up input and output layers
  size_t num_of_layers = layer_ids.size();
  algo->src_layers_.resize( num_of_layers );
  algo->dst_layers_.resize( num_of_layers );
  std::vector< std::string > dst_layer_ids( num_of_layers );
  for ( size_t j = 0; j < num_of_layers; ++j )
  {
    size_t i = num_of_layers - 1 - j;
    algo->find_layer( layer_ids[ i ], algo->src_layers_[ i ] );
    if ( algo->replace_ )
    {
      algo->lock_for_processing( algo->src_layers_[ i ] );
    }
    else
    {
      algo->lock_for_use( algo->src_layers_[ i ] );
    }

    switch ( algo->src_layers_[ i ]->get_type() )
    {
    case Core::VolumeType::DATA_E:
      algo->create_and_lock_data_layer( output_transform, 
        algo->src_layers_[ i ], algo->dst_layers_[ i ] );
      break;
    case Core::VolumeType::MASK_E:
      algo->create_and_lock_mask_layer( output_transform,
        algo->src_layers_[ i ], algo->dst_layers_[ i ] );
      static_cast< MaskLayer* >( algo->dst_layers_[ i ].get() )->color_state_->set(
        static_cast< MaskLayer* >( algo->src_layers_[ i ].get() )->color_state_->get() );
      break;
    default:
      assert( false );
    }

    if ( !algo->dst_layers_[ i ] )
    {
      context->report_error( "Could not allocate enough memory." );
      return false;
    }
    
    dst_layer_ids[ i ] = algo->dst_layers_[ i ]->get_layer_id();
  }

  if ( this->private_->resample_to_layer_ )
  {
    LayerManager::Instance()->move_layer_above( algo->dst_layers_[ 0 ], 
      LayerManager::Instance()->get_layer_by_id( this->private_->dst_layer_id_ ) );
  }

  // Compute resample ranges if resampling to a given grid
  if ( this->private_->resample_to_grid_ || this->private_->resample_to_layer_ )
  {
    Core::Transform inverse_src_trans = algo->src_layers_[ 0 ]->
      get_grid_transform().get_inverse();
    // Compute the range of the destination grid in world space
    // NOTE: Since we assume cell centering, the actual range of the grid should be
    // extended by half a voxel in each direction
    Core::Point start( -0.5, -0.5, -0.5 );
    Core::Point end( algo->dimesions_[ 0 ] - 0.5, algo->dimesions_[ 1 ] - 0.5, 
      algo->dimesions_[ 2 ] - 0.5 );
    start = this->private_->grid_transform_ * start;
    end = this->private_->grid_transform_ * end;

    // Compute the resample range relative to the input in index space
    start = inverse_src_trans * start;
    end = inverse_src_trans * end;
    for ( int i = 0; i < 3; ++i )
    {
      algo->range_min_[ i ] = start[ i ];
      algo->range_max_[ i ] = end[ i ];
    }
  }

  // Return the ids of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( dst_layer_ids ) );

  // Build the undo-redo record
  algo->create_undo_redo_record( context, this->shared_from_this() );

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
  action->private_->layer_ids_.set_value( layer_ids );
  action->private_->x_.set_value( x );
  action->private_->y_.set_value( y );
  action->private_->z_.set_value( z );
  action->private_->kernel_.set_value( kernel );
  action->private_->param1_.set_value( param1 );
  action->private_->param2_.set_value( param2 );
  action->private_->replace_.set_value( replace );

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionResample::Dispatch( Core::ActionContextHandle context, 
                const std::vector< std::string >& layer_ids, 
                const Core::GridTransform& grid_trans, 
                const std::string& padding, const std::string& kernel, 
                double param1, double param2, bool replace )
{
  ActionResample* action = new ActionResample;
  action->private_->layer_ids_.set_value( layer_ids );
  action->private_->grid_transform_ = grid_trans;
  action->private_->resample_to_grid_ = true;
  action->private_->padding_ = padding;
  
  int nx = static_cast< int >( grid_trans.get_nx() );
  int ny = static_cast< int >( grid_trans.get_ny() );
  int nz = static_cast< int >( grid_trans.get_nz() );
  action->private_->x_.set_value( nx );
  action->private_->y_.set_value( ny );
  action->private_->z_.set_value( nz );

  action->private_->kernel_.set_value( kernel );
  action->private_->param1_.set_value( param1 );
  action->private_->param2_.set_value( param2 );
  action->private_->replace_.set_value( replace );

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionResample::Dispatch( Core::ActionContextHandle context, 
                const std::string& src_layer, const std::string& dst_layer, 
                const std::string& padding, const std::string& kernel, 
                double param1, double param2, bool replace )
{
  ActionResample* action = new ActionResample;

  std::vector< std::string > layer_ids( 1, src_layer );
  action->private_->layer_ids_.set_value( layer_ids );
  action->private_->dst_layer_id_ = dst_layer;
  action->private_->resample_to_layer_ = true;

  action->private_->padding_ = padding;

  action->private_->kernel_.set_value( kernel );
  action->private_->param1_.set_value( param1 );
  action->private_->param2_.set_value( param2 );
  action->private_->replace_.set_value( replace );

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
