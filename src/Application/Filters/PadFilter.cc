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

#include <Application/Filters/PadFilter.h>

#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>

using namespace Filter;
using namespace Seg3D;
using namespace Core;

PadFilter::PadFilter( bool replace, const std::string& padding, SandboxID sandbox )
: replace_(replace),
  padding_(padding)
{
  this->set_sandbox( sandbox );
}


bool PadFilter::setup_layers( const std::vector< std::string >& layer_ids, const Core::GridTransform& grid_transform )
{
  size_t num_layers = layer_ids.size();
  this->src_layers_.resize( num_layers );
  this->dst_layers_.resize( num_layers );
  this->dst_layer_ids_.resize( num_layers );
  this->output_transforms_.resize( num_layers );

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

    this->output_transforms_[ i ] = grid_transform;
    this->output_transforms_[ i ].set_originally_node_centered( this->src_layers_[ i ]->get_grid_transform().get_originally_node_centered() );

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

  return true;
}

void PadFilter::pad_data_layer( DataLayerHandle input, DataLayerHandle output )
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

void PadFilter::pad_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
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

void PadFilter::run_filter()
{
  this->pad_internals_.reset( new PadFilterInternals( this->src_layers_[0], this->dst_layers_[0], this->padding_ ) );

  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    if ( this->src_layers_[ i ]->get_type() == VolumeType::DATA_E )
    {
      DataLayerHandle src_data_layer = boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] );
      if ( ! src_data_layer )
      {
        this->report_error("Error obtaining source data layer.");
        return;
      }

      DataLayerHandle dst_data_layer = boost::dynamic_pointer_cast< DataLayer >( this->dst_layers_[ i ] );
      if ( ! dst_data_layer )
      {
        this->report_error("Error obtaining destination data layer.");
        return;
      }

      this->pad_data_layer( src_data_layer, dst_data_layer );
    }
    else if ( this->src_layers_[ i ]->get_type() == VolumeType::MASK_E )
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
    }
    else
    {
      CORE_LOG_WARNING("Attempting to pad unsupported layer type.");
    }

    if ( ! this->dst_layers_[ i ] )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }

    if ( this->check_abort() ) break;
  }
}

