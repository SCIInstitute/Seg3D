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

#include <Core/Volume/MaskVolume.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <bitset>


namespace Core
{

MaskVolume::MaskVolume( const GridTransform& grid_transform, 
  const MaskDataBlockHandle& mask_data_block ) :
  Volume( grid_transform ),
  mask_data_block_( mask_data_block )
{
}

MaskVolume::MaskVolume( const GridTransform& grid_transform ) :
  Volume( grid_transform )
{
    MaskDataBlockManager::Instance()->create( grid_transform, mask_data_block_ );
}

VolumeType MaskVolume::get_type() const
{
  return VolumeType::MASK_E;
}

MaskDataBlockHandle MaskVolume::mask_data_block() const
{
  return this->mask_data_block_;
}

template< class DATA >
static bool CreateMaskFromNonZeroDataInternal( const DataVolumeHandle& data, 
  MaskDataBlockHandle& mask )
{
  DATA* src   = reinterpret_cast<DATA*>( data->data_block()->get_data() ); 
  size_t size = mask->get_size();

  unsigned char* mask_data  = mask->get_mask_data();
  unsigned char  mask_value = mask->get_mask_value();
  unsigned char  not_mask_value = ~( mask->get_mask_value() );

  for ( size_t j = 0; j < size; j++ )
  {
    if ( src[ j ] ) mask_data[ j ] |= mask_value;
    else mask_data[ j ] &= not_mask_value;
  }

  return true;
}

bool MaskVolume::CreateMaskFromNonZeroData( const DataVolumeHandle data, 
    MaskDataBlockHandle& mask )
{
  // Ensure there is no valid pointer left in the handle
  mask.reset();

  // Check if there is any data
  if ( !data ) return false;

  // Lock the source data
  DataVolume::lock_type lock( data->get_mutex( ) );

  // Create a new mask data block
  if ( !( MaskDataBlockManager::Instance()->create( data->get_grid_transform(), mask ) ) )
  {
    // Could not create a valid mask data block
    return false;
  }

  // Lock the mask layer as it may contain additional masks that are currently in use
  // Hence we need to have full access
  MaskDataBlock::lock_type mask_lock( mask->get_mutex() );

  switch( data->get_type() )
  {
  case DataType::CHAR_E:
    return CreateMaskFromNonZeroDataInternal<signed char>( data, mask );
  case DataType::UCHAR_E:
    return CreateMaskFromNonZeroDataInternal<unsigned char>( data, mask );
  case DataType::SHORT_E:
    return CreateMaskFromNonZeroDataInternal<short>( data, mask );
  case DataType::USHORT_E:
    return CreateMaskFromNonZeroDataInternal<unsigned short>( data, mask );
  case DataType::INT_E:
    return CreateMaskFromNonZeroDataInternal<int>( data, mask );
  case DataType::UINT_E:
    return CreateMaskFromNonZeroDataInternal<unsigned int>( data, mask );
  case DataType::FLOAT_E:
    return CreateMaskFromNonZeroDataInternal<float>( data, mask );
  case DataType::DOUBLE_E:
    return CreateMaskFromNonZeroDataInternal<double>( data, mask );
  default:
    return false;
  }
}

template< class DATA >
static bool CreateMaskFromBitPlaneDataInternal( const DataVolumeHandle& data, 
  std::vector<MaskDataBlockHandle>& masks )
{
  masks.clear();

  DATA* src   = reinterpret_cast<DATA*>( data->data_block()->get_data() ); 
  size_t size = data->get_size();

  DATA used_bits(0);

  for ( size_t j = 0; j < size; j++ )
  {
    used_bits |= src[ j ];
  }

  std::bitset< sizeof( DATA ) > bits( used_bits );

  masks.resize( bits.count() );

  for ( size_t k = 0; k < bits.size(); k++ )
  {
    if ( bits[ k ] )
    {
      MaskDataBlockHandle mask;
      if ( ! ( MaskDataBlockManager::Instance()->create( data->get_grid_transform(), mask ) ) )
      {
        masks.clear();
        return false;
      }

      unsigned char* mask_data  = mask->get_mask_data();
      unsigned char  mask_value = mask->get_mask_value();
      unsigned char  not_mask_value = ~( mask->get_mask_value() );

      DATA test_value( 1 << k );

      for ( size_t j = 0; j < size; j++ )
      {
        if ( src[ j ] & test_value ) mask_data[ j ] |= mask_value;
        else mask_data[ j ] &= not_mask_value;
      }
    }
  }

  return true;
}

bool MaskVolume::CreateMaskFromBitPlaneData( const DataVolumeHandle data, 
  std::vector<MaskDataBlockHandle>& masks )
{
  // Ensure there is no valid pointer left in the handle
  masks.clear();

  // Check if there is any data
  if ( !data ) return false;

  // Lock the source data
  DataBlock::lock_type lock( data->get_mutex( ) );

  switch( data->get_type() )
  {
  case DataType::CHAR_E:
    return CreateMaskFromBitPlaneDataInternal<signed char>( data, masks );
  case DataType::UCHAR_E:
    return CreateMaskFromBitPlaneDataInternal<unsigned char>( data, masks );
  case DataType::SHORT_E:
    return CreateMaskFromBitPlaneDataInternal<short>( data, masks );
  case DataType::USHORT_E:
    return CreateMaskFromBitPlaneDataInternal<unsigned short>( data, masks );
  case DataType::INT_E:
    return CreateMaskFromBitPlaneDataInternal<int>( data, masks );
  case DataType::UINT_E:
    return CreateMaskFromBitPlaneDataInternal<unsigned int>( data, masks );
  default:
    return false;
  }
}

template< class DATA >
static bool CreateMaskFromLabelDataInternal( const DataVolumeHandle& data, 
  std::vector<MaskDataBlockHandle>& masks )
{
  masks.clear();

  DATA* src   = reinterpret_cast<DATA*>( data->data_block()->get_data() ); 
  size_t size = data->get_size();
  DATA label( 0 );
  DATA zero_label( 0 );

  size_t next_label_index = 0;
  while ( next_label_index < size && masks.size() < 32 )
  {
    // find the mask we are looking for
    while ( next_label_index < size )
    {
      if ( src[ next_label_index ] != zero_label ) break;
      next_label_index++;
    }  

    if ( next_label_index < size )
    {
      MaskDataBlockHandle mask;
      if ( ! ( MaskDataBlockManager::Instance()->create( data->get_grid_transform(), mask) ) )
      {
        masks.clear();
        return false;
      }

      unsigned char* mask_data  = mask->get_mask_data();
      unsigned char  mask_value = mask->get_mask_value();
      unsigned char  not_mask_value = ~( mask->get_mask_value() );
      label = src[ next_label_index ];

      // Lock the mask layer as it may contain additional masks that are currently in use
      // Hence we need to have full access
      MaskDataBlock::lock_type mask_lock( mask->get_mutex() );

      for ( size_t j = next_label_index; j < size; j++ )
      {
        if ( src[ j ] == label ) 
        {
          src[ j ] = zero_label;
          mask_data[ j ] |= mask_value;
        }
        else
        {
          mask_data[ j ] &= not_mask_value;
        }
      }

      masks.push_back( mask );    
    }
  }

  return true;
}

bool MaskVolume::CreateMaskFromLabelData( const DataVolumeHandle src_data, 
  std::vector<MaskDataBlockHandle>& masks, bool reuse_data  )
{
  // Ensure there is no valid pointer left in the handle
  masks.clear();

  // Check if there is any data
  if ( !src_data ) return false;


  DataBlockHandle data;
  // Clone the data so we can reuse the data block
  // NOTE: This function is intended for using on a data block whose data can be reused
  if ( !reuse_data ) 
  {
    if ( !( DataBlock::Clone( src_data->data_block(), data ) ) ) return false;
  }
  else
  {
    data = src_data->data_block();
  }

  DataVolumeHandle new_data = DataVolumeHandle( new DataVolume( src_data->get_grid_transform(), data ) );
  
  // Lock the source data
  DataBlock::lock_type lock( data->get_mutex( ) );

  switch( data->get_type() )
  {
  case DataType::CHAR_E:
    return CreateMaskFromLabelDataInternal<signed char>( new_data, masks );
  case DataType::UCHAR_E:
    return CreateMaskFromLabelDataInternal<unsigned char>( new_data, masks );
  case DataType::SHORT_E:
    return CreateMaskFromLabelDataInternal<short>( new_data, masks );
  case DataType::USHORT_E:
    return CreateMaskFromLabelDataInternal<unsigned short>( new_data, masks );
  case DataType::INT_E:
    return CreateMaskFromLabelDataInternal<int>( new_data, masks );
  case DataType::UINT_E:
    return CreateMaskFromLabelDataInternal<unsigned int>( new_data, masks );
  case DataType::FLOAT_E:
    return CreateMaskFromLabelDataInternal<float>( new_data, masks );
  case DataType::DOUBLE_E:
    return CreateMaskFromLabelDataInternal<double>( new_data, masks );
  default:
    return false;
  }
}

} // end namespace Core