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

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#define _HAS_CPP0X 0
#endif

#include <bitset>

#include <Utils/DataBlock/MaskDataBlockManager.h>
#include <Utils/DataBlock/StdDataBlock.h>


namespace Utils
{

// CLASS MaskDataBlockEntry
// Helper class with information of each data block that is used for masked layers

class MaskDataBlockEntry
{
public:
  MaskDataBlockEntry( DataBlockHandle data_block ) :
    data_block_( data_block ), data_masks_( 8 )
  {
  }

  // The datablock that holds the masks
  DataBlockHandle data_block_;

  // Accounting which bits are used
  std::bitset< 8 > bits_used_;

  // Pointers to th MaskDataBlocks that represent these bitplanes
  std::vector< MaskDataBlockWeakHandle > data_masks_;
};

// CLASS MaskDataBlockManagerInternal
// The internals of the manager class.

class MaskDataBlockManagerInternal : public boost::noncopyable
{
  // -- internals of the MaskDataBlockManager --
public:
  // List that maintains a list of which bits are used in
  typedef std::vector< MaskDataBlockEntry > mask_list_type;
  mask_list_type mask_list_;

  // Mutex that protects the list
  MaskDataBlockManager::mutex_type mutex_;  
};



MaskDataBlockManager::MaskDataBlockManager() :
  private_( new MaskDataBlockManagerInternal )
{
}

MaskDataBlockManager::~MaskDataBlockManager()
{
}

bool MaskDataBlockManager::create( size_t nx, size_t ny, size_t nz, MaskDataBlockHandle& mask )
{
  lock_type lock( get_mutex() );

  DataBlockHandle data_block;
  unsigned int mask_bit = 0;
  size_t mask_entry_index = 0;

  MaskDataBlockManagerInternal::mask_list_type& mask_list = private_->mask_list_;

  for (size_t j=0; j<mask_list.size(); j++)
  {
    // Find an empty location
    if ( ( nx == mask_list[ j ].data_block_->get_nx() ) &&
      ( ny == mask_list[ j ].data_block_->get_ny() ) &&
      ( nz == mask_list[ j ].data_block_->get_nz() ) &&
      ( mask_list[ j ].bits_used_.count() != 8 ) )
    {
      data_block = mask_list[ j ].data_block_;
      mask_entry_index = j;

      for ( size_t k = 0; k < 8; k++) 
      {
        if ( !( mask_list[j].bits_used_.test( k ) ) )
        {
          mask_bit = static_cast<unsigned int>( k );
          // drop out of for loop
          break;
        }
      }
      // drop out of for loop
      break;
    }
  }

  if ( !( data_block.get() ) )
  {
    // Could not find empty position, so create a new data block
    data_block = DataBlockHandle( new StdDataBlock( nx, ny, nz, DataType::UCHAR_E ) );
    mask_bit = 0;
    mask_entry_index = mask_list.size();
    mask_list.push_back( MaskDataBlockEntry( data_block ) );
  }

  // Generate the new mask
  mask = MaskDataBlockHandle( new MaskDataBlock( data_block, mask_bit ) );
  // Clear the mask before using it
  // TODO: we might want to put this logic in the constructor of MaskVolume
  size_t data_size = nx * ny * nz;
  for ( size_t i = 0; i< data_size; i++ )
  {
    mask->clear_mask_at( i );
  }

  // Mark the bitplane as being used before returning the mask
  mask_list[ mask_entry_index ].bits_used_[ mask_bit ] = 1;
  mask_list[ mask_entry_index ].data_masks_[ mask_bit ] = mask;

  return true;
}

void MaskDataBlockManager::release(DataBlockHandle& datablock, unsigned int mask_bit)
{
  lock_type lock( get_mutex() );

  MaskDataBlockManagerInternal::mask_list_type& mask_list = private_->mask_list_;

  // Remove the MaskDataBlock from the list
  for ( size_t j = 0 ; j < mask_list.size() ; j++ )
  {
    if ( mask_list[ j ].data_block_ == datablock )
    {
      mask_list[ j ].bits_used_[mask_bit] = 0;
      mask_list[ j ].data_masks_[mask_bit].reset();

      // If the DataBlock is not used any more clear it
      if (mask_list[ j ].bits_used_.count() == 0)
      {
        mask_list.erase( mask_list.begin() + j);
      }

      break;
    }
  }
}

MaskDataBlockManager::mutex_type& MaskDataBlockManager::get_mutex()
{
  return private_->mutex_;
}

bool MaskDataBlockManager::compact()
{
  // TODO: Need to implement this
  return false;
}

template< class DATA >
static bool CreateMaskFromNonZeroDataInternal( const DataBlockHandle& data, 
  MaskDataBlockHandle& mask )
{
  DATA* src   = reinterpret_cast<DATA*>( data->get_data() ); 
  size_t size = mask->get_size();

  unsigned char* mask_data  = mask->get_mask_data();
  unsigned char  mask_value = mask->get_mask_value();

  for ( size_t j = 0; j < size; j++ )
  {
    if ( src[ j ] ) mask_data[ j ] |= mask_value;
  }
  
  return true;
}

bool MaskDataBlockManager::CreateMaskFromNonZeroData( const DataBlockHandle data, 
  MaskDataBlockHandle& mask )
{
  // Ensure there is no valid pointer left in the handle
  mask.reset();
  
  // Check if there is any data
  if ( !data ) return false;
  
  // Lock the source data
  DataBlock::lock_type lock( data->get_mutex( ) );

  // Create a new mask data block
  if ( !( MaskDataBlockManager::Instance()->create( data->get_nx(), data->get_ny(), 
    data->get_nz(), mask ) ) )
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
static bool CreateMaskFromBitPlaneDataInternal( const DataBlockHandle& data, 
  std::vector<MaskDataBlockHandle>& masks )
{
  masks.clear();
  
  DATA* src   = reinterpret_cast<DATA*>( data->get_data() ); 
  size_t size = data->get_size();

  DATA used_bits(0);
  
  for ( size_t j = 0; j < size; j++ )
  {
    used_bits |= src[ j ];
  }

  std::bitset< sizeof( DATA ) > bits( used_bits );
  
  masks.resize( bits.count() );
  for ( size_t j = 0; j < bits.count(); j++ )
  {
  }
  
  for ( size_t k = 0; k < bits.size(); k++ )
  {
    if ( bits[ k ] )
    {
      MaskDataBlockHandle mask;
      if ( ! ( MaskDataBlockManager::Instance()->create( data->get_nx(), data->get_ny(), 
        data->get_nz(), mask ) ) )
      {
        masks.clear();
        return false;
      }

      unsigned char* mask_data  = mask->get_mask_data();
      unsigned char  mask_value = mask->get_mask_value();
      
      DATA test_value( 1 << k );
      
      for ( size_t j = 0; j < size; j++ )
      {
        if ( src[ j ] & test_value ) mask_data[ j ] |= mask_value;
      }
    }
  }

  return true;
}

bool MaskDataBlockManager::CreateMaskFromBitPlaneData( const DataBlockHandle data, 
    std::vector<MaskDataBlockHandle>& masks  )
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
static bool CreateMaskFromLabelDataInternal( const DataBlockHandle& data, 
  std::vector<MaskDataBlockHandle>& masks )
{
  masks.clear();
  
  DATA* src   = reinterpret_cast<DATA*>( data->get_data() ); 
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
      if ( ! ( MaskDataBlockManager::Instance()->create( data->get_nx(), data->get_ny(), 
        data->get_nz(), mask) ) )
      {
        masks.clear();
        return false;
      }

      unsigned char* mask_data  = mask->get_mask_data();
      unsigned char  mask_value = mask->get_mask_value();
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
      }
      
      masks.push_back( mask );    
    }
  }

  return true;
}

