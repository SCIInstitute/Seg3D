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
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/Actions/ActionArithmeticFilter.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/StatusBar/StatusBar.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, ArithmeticFilter )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.
//////////////////////////////////////////////////////////////////////////

class ArithmeticFilterAlgo : public LayerFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;
  Core::ArrayMathEngine engine_;

  bool preserve_data_format_;

public:

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run_filter();

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Arithmetic Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Arithmetic";
  }
};


void ArithmeticFilterAlgo::run_filter()
{
  std::string error;
  if ( !this->engine_.run( error ) )
  {
    CORE_LOG_ERROR( error );
    return;
  }

  if ( !this->check_abort() )
  {
    Core::DataBlockHandle output_data_block;
    this->engine_.get_data_block( ActionArithmeticFilter::RESULT_C, output_data_block );
    
    if ( this->dst_layer_->get_type() == Core::VolumeType::MASK_E )
    {
      Core::MaskDataBlockHandle mask_data_block;
      Core::MaskDataBlockManager::ConvertLargerThan( output_data_block, 
        this->dst_layer_->get_grid_transform(), mask_data_block );
      this->dispatch_insert_mask_volume_into_layer( this->dst_layer_, Core::MaskVolumeHandle(
        new Core::MaskVolume( this->dst_layer_->get_grid_transform(), 
        mask_data_block ) ) );
    }
    else
    {
      if ( this->preserve_data_format_ && 
        output_data_block->get_data_type() != this->src_layer_->get_data_type() )
      {

        Core::DataBlockHandle converted_data_block;
        Core::DataBlock::ConvertDataType( output_data_block, converted_data_block, 
          this->src_layer_->get_data_type() ); 
        output_data_block = converted_data_block;
      }
      this->dispatch_insert_data_volume_into_layer( this->dst_layer_, Core::DataVolumeHandle(
        new Core::DataVolume( this->dst_layer_->get_grid_transform(), output_data_block ) ), 
        true );
    }
  }

  this->dst_layer_->update_progress( 1.0 );
}

//////////////////////////////////////////////////////////////////////////
// Class ActionArithmeticFilterPrivate
//////////////////////////////////////////////////////////////////////////

class ActionArithmeticFilterPrivate
{
public:
  // VALIDATE:
  // Validate the filter inputs, outputs and expression prior to running filter.
  bool validate_parser( Core::ActionContextHandle& context );

  std::vector< std::string > layer_ids_;
  std::string expressions_;
  std::string output_type_;
  bool replace_;
  bool preserve_data_format_;
  SandboxID sandbox_;
  boost::shared_ptr< ArithmeticFilterAlgo > algo_;
};

