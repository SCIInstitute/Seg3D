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

// STL includes
#include <bitset>

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/NrrdData.h>

// Application includes
#include <Application/Layer/MaskLayer.h>

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( MaskDataBlockManager );

// CLASS MaskDataBlockEntry
// Helper class with information of each data block that is used for masked layers

class MaskDataBlockEntry
{
public:
  MaskDataBlockEntry( DataBlockHandle data_block, GridTransform grid_transform ) :
    data_block_( data_block ), data_masks_( 8 ), grid_transform_( grid_transform )
  {
  }

  // The datablock that holds the masks
  DataBlockHandle data_block_;

  // Accounting which bits are used
  std::bitset< 8 > bits_used_;

  // Pointers to th MaskDataBlocks that represent these bitplanes
  std::vector< MaskDataBlockWeakHandle > data_masks_;

  GridTransform grid_transform_;

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
};



MaskDataBlockManager::MaskDataBlockManager() :
  private_( new MaskDataBlockManagerInternal )
{
}

MaskDataBlockManager::~MaskDataBlockManager()
{
}

bool MaskDataBlockManager::create( GridTransform grid_transform, MaskDataBlockHandle& mask )
{
  lock_type lock( get_mutex() );

  DataBlockHandle data_block;
  unsigned int mask_bit = 0;
  size_t mask_entry_index = 0;

  MaskDataBlockManagerInternal::mask_list_type& mask_list = private_->mask_list_;

  for (size_t j=0; j<mask_list.size(); j++)
  {
    // Find an empty location
    if ( ( mask_list[ j ].bits_used_.count() != 8 ) &&
      ( grid_transform == mask_list[ j ].grid_transform_ ) )
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
    data_block = StdDataBlock::New( grid_transform.get_nx(), grid_transform.get_ny(), 
      grid_transform.get_nz(), DataType::UCHAR_E );
    mask_bit = 0;
    mask_entry_index = mask_list.size();
    mask_list.push_back( MaskDataBlockEntry( data_block, grid_transform ) );
  }

  // Generate the new mask
  mask = MaskDataBlockHandle( new MaskDataBlock( data_block, mask_bit ) );
  // Clear the mask before using it
  // TODO: we might want to put this logic in the constructor of MaskVolume
  
  size_t data_size = grid_transform.get_nx() * grid_transform.get_ny() * grid_transform.get_nz();
  unsigned char* data = mask->get_mask_data();
  unsigned char not_mask_value = ~( mask->get_mask_value() );
  
  size_t data_size8 = data_size & ~(0x7);
  size_t i = 0;
  for ( ; i< data_size8; i+=8 )
  {
    data[ i ] &= not_mask_value;
    data[ i+1 ] &= not_mask_value;
    data[ i+2 ] &= not_mask_value;
    data[ i+3 ] &= not_mask_value;
    data[ i+4 ] &= not_mask_value;
    data[ i+5 ] &= not_mask_value;
    data[ i+6 ] &= not_mask_value;
    data[ i+7 ] &= not_mask_value;
  }
  for ( ; i< data_size; i++ )
  {
    data[ i ] &= not_mask_value;
  }

  // Mark the bitplane as being used before returning the mask
  mask_list[ mask_entry_index ].bits_used_[ mask_bit ] = 1;
  mask_list[ mask_entry_index ].data_masks_[ mask_bit ] = mask;

  return true;
}

bool MaskDataBlockManager::create( DataBlock::generation_type generation, unsigned int bit, 
                  GridTransform& grid_transform, MaskDataBlockHandle& mask )
{
  lock_type lock( this->get_mutex() );

  DataBlockHandle data_block;

  MaskDataBlockManagerInternal::mask_list_type& mask_list = private_->mask_list_;

  for ( size_t j=0; j < mask_list.size(); j++ )
  {
    if ( mask_list[ j ].data_block_->get_generation() == generation )
    {
      assert( mask_list[ j ].bits_used_[ bit ] == 0 );
      grid_transform = mask_list[ j ].grid_transform_;
      mask = MaskDataBlockHandle( new MaskDataBlock( mask_list[ j ].data_block_, bit ) );
      mask_list[ j ].bits_used_[ bit ] = 1;
      mask_list[ j ].data_masks_[ bit ] = mask;

      return true;
    }
  }

  return false;
}


