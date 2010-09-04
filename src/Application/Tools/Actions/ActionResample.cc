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
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tools/Actions/ActionResample.h>
#include <Application/Tool/BaseFilter.h>

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
};

//////////////////////////////////////////////////////////////////////////
// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.
//////////////////////////////////////////////////////////////////////////

class ResampleAlgo : public BaseFilter
{

public:
  std::vector< LayerHandle > src_layers_;
  std::vector< LayerHandle > dst_layers_;
  bool replace_;
  unsigned int dimesions_[ 3 ];

  NrrdKernelSpec* mask_kernel_;
  NrrdKernelSpec* data_kernel_;
  NrrdResampleContext* rsmc_;

public:
  ResampleAlgo( const std::string& kernel, double param1, double param2 );
  virtual ~ResampleAlgo();

  bool compute_output_grid_transform( const std::string& input_layerid, 
    Core::GridTransform& grid_transform );

  bool nrrd_resmaple( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk );
  void resmaple_data_layer( DataLayerHandle input, DataLayerHandle output );
  void resample_mask_layer( MaskLayerHandle input, MaskLayerHandle output );

  // RUN:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run();

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Resample";
  }
};

ResampleAlgo::ResampleAlgo( const std::string& kernel, double param1, double param2 )
{
  this->rsmc_ = nrrdResampleContextNew();
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
  switch ( layer->type() )
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
    error |= nrrdResampleRangeFullSet( this->rsmc_, axis );
  }
  if ( !error )
  {
    error |= nrrdResampleExecute( this->rsmc_, nout );
  }

  return !error;
}

void ResampleAlgo::resmaple_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  Core::NrrdDataHandle nrrd_in( new Core::NrrdData( 
    input->get_data_volume()->get_data_block(),
    input->get_grid_transform().transform() ) );
  Nrrd* nrrd_out = nrrdNew();
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
    this->dispatch_insert_data_volume_into_layer( output, data_volume, false, true );
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
  Core::DataBlockHandle input_data_block;
  Core::MaskDataBlockManager::Convert( input->get_mask_volume()->get_mask_data_block(),
    input_data_block, Core::DataType::UCHAR_E );
  Core::NrrdDataHandle nrrd_in( new Core::NrrdData( input_data_block,
    input->get_grid_transform().transform() ) );
  Nrrd* nrrd_out = nrrdNew();
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
    Core::MaskDataBlockManager::Convert( data_block, nrrd_data->get_grid_transform(),
      mask_data_block );
    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
      nrrd_data->get_grid_transform(), mask_data_block ) );
    this->dispatch_insert_mask_volume_into_layer( output, mask_volume, true );
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

void ResampleAlgo::run()
{
  this->rsmc_ = nrrdResampleContextNew();
  this->rsmc_->verbose = 0;

  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    switch ( this->src_layers_[ i ]->type() )
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

    if ( this->check_abort() ) break;
  }
}

//////////////////////////////////////////////////////////////////////////
// Class ActionResample
//////////////////////////////////////////////////////////////////////////

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
}

bool ActionResample::validate( Core::ActionContextHandle& context )
{
  const std::vector< std::string > layer_ids = this->private_->layer_ids_.value();
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
  
  if ( this->private_->x_.value() < 1 ||
    this->private_->y_.value() < 1 ||
    this->private_->z_.value() < 1 )
  {
    context->report_error( "Invalid resample size" );
    return false;
  }
  
  if ( this->private_->kernel_.value() != BOX_C &&
    this->private_->kernel_.value() != TENT_C &&
    this->private_->kernel_.value() != CUBIC_BS_C &&
    this->private_->kernel_.value() != CUBIC_CR_C &&
    this->private_->kernel_.value() != QUARTIC_C &&
    this->private_->kernel_.value() != GAUSSIAN_C )
  {
    context->report_error( "Unkown kernel type" );
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

  // Set up input and output layers
  const std::vector< std::string > layer_ids = this->private_->layer_ids_.value();
  Core::GridTransform output_transform;
  algo->compute_output_grid_transform( layer_ids[ 0 ], output_transform );
  size_t num_of_layers = layer_ids.size();
  algo->src_layers_.resize( num_of_layers );
  algo->dst_layers_.resize( num_of_layers );
  std::vector< std::string > dst_layer_ids( num_of_layers );
  for ( size_t i = 0; i < num_of_layers; ++i )
  {
    algo->find_layer( layer_ids[ i ], algo->src_layers_[ i ] );
    if ( algo->replace_ )
    {
      algo->lock_for_processing( algo->src_layers_[ i ] );
    }
    else
    {
      algo->lock_for_use( algo->src_layers_[ i ] );
    }

    switch ( algo->src_layers_[ i ]->type() )
    {
    case Core::VolumeType::DATA_E:
      algo->create_and_lock_data_layer( output_transform, 
        algo->src_layers_[ i ], algo->dst_layers_[ i ] );
      break;
    case Core::VolumeType::MASK_E:
      algo->create_and_lock_mask_layer( output_transform,
        algo->src_layers_[ i ], algo->dst_layers_[ i ] );
      break;
    default:
      assert( false );
    }
    dst_layer_ids[ i ] = algo->dst_layers_[ i ]->get_layer_id();
  }
  
  // Return the ids of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( dst_layer_ids ) );

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

} // end namespace Seg3D
