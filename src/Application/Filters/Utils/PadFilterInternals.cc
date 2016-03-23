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

#include <Application/Filters/Utils/PadFilterInternals.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Utils/PadValues.h>

#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Utils/Log.h>


using namespace Filter;
using namespace Seg3D;
using namespace Core;


PadFilterInternals::PadFilterInternals(LayerHandle src_layer, LayerHandle dst_layer, const std::string& padding)
: padding_( padding ),
  EPSILON(1e-2),
  src_trans_( src_layer->get_grid_transform() ),
  dst_trans_( dst_layer->get_grid_transform() )
{
  Point src_origin = src_trans_ * Point( 0.0, 0.0, 0.0 );
  Transform inv_dst_trans = dst_trans_.get_inverse();
  src_origin_to_dst_index_ = inv_dst_trans * src_origin;

  // Compute the range of the source volume mapped to the destination volume in index space
  this->mapped_x_start_ = static_cast< int >( src_origin_to_dst_index_[ 0 ] );
  this->mapped_y_start_ = static_cast< int >( src_origin_to_dst_index_[ 1 ] );
  this->mapped_z_start_ = static_cast< int >( src_origin_to_dst_index_[ 2 ] );
  this->overlap_x_start_ = Max( 0, this->mapped_x_start_ );
  this->overlap_y_start_ = Max( 0, this->mapped_y_start_ );
  this->overlap_z_start_ = Max( 0, this->mapped_z_start_ );
  this->overlap_nx_ = Max( Min( this->mapped_x_start_ + static_cast< int >( src_trans_.get_nx() ),
                                static_cast< int >( dst_trans_.get_nx() ) ) -
                                this->overlap_x_start_, 0 );
  this->overlap_ny_ = Max( Min( this->mapped_y_start_ + static_cast< int >( src_trans_.get_ny() ),
                                static_cast< int >( dst_trans_.get_ny() ) ) -
                                this->overlap_y_start_, 0 );
  this->overlap_nz_ = Max( Min( this->mapped_z_start_ + static_cast< int >( src_trans_.get_nz() ),
                                static_cast< int >( dst_trans_.get_nz() ) ) -
                                this->overlap_z_start_, 0 );
}

// for use in resample filters
bool PadFilterInternals::detect_padding_only()
{
  // Compare spacing
  Vector src_spacing = src_trans_ * Vector( 1.0, 1.0, 1.0 );
  Vector dst_spacing = dst_trans_ * Vector( 1.0, 1.0, 1.0 );
  if ( Abs( src_spacing[ 0 ] - dst_spacing[ 0 ] ) > EPSILON * src_spacing[ 0 ] ||
       Abs( src_spacing[ 1 ] - dst_spacing[ 1 ] ) > EPSILON * src_spacing[ 1 ] ||
       Abs( src_spacing[ 2 ] - dst_spacing[ 2 ] ) > EPSILON * src_spacing[ 2 ] )
  {
    return false;
  }

  // Check if source grid aligns with destination grid
  if ( Abs( Fraction( this->src_origin_to_dst_index_[ 0 ] ) ) > EPSILON ||
       Abs( Fraction( this->src_origin_to_dst_index_[ 1 ] ) ) > EPSILON ||
       Abs( Fraction( this->src_origin_to_dst_index_[ 2 ] ) ) > EPSILON )
  {
    return false;
  }

  CORE_LOG_DEBUG( "Resample tool: padding only situation detected." );

  return true;
}

template< class T >
void PadFilterInternals::pad_and_crop_typed_data( DataBlockHandle src,
                                                  DataBlockHandle dst,
                                                  DataLayerHandle output_layer,
                                                  LayerAbstractFilterHandle layer_filter )
{
  int src_nx = static_cast< int >( src_trans_.get_nx() );
  int src_ny = static_cast< int >( src_trans_.get_ny() );
  int src_nxy = src_nx * src_ny;

  int dst_nx = static_cast< int >( dst_trans_.get_nx() );
  int dst_ny = static_cast< int >( dst_trans_.get_ny() );
  int dst_nz = static_cast< int >( dst_trans_.get_nz() );
  int dst_nxy = dst_nx * dst_ny;

  const T* src_data = reinterpret_cast< T* >( src->get_data() );
  T* dst_data = reinterpret_cast< T* >( dst->get_data() );

  T padding_val;
  if ( this->padding_ == PadValues::ZERO_C )
  {
    padding_val = T( 0 );
  }
  else if ( this->padding_ == PadValues::MIN_C )
  {
    padding_val = static_cast< T >( src->get_min() );
  }
  else
  {
    padding_val = static_cast< T >( src->get_max() );
  }

  // Pad the non-overlapped part in Z-direction
  for ( int z = 0; z < this->overlap_z_start_; ++z )
  {
    if ( layer_filter->check_abort() )  return;

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
    if ( layer_filter->check_abort() )  return;

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
    if ( layer_filter->check_abort() )  return;

    int start_index = z * dst_nxy;
    for ( int i = 0; i < dst_nxy; ++i )
    {
      dst_data[ start_index + i ] = padding_val;
    }
  }

  output_layer->update_progress_signal_( 1.0 );
}