void MaskDataBlockManager::release(DataBlockHandle& datablock, unsigned int mask_bit)
{
  lock_type lock( get_mutex() );

  MaskDataBlockManagerInternal::mask_list_type& mask_list = this->private_->mask_list_;

  // Remove the MaskDataBlock from the list
  for ( size_t j = 0 ; j < mask_list.size() ; j++ )
  {
    if ( mask_list[ j ].data_block_ == datablock )
    {
      mask_list[ j ].bits_used_[mask_bit] = 0;
      mask_list[ j ].data_masks_[mask_bit].reset();

      // If the DataBlock is not used any more clear it
      if ( mask_list[ j ].bits_used_.count() == 0 )
      {
        mask_list.erase( mask_list.begin() + j);
      }

      break;
    }
  }
}

bool MaskDataBlockManager::compact()
{
  // TODO: Need to implement this
  return false;
}

void MaskDataBlockManager::register_data_block( DataBlockHandle data_block, 
                         const GridTransform& grid_transform )
{
  this->private_->mask_list_.push_back( MaskDataBlockEntry( data_block, grid_transform ) );
}

bool MaskDataBlockManager::save_data_blocks( boost::filesystem::path data_save_path )
{
  lock_type lock( get_mutex() );

  MaskDataBlockManagerInternal::mask_list_type& mask_list = private_->mask_list_;

  for ( size_t j = 0 ; j < mask_list.size() ; j++ )
  {
    boost::filesystem::path volume_path = data_save_path / 
      ( Core::ExportToString( mask_list[ j ].data_block_->
      get_generation() ) + ".nrrd" );

    if( !boost::filesystem::exists( volume_path ) )
    {
      NrrdDataHandle nrrd = NrrdDataHandle( new NrrdData( 
        mask_list[ j ].data_block_, mask_list[ j ].grid_transform_ ) );

      std::string error;

      if ( ! ( NrrdData::SaveNrrd( volume_path.string(), nrrd, error ) ) ) 
      {
        CORE_LOG_ERROR( error );
        return false;
      }
    }
  }

  return true;
}

bool MaskDataBlockManager::Create( GridTransform grid_transform, MaskDataBlockHandle& mask )
{
  return MaskDataBlockManager::Instance()->create( grid_transform, mask );
}

template< class T >
bool ConvertToMaskInternal( DataBlockHandle data, MaskDataBlockHandle& mask, bool invert )
{
  T* data_ptr = reinterpret_cast<T*>( data->get_data() );
  
  unsigned char* mask_ptr = mask->get_mask_data();
  unsigned char mask_value;
  unsigned char not_mask_value;

  if ( invert )
  {
    not_mask_value = mask->get_mask_value();
    mask_value = ~( mask->get_mask_value() );
  }
  else
  {
    mask_value = mask->get_mask_value();
    not_mask_value = ~( mask->get_mask_value() );
  }
  
  size_t size = data->get_size();
  size_t size8 = size & ~(0x7);
  for ( size_t j = 0; j < size8; j+= 8 )
  {
    if ( data_ptr[ j ] ) mask_ptr[ j ] |= mask_value; else mask_ptr[ j ] &= not_mask_value;
    if ( data_ptr[ j + 1 ] ) mask_ptr[ j + 1 ] |= mask_value; else mask_ptr[ j + 1 ] &= not_mask_value;
    if ( data_ptr[ j + 2 ] ) mask_ptr[ j + 2 ] |= mask_value; else mask_ptr[ j + 2 ] &= not_mask_value;
    if ( data_ptr[ j + 3 ] ) mask_ptr[ j + 3 ] |= mask_value; else mask_ptr[ j + 3 ] &= not_mask_value;
    if ( data_ptr[ j + 4 ] ) mask_ptr[ j + 4 ] |= mask_value; else mask_ptr[ j + 4 ] &= not_mask_value;
    if ( data_ptr[ j + 5 ] ) mask_ptr[ j + 5 ] |= mask_value; else mask_ptr[ j + 5 ] &= not_mask_value;
    if ( data_ptr[ j + 6 ] ) mask_ptr[ j + 6 ] |= mask_value; else mask_ptr[ j + 6 ] &= not_mask_value;
    if ( data_ptr[ j + 7 ] ) mask_ptr[ j + 7 ] |= mask_value; else mask_ptr[ j + 7 ] &= not_mask_value;
  }
  for ( size_t j = size8; j < size; j++ )
  {
    if ( data_ptr[ j ] ) mask_ptr[ j ] |= mask_value; else mask_ptr[ j ] &= not_mask_value; 
  }
  
  return true;
}