bool ActionArithmeticFilterPrivate::validate_parser( Core::ActionContextHandle& context )
{
  // Validate algorithm
  // Create algorithm
  this->algo_ = boost::shared_ptr< ArithmeticFilterAlgo >( new ArithmeticFilterAlgo );
  this->algo_->set_sandbox( this->sandbox_ );

  bool replace = this->replace_;

  const std::vector< std::string >& layer_ids = this->layer_ids_;
  std::vector< LayerHandle > layers( layer_ids.size() );

  std::string name( "A" );
  for ( size_t i = 0; i < layer_ids.size(); ++i )
  {
    name[ 0 ] = static_cast< char >( 'A' + i );
    if ( this->algo_->find_layer( layer_ids[ i ], layers[ i ] ) )
    {
      Core::DataBlockHandle input_data_block;
      switch ( layers[ i ]->get_type() )
      {
      case Core::VolumeType::MASK_E:
        if ( ! ( Core::MaskDataBlockManager::Convert( dynamic_cast< MaskLayer* >( 
          layers[ i ].get() )->get_mask_volume()->get_mask_data_block(), 
          input_data_block, Core::DataType::UCHAR_E ) ) )
        {
          context->report_error( 
            std::string("Arithmetic Filter: Running out of memory.") );
          this->algo_.reset();
          return false;
        }
        break;
      case Core::VolumeType::DATA_E:
        if ( ! ( input_data_block = dynamic_cast< DataLayer* >( layers[ i ].get() )->
          get_data_volume()->get_data_block() ) )
        {
          context->report_error( 
            std::string("Arithmetic Filter: Running out of memory.") );
          this->algo_.reset();
          return false;
        }
        break;
      }

      std::string error;
      if ( ! this->algo_->engine_.add_input_data_block( name, input_data_block, error ) )
      {
        context->report_error( error );
        this->algo_.reset();
        return false;
      }

      if ( i == 0 && replace )
      {
        if ( ! ( this->algo_->lock_for_processing( layers[ i ] ) ) )
        {
          context->report_error( "Could not lock target layer." );
          this->algo_.reset();
          return false;
        }
      }
      else
      {
        // Check if it was already locked
        bool already_locked = false;
        for ( size_t  j = 0; j < i; j++ )
        {
          if ( layers[ i ] == layers[ j ] )
          {
            already_locked = true;
            break;
          }
        }
        if ( ! already_locked )
        {
          if ( ! ( this->algo_->lock_for_use( layers[ i ] ) ) )
          {
            context->report_error( "Could not lock layer." );
            this->algo_.reset();
            return false;       
          }
        }
      }
    }
  }

  // We must have a valid first input layer
  if( !layers[ 0 ] )
  {
    context->report_error( "Invalid target layer." );
    return false;
  }

  if ( replace )
  {
    this->algo_->dst_layer_ = layers[ 0 ];
    // Make sure the dst layer type matches the output type
    if( !( ( this->algo_->dst_layer_->get_type() == Core::VolumeType::MASK_E && 
      this->output_type_ ==  ActionArithmeticFilter::MASK_C ) ||
      ( this->algo_->dst_layer_->get_type() == Core::VolumeType::DATA_E && 
      this->output_type_ ==  ActionArithmeticFilter::DATA_C ) ) )
    {
      context->report_error( 
        std::string("Arithmetic Filter: Cannot replace first input -- input/output types don't match.") );
      return false;
    }
  }
  else if ( this->output_type_ == ActionArithmeticFilter::DATA_C )
  {
    if ( ! (this->algo_->create_and_lock_data_layer_from_layer( layers[ 0 ], 
      this->algo_->dst_layer_ ) ) )
    {
      context->report_error( "Could not create output layer." );
      this->algo_.reset();
      return false;   
    }
  }
  else
  {
    if ( ! ( this->algo_->create_and_lock_mask_layer_from_layer( layers[ 0 ], 
      this->algo_->dst_layer_ ) ) )
    {
      context->report_error( "Could not create output layer." );
      this->algo_.reset();
      return false;   
    }
  }

  if( this->preserve_data_format_ )
  {
    this->algo_->src_layer_ = layers[ 0 ];
  }

  const Core::GridTransform& grid_trans = layers[ 0 ]->get_grid_transform();
  std::string error;

  if ( this->algo_->dst_layer_->get_type() == Core::VolumeType::MASK_E )
  {
    if( !this->algo_->engine_.add_output_data_block( ActionArithmeticFilter::RESULT_C, 
      grid_trans.get_nx(), grid_trans.get_ny(), grid_trans.get_nz(), 
      Core::DataType::CHAR_E, error ) )
    {
      context->report_error( error );
      this->algo_.reset();
      return false;
    }
  }
  else
  {
    if( !this->algo_->engine_.add_output_data_block( ActionArithmeticFilter::RESULT_C, 
      grid_trans.get_nx(), grid_trans.get_ny(), grid_trans.get_nz(), 
      Core::DataType::FLOAT_E, error ) )
    {
      context->report_error( error );
      this->algo_.reset();
      return false;
    }
  }

  this->algo_->engine_.add_expressions( this->expressions_ );

  if( !this->algo_->engine_.parse_and_validate( error ) )
  {
    context->report_error( error );
    // We need algo to go out of scope so that layers are unlocked
    this->algo_.reset();
    return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////
// Class ActionArithmeticFilter
//////////////////////////////////////////////////////////////////////////

const std::string ActionArithmeticFilter::DATA_C( "data" );
const std::string ActionArithmeticFilter::MASK_C( "mask" );
const std::string ActionArithmeticFilter::RESULT_C( "RESULT" );

ActionArithmeticFilter::ActionArithmeticFilter() :
  private_( new ActionArithmeticFilterPrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->expressions_ );
  this->add_parameter( this->private_->output_type_ );
  this->add_parameter( this->private_->replace_ );
  this->add_parameter( this->private_->preserve_data_format_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionArithmeticFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) ) return false;

  if ( this->private_->layer_ids_.size() == 0 )
  {
    context->report_error( "No input layers specified" );
    return false;
  }
  
  // Validate layers
  Core::GridTransform grid_trans;
  for ( size_t i = 0; i < this->private_->layer_ids_.size(); ++i )
  {
    if ( this->private_->layer_ids_[ i ] == "<none>" || this->private_->layer_ids_[ i ] == "" ) continue;
    // Check for layer existence
    LayerHandle layer = LayerManager::FindLayer( this->private_->layer_ids_[ i ], 
      this->private_->sandbox_ );
    if ( !layer )
    {
      context->report_error( "Layer '" + this->private_->layer_ids_[ i ] + "' doesn't exist" );
      return false;
    }

    // Make sure that all the layers are in the same group
    if ( i == 0 )
    {
      grid_trans = layer->get_grid_transform();
    }
    else if ( grid_trans != layer->get_grid_transform() )
    {
      context->report_error( "Input layers do not belong to the same group" );
      return false;
    }
    
    // Check for layer availability 
    if ( ! LayerManager::CheckLayerAvailability( this->private_->layer_ids_[ i ], 
      i == 0 && this->private_->replace_, context, this->private_->sandbox_ ) ) return false;
  }

  // Validate parser inputs, outputs, and expression
  if( !this->private_->validate_parser( context ) )
  {
    return false;
  }
  
  // Validation successful
  return true;
}

bool ActionArithmeticFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Copy the parameters over to the algorithm that runs the filter
  this->private_->algo_->preserve_data_format_ = this->private_->preserve_data_format_;

  // Connect ArrayMathEngine progress signal to output layer progress signal
  this->private_->algo_->engine_.update_progress_signal_.connect(
    boost::bind( &Layer::update_progress, this->private_->algo_->dst_layer_, _1, 0.0, 1.0 ) );

  // Return the ids of the destination layer.
  result.reset( new Core::ActionResult( this->private_->algo_->dst_layer_->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( this->private_->algo_->get_notifier() );
  }

  // Create undo/redo record for this layer action
  this->private_->algo_->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

  // Start the filter.
  Core::Runnable::Start( this->private_->algo_ );

  // We need algo to go out of scope once it finishes so that layers are unlocked
  this->private_->algo_.reset();

  return true;
}

void ActionArithmeticFilter::Dispatch( Core::ActionContextHandle context, 
                    const std::vector< std::string >& layer_ids, 
                    const std::string& expressions, 
                    const std::string& output_type, 
                    bool replace, bool preserve_data_format )
{
  ActionArithmeticFilter* action = new ActionArithmeticFilter;
  action->private_->layer_ids_= layer_ids;
  action->private_->expressions_ = expressions;
  action->private_->output_type_ = output_type;
  action->private_->replace_ = replace;
  action->private_->preserve_data_format_ = preserve_data_format;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