bool MaskDataBlockManager::CreateMaskFromLabelData( const DataBlockHandle src_data, 
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
    if ( !( DataBlock::Clone( src_data, data ) ) ) return false;
  }
  else
  {
    data = src_data;
  }

  // Lock the source data
  DataBlock::lock_type lock( data->get_mutex( ) );

  switch( data->get_type() )
  {
    case DataType::CHAR_E:
      return CreateMaskFromLabelDataInternal<signed char>( data, masks );
    case DataType::UCHAR_E:
      return CreateMaskFromLabelDataInternal<unsigned char>( data, masks );
    case DataType::SHORT_E:
      return CreateMaskFromLabelDataInternal<short>( data, masks );
    case DataType::USHORT_E:
      return CreateMaskFromLabelDataInternal<unsigned short>( data, masks );
    case DataType::INT_E:
      return CreateMaskFromLabelDataInternal<int>( data, masks );
    case DataType::UINT_E:
      return CreateMaskFromLabelDataInternal<unsigned int>( data, masks );
    case DataType::FLOAT_E:
      return CreateMaskFromLabelDataInternal<float>( data, masks );
    case DataType::DOUBLE_E:
      return CreateMaskFromLabelDataInternal<double>( data, masks );
    default:
      return false;
  }
}

} // end namespace Utils