bool MaskDataBlockManager::Convert( DataBlockHandle data, 
  GridTransform grid_transform, MaskDataBlockHandle& mask, bool invert )
{
  if ( !( MaskDataBlockManager::Instance()->create( grid_transform, mask ) ) )
  {
    return false;
  }

  assert( mask->get_nx() == data->get_nx() );
  assert( mask->get_ny() == data->get_ny() );
  assert( mask->get_nz() == data->get_nz() );
  
  DataBlock::shared_lock_type lock( data->get_mutex( ) );
  DataBlock::lock_type mask_lock( mask->get_mutex( ) );
  
  switch( data->get_data_type() )
  {
    case DataType::CHAR_E:
      return ConvertToMaskInternal<signed char>( data, mask, invert );
    case DataType::UCHAR_E:
      return ConvertToMaskInternal<unsigned char>( data, mask, invert );
    case DataType::SHORT_E:
      return ConvertToMaskInternal<short>( data, mask, invert );
    case DataType::USHORT_E:
      return ConvertToMaskInternal<unsigned short>( data, mask, invert );
    case DataType::INT_E:
      return ConvertToMaskInternal<int>( data, mask, invert );
    case DataType::UINT_E:
      return ConvertToMaskInternal<unsigned int>( data, mask, invert );
    case DataType::FLOAT_E:
      return ConvertToMaskInternal<float>( data, mask, invert );
    case DataType::DOUBLE_E:
      return ConvertToMaskInternal<double>( data, mask, invert );
  }

  return false;
}

template< class T >
bool ConvertLabelToMaskInternal( DataBlockHandle data, MaskDataBlockHandle& mask, double label )
{
  T typed_label = static_cast<T>( label );
  T* data_ptr = reinterpret_cast<T*>( data->get_data() );
  
  unsigned char* mask_ptr = mask->get_mask_data();
  unsigned char mask_value = mask->get_mask_value();
  unsigned char not_mask_value = ~( mask->get_mask_value() );
  
  size_t size = data->get_size();
  size_t size8 = size & ~(0x7);
  for ( size_t j = 0; j < size8; j+= 8 )
  {
    if ( data_ptr[ j ] == typed_label ) mask_ptr[ j ] |= mask_value; 
      else mask_ptr[ j ] &= not_mask_value;
    if ( data_ptr[ j + 1 ] == typed_label ) mask_ptr[ j + 1 ] |= mask_value;
      else mask_ptr[ j + 1 ] &= not_mask_value;
    if ( data_ptr[ j + 2 ] == typed_label ) mask_ptr[ j + 2 ] |= mask_value; 
      else mask_ptr[ j + 2 ] &= not_mask_value;
    if ( data_ptr[ j + 3 ] == typed_label ) mask_ptr[ j + 3 ] |= mask_value; 
      else mask_ptr[ j + 3 ] &= not_mask_value;
    if ( data_ptr[ j + 4 ] == typed_label ) mask_ptr[ j + 4 ] |= mask_value; 
      else mask_ptr[ j + 4 ] &= not_mask_value;
    if ( data_ptr[ j + 5 ] == typed_label ) mask_ptr[ j + 5 ] |= mask_value; 
      else mask_ptr[ j + 5 ] &= not_mask_value;
    if ( data_ptr[ j + 6 ] == typed_label ) mask_ptr[ j + 6 ] |= mask_value; 
      else mask_ptr[ j + 6 ] &= not_mask_value;
    if ( data_ptr[ j + 7 ] == typed_label ) mask_ptr[ j + 7 ] |= mask_value; 
      else mask_ptr[ j + 7 ] &= not_mask_value;
  }
  for ( size_t j = size8; j < size; j++ )
  {
    if ( data_ptr[ j ] == typed_label ) mask_ptr[ j ] |= mask_value; 
      else mask_ptr[ j ] &= not_mask_value; 
  }
  
  return true;
}



