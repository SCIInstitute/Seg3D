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
  bool replace_;
  unsigned int dimesions_[ 3 ];

  NrrdKernelSpec* mask_kernel_;
  NrrdKernelSpec* data_kernel_;
  NrrdResampleContext* rsmc_;

public:

  bool nrrd_resmaple( Nrrd* nin, Nrrd* nout, NrrdKernelSpec* unuk )
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

  void resmaple_data_layer( DataLayerHandle input )
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
      data_block->update_histogram();
      Core::DataVolumeHandle data_volume( new Core::DataVolume( 
        nrrd_data->get_grid_transform(), data_block ) );
      std::string name = this->get_filter_name() + "_" + input->get_layer_name();
      LayerManager::DispatchCreateAndInsertDataLayer( name, data_volume );
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

  void resample_mask_layer( MaskLayerHandle input )
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
      mask_data_block->increase_generation();
      Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
        nrrd_data->get_grid_transform(), mask_data_block ) );
      std::string name = this->get_filter_name() + "_" + input->get_layer_name();
      LayerManager::DispatchCreateAndInsertMaskLayer( name, mask_volume );
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

  // RUN:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run()
  {
    this->rsmc_ = nrrdResampleContextNew();
    this->rsmc_->verbose = 0;

    for ( size_t i = 0; i < this->src_layers_.size(); ++i )
    {
      switch ( this->src_layers_[ i ]->type() )
      {
      case Core::VolumeType::DATA_E:
        this->resmaple_data_layer(
          boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] ) );
        break;
      case Core::VolumeType::MASK_E:
        this->resample_mask_layer(
          boost::dynamic_pointer_cast< MaskLayer >( this->src_layers_[ i ] ) );
        break;
      }

      if ( this->check_abort() ) break;
    }
    
    nrrdKernelSpecNix( this->data_kernel_ );
    nrrdKernelSpecNix( this->mask_kernel_ );
    nrrdResampleContextNix( this->rsmc_ );
  }

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Resample";
  }
};

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
  boost::shared_ptr< ResampleAlgo > algo( new ResampleAlgo );

  // Set up parameters and kernels
  algo->replace_ = this->private_->replace_.value();
  algo->dimesions_[ 0 ] = static_cast< unsigned int >( this->private_->x_.value() );
  algo->dimesions_[ 1 ] = static_cast< unsigned int >( this->private_->y_.value() );
  algo->dimesions_[ 2 ] = static_cast< unsigned int >( this->private_->z_.value() );

  algo->mask_kernel_ = nrrdKernelSpecNew();
  // For resampling mask layers, any kernel other than "cheap" doesn't make sense
  algo->mask_kernel_->kernel = nrrdKernelCheap;
  algo->mask_kernel_->parm[ 0 ] = 1.0;

  algo->data_kernel_ = nrrdKernelSpecNew();
  algo->data_kernel_->parm[ 0 ] = 1.0;
  if ( this->private_->kernel_.value() == BOX_C )
  {
    algo->data_kernel_->kernel = nrrdKernelBox;
  }
  else if ( this->private_->kernel_.value() == TENT_C )
  {
    algo->data_kernel_->kernel = nrrdKernelTent;
  }
  else if ( this->private_->kernel_.value() == CUBIC_BS_C )
  {
    algo->data_kernel_->kernel = nrrdKernelBCCubic;
    algo->data_kernel_->parm[ 1 ] = 1.0;
    algo->data_kernel_->parm[ 2 ] = 0.0;
  }
  else if ( this->private_->kernel_.value() == CUBIC_CR_C )
  {
    algo->data_kernel_->kernel = nrrdKernelBCCubic;
    algo->data_kernel_->parm[ 1 ] = 0.0;
    algo->data_kernel_->parm[ 2 ] = 0.5;
  }
  else if ( this->private_->kernel_.value() == QUARTIC_C )
  {
    algo->data_kernel_->kernel = nrrdKernelAQuartic;
    algo->data_kernel_->parm[ 1 ] = 0.0834;
  }
  else
  {
    algo->data_kernel_->kernel = nrrdKernelGaussian;
    algo->data_kernel_->parm[ 0 ] = this->private_->param1_.value();
    algo->data_kernel_->parm[ 1 ] = this->private_->param2_.value();
  }
  
  // Set up input and output layers
  const std::vector< std::string > layer_ids = this->private_->layer_ids_.value();
  size_t num_of_layers = layer_ids.size();
  algo->src_layers_.resize( num_of_layers );
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
  }
  
  // Return the ids of the destination layer.
  //result = Core::ActionResultHandle( new Core::ActionResult( dst_layer_ids ) );

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