DataBlockHandle PadFilterInternals::pad_and_crop_data_layer( DataLayerHandle input, DataLayerHandle output, LayerAbstractFilterHandle layer_filter )
{
  DataBlockHandle input_datablock = input->get_data_volume()->get_data_block();
  DataBlockHandle output_datablock = StdDataBlock::New( output->get_grid_transform(), input_datablock->get_data_type() );
  if ( ! output_datablock )
  {
    CORE_LOG_ERROR( "Could not allocate enough memory" );
    return DataBlockHandle();
  }

  DataBlock::shared_lock_type data_lock( input_datablock->get_mutex() );
  switch ( input_datablock->get_data_type() )
  {
    case DataType::CHAR_E:
      this->pad_and_crop_typed_data< signed char >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::UCHAR_E:
      this->pad_and_crop_typed_data< unsigned char >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::SHORT_E:
      this->pad_and_crop_typed_data< short >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::USHORT_E:
      this->pad_and_crop_typed_data< unsigned short >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::INT_E:
      this->pad_and_crop_typed_data< int >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::UINT_E:
      this->pad_and_crop_typed_data< unsigned int >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::LONGLONG_E:
      this->pad_and_crop_typed_data< long long >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::ULONGLONG_E:
      this->pad_and_crop_typed_data< unsigned long long >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::FLOAT_E:
      this->pad_and_crop_typed_data< float >( input_datablock, output_datablock, output, layer_filter );
      break;
    case DataType::DOUBLE_E:
      this->pad_and_crop_typed_data< double >( input_datablock, output_datablock, output, layer_filter );
      break;
    default:
      CORE_LOG_ERROR("Unsupported data type");
      return DataBlockHandle();
  }

  data_lock.unlock();
  return output_datablock;
}

DataBlockHandle PadFilterInternals::pad_and_crop_mask_layer( MaskLayerHandle input, MaskLayerHandle output, LayerAbstractFilterHandle layer_filter )
{
  MaskDataBlockHandle input_mask = input->get_mask_volume()->get_mask_data_block();
  DataBlockHandle output_mask = StdDataBlock::New( output->get_grid_transform(), DataType::UCHAR_E );
  if ( ! output_mask )
  {
    CORE_LOG_ERROR( "Could not allocate enough memory" );
    return DataBlockHandle();
  }

  MaskDataBlock::shared_lock_type data_lock( input_mask->get_mutex() );
  const unsigned char* src_data = input_mask->get_mask_data();
  unsigned char* dst_data = reinterpret_cast< unsigned char* >( output_mask->get_data() );
  unsigned char mask_value = input_mask->get_mask_value();

  GridTransform src_trans_ = input->get_grid_transform();
  int src_nx = static_cast< int >( src_trans_.get_nx() );
  int src_ny = static_cast< int >( src_trans_.get_ny() );
  int src_nxy = src_nx * src_ny;

  GridTransform dst_trans_ = output->get_grid_transform();
  int dst_nx = static_cast< int >( dst_trans_.get_nx() );
  int dst_ny = static_cast< int >( dst_trans_.get_ny() );
  int dst_nz = static_cast< int >( dst_trans_.get_nz() );
  int dst_nxy = dst_nx * dst_ny;

  // Pad the non-overlapped part in Z-direction
  for ( int z = 0; z < this->overlap_z_start_; ++z )
  {
    if ( layer_filter->check_abort() ) return DataBlockHandle();

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
    if ( layer_filter->check_abort() ) return DataBlockHandle();

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
    if ( layer_filter->check_abort() ) return DataBlockHandle();

    int start_index = z * dst_nxy;
    for ( int i = 0; i < dst_nxy; ++i )
    {
      dst_data[ start_index + i ] = 0;
    }
  }

  output->update_progress_signal_( 1.0 );

  data_lock.unlock();
  return output_mask;
}