bool MaskDataBlockManager::ConvertLabel( DataBlockHandle data, 
  GridTransform grid_transform, MaskDataBlockHandle& mask, double label )
{
  if ( !( MaskDataBlockManager::Instance()->create( grid_transform, mask ) ) )
  {
    return false;
  }

  assert( mask->get_nx() == data->get_nx() );
  assert( mask->get_ny() == data->get_ny() );
  assert( mask->get_nz() == data->get_nz() );
  
  DataBlock::shared_lock_type lock( data->get_mutex( ) );
  DataBlock::lock_type mask_lock( mask->get_mutex( ) );
  
  switch( data->get_data_type() )
  {
    case DataType::CHAR_E:
      return ConvertLabelToMaskInternal<signed char>( data, mask, label );
    case DataType::UCHAR_E:
      return ConvertLabelToMaskInternal<unsigned char>( data, mask, label );
    case DataType::SHORT_E:
      return ConvertLabelToMaskInternal<short>( data, mask, label );
    case DataType::USHORT_E:
      return ConvertLabelToMaskInternal<unsigned short>( data, mask, label );
    case DataType::INT_E:
      return ConvertLabelToMaskInternal<int>( data, mask, label );
    case DataType::UINT_E:
      return ConvertLabelToMaskInternal<unsigned int>( data, mask, label );
    case DataType::FLOAT_E:
      return ConvertLabelToMaskInternal<float>( data, mask, label );
    case DataType::DOUBLE_E:
      return ConvertLabelToMaskInternal<double>( data, mask, label );
  }

  return false;
}


template< class T>
bool ConvertToDataInternal( MaskDataBlockHandle mask, DataBlockHandle& data, bool invert )
{
  MaskDataBlock::shared_lock_type lock( mask->get_mutex( ) );

  unsigned char* mask_ptr = mask->get_mask_data();
  unsigned char mask_value = mask->get_mask_value();
  
  data = StdDataBlock::New( mask->get_nx(), mask->get_ny(), mask->get_nz(), 
    GetDataType( reinterpret_cast< T* >( 0 ) ) );
  T* data_ptr = reinterpret_cast< T* >( data->get_data() );
  
  const T on = static_cast<T>( invert?0:1 );
  const T off = static_cast<T>( invert?1:0 );
  size_t size = data->get_size();
  size_t size8 = size & ~(0x7);

  for ( size_t j = 0; j < size8; j+= 8 )
  {
    if ( mask_ptr[ j ] & mask_value ) data_ptr[ j ] = on; else data_ptr[ j ] = off;
    if ( mask_ptr[ j + 1 ] & mask_value ) data_ptr[ j + 1 ] = on; else data_ptr[ j + 1 ] = off;
    if ( mask_ptr[ j + 2 ] & mask_value ) data_ptr[ j + 2 ] = on; else data_ptr[ j + 2 ] = off;
    if ( mask_ptr[ j + 3 ] & mask_value ) data_ptr[ j + 3 ] = on; else data_ptr[ j + 3 ] = off;
    if ( mask_ptr[ j + 4 ] & mask_value ) data_ptr[ j + 4 ] = on; else data_ptr[ j + 4 ] = off;
    if ( mask_ptr[ j + 5 ] & mask_value ) data_ptr[ j + 5 ] = on; else data_ptr[ j + 5 ] = off;
    if ( mask_ptr[ j + 6 ] & mask_value ) data_ptr[ j + 6 ] = on; else data_ptr[ j + 6 ] = off;
    if ( mask_ptr[ j + 7 ] & mask_value ) data_ptr[ j + 7 ] = on; else data_ptr[ j + 7 ] = off;
  }
  for ( size_t j = size8; j < size; j++ )
  {
    if ( mask_ptr[ j ] & mask_value ) data_ptr[ j ] = on; else data_ptr[ j ] = off;
  }

  return true;
}

bool MaskDataBlockManager::Convert( MaskDataBlockHandle mask, DataBlockHandle& data,
  DataType data_type, bool invert )
{
  switch( data_type )
  {
    case DataType::CHAR_E:
      return ConvertToDataInternal<signed char>( mask, data, invert );
    case DataType::UCHAR_E:
      return ConvertToDataInternal<unsigned char>( mask, data, invert );
    case DataType::SHORT_E:
      return ConvertToDataInternal<short>( mask, data, invert );
    case DataType::USHORT_E:
      return ConvertToDataInternal<unsigned short>( mask, data, invert );
    case DataType::INT_E:
      return ConvertToDataInternal<int>( mask, data, invert );
    case DataType::UINT_E:
      return ConvertToDataInternal<unsigned int>( mask, data, invert );
    case DataType::FLOAT_E:
      return ConvertToDataInternal<float>( mask, data, invert );
    case DataType::DOUBLE_E:
      return ConvertToDataInternal<double>( mask, data, invert );
  } 
  return false;
}



} // end namespace Core
