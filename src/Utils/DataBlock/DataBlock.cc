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

#include <Utils/DataBlock/DataBlock.h>
#include <Utils/DataBlock/StdDataBlock.h>

namespace Utils
{

DataBlock::DataBlock() :
  nx_( 0 ), 
  ny_( 0 ), 
  nz_( 0 ), 
  data_type_( DataType::UNKNOWN_E ), 
  data_( 0 )
{
}

DataBlock::~DataBlock()
{
}

double DataBlock::get_data_at( size_t index ) const
{
  switch( this->data_type_ )
  {
  case DataType::CHAR_E:
    {
      signed char* data = reinterpret_cast<signed char*>( this->data_ );
      return static_cast<double>( data[ index ] );
    }     
  case DataType::UCHAR_E:
    {
      unsigned char* data = reinterpret_cast<unsigned char*>( this->data_ );
      return static_cast<double>( data[ index ] );
    }     
  case DataType::SHORT_E:
    {
      short* data = reinterpret_cast<short*>( this->data_ );
      return static_cast<double>( data[ index ] );
    }     
  case DataType::USHORT_E:
    {
      unsigned short* data = reinterpret_cast<unsigned short*>( this->data_ );
      return static_cast<double>( data[ index ] );
    }
  case DataType::INT_E:
    {
      int* data = reinterpret_cast<int*>( this->data_ );
      return static_cast<double>( data[ index ] );
    }     
  case DataType::UINT_E:
    {
      unsigned int* data = reinterpret_cast<unsigned int*>( this->data_ );
      return static_cast<double>( data[ index ] );
    }       
  case DataType::FLOAT_E:
    {
      float* data = reinterpret_cast<float*>( this->data_ );
      return static_cast<double>( data[ index ] );
    }     
  case DataType::DOUBLE_E:
    {
      double* data = reinterpret_cast<double*>( this->data_ );
      return data[ index ];
    }     
  }
  
  return 0.0;
}

void DataBlock::set_data_at( size_t index, double value )
{
  switch( this->data_type_ )
  {
    case DataType::CHAR_E:
    {
      signed char* data = reinterpret_cast<signed char*>( this->data_ );
      data[ index ] = static_cast<signed char>( value );
      return;
    }     
    case DataType::UCHAR_E:
    {
      unsigned char* data = reinterpret_cast<unsigned char*>( this->data_ );
      data[ index ] = static_cast<unsigned char>( value );
      return;
    }     
    case DataType::SHORT_E:
    {
      short* data = reinterpret_cast<short*>( this->data_ );
      data[ index ] = static_cast<short>( value );
      return;
    }     
    case DataType::USHORT_E:
    {
      unsigned short* data = reinterpret_cast<unsigned short*>( this->data_ );
      data[ index ] = static_cast<unsigned short>( value );
      return;
    } 
    case DataType::INT_E:
    {
      int* data = reinterpret_cast<int*>( this->data_ );
      data[ index ] = static_cast<int>( value );
      return;
    }     
    case DataType::UINT_E:
    {
      unsigned int* data = reinterpret_cast<unsigned int*>( this->data_ );
      data[ index ] = static_cast<unsigned int>( value );
      return;
    } 
    case DataType::FLOAT_E:
    {
      float* data = reinterpret_cast<float*>( this->data_ );
      data[ index ] = static_cast<float>( value );
      return;
    }     
    case DataType::DOUBLE_E:
    {
      double* data = reinterpret_cast<double*>( this->data_ );
      data[ index ] = value;
      return;
    }
  }
}

void DataBlock::set_type( DataType type )
{
  this->data_type_ = type;
}

void DataBlock::update_histogram()
{
  lock_type lock( get_mutex() );

  switch( this->data_type_ )
  {
    case DataType::CHAR_E:
      this->histogram_.compute( reinterpret_cast<signed char*>( get_data() ), get_size() );
      break;
    case DataType::UCHAR_E:
      this->histogram_.compute( reinterpret_cast<unsigned char*>( get_data() ), get_size() );
      break;
    case DataType::SHORT_E:
      this->histogram_.compute( reinterpret_cast<short*>( get_data() ), get_size() );
      break;
    case DataType::USHORT_E:
      this->histogram_.compute( reinterpret_cast<unsigned short*>( get_data() ), get_size() );
      break;
    case DataType::INT_E:
      this->histogram_.compute( reinterpret_cast<int*>( get_data() ), get_size() );
      break;
    case DataType::UINT_E:
      this->histogram_.compute( reinterpret_cast<unsigned int*>( get_data() ), get_size() );
      break;
    case DataType::FLOAT_E:
      this->histogram_.compute( reinterpret_cast<float*>( get_data() ), get_size() );
      break;
    case DataType::DOUBLE_E:
      this->histogram_.compute( reinterpret_cast<double*>( get_data() ), get_size() );
      break;
  }
}


template<class DATA>
bool ConvertDataTypeInternal( DATA* src, DataBlockHandle& dst_data_block )
{
  size_t size = dst_data_block->get_size();
  switch ( dst_data_block->get_type() )
  {
    case DataType::CHAR_E:  
    {
      signed char* dst = reinterpret_cast<signed char*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<signed char>( src[ j ] );
      return true;
    }
    case DataType::UCHAR_E: 
    {
      unsigned char* dst = reinterpret_cast<unsigned char*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<unsigned char>( src[ j ] );
      return true;
    }
    case DataType::SHORT_E: 
    {
      short* dst = reinterpret_cast<short*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<short>( src[ j ] );
      return true;
    }
    case DataType::USHORT_E:  
    {
      unsigned short* dst = reinterpret_cast<unsigned short*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<unsigned short>( src[ j ] );
      return true;
    }
    case DataType::INT_E: 
    {
      int* dst = reinterpret_cast<int*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<int>( src[ j ] );
      return true;
    }
    case DataType::UINT_E:  
    {
      unsigned int* dst = reinterpret_cast<unsigned int*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<unsigned int>( src[ j ] );
      return true;
    }
    case DataType::FLOAT_E: 
    {
      float* dst = reinterpret_cast<float*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<float>( src[ j ] );
      return true;
    }
    case DataType::DOUBLE_E:  
    {
      double* dst = reinterpret_cast<double*>( dst_data_block->get_data() );
      for ( size_t j = 0 ; j < size; j++ ) dst[ j ] = static_cast<double>( src[ j ] );
      return true;
    }
    default:
    {
      dst_data_block.reset();
      return false;
    }
  }
}


bool DataBlock::ConvertDataType( const DataBlockHandle& src_data_block, 
  DataBlockHandle& dst_data_block, DataType new_data_type )
{
  lock_type lock( src_data_block->get_mutex( ) );

  dst_data_block = DataBlockHandle( new StdDataBlock( src_data_block->get_nx(),
    src_data_block->get_ny(), src_data_block->get_nz(), new_data_type ) );
    
  if ( !dst_data_block )
  {
    return false;
  }
  
  switch( src_data_block->get_type() )
  {
    case DataType::CHAR_E:
      return ConvertDataTypeInternal<signed char>( 
        reinterpret_cast<signed char*>( src_data_block->get_data() ), dst_data_block );
    case DataType::UCHAR_E:
      return ConvertDataTypeInternal<unsigned char>( 
        reinterpret_cast<unsigned char*>( src_data_block->get_data() ), dst_data_block );
    case DataType::SHORT_E:
      return ConvertDataTypeInternal<short>( 
        reinterpret_cast<short*>( src_data_block->get_data() ), dst_data_block );
    case DataType::USHORT_E:
      return ConvertDataTypeInternal<unsigned short>( 
        reinterpret_cast<unsigned short*>( src_data_block->get_data() ), dst_data_block );
    case DataType::INT_E:
      return ConvertDataTypeInternal<int>( 
        reinterpret_cast<int*>( src_data_block->get_data() ), dst_data_block );
    case DataType::UINT_E:
      return ConvertDataTypeInternal<unsigned int>( 
        reinterpret_cast<unsigned int*>( src_data_block->get_data() ), dst_data_block );
    case DataType::FLOAT_E:
      return ConvertDataTypeInternal<float>( 
        reinterpret_cast<float*>( src_data_block->get_data() ), dst_data_block );
    case DataType::DOUBLE_E:
      return ConvertDataTypeInternal<double>( 
        reinterpret_cast<double*>( src_data_block->get_data() ), dst_data_block );
    default:
      dst_data_block.reset();
      return false;
  }
}

template<class DATA>
bool PermuteDataBlockInternal( const DataBlockHandle& src_data_block, 
  DataBlockHandle& dst_data_block, std::vector<int>& permutation )
{
  DATA* src = reinterpret_cast<DATA*>( src_data_block->get_data() );
  DATA* dst = reinterpret_cast<DATA*>( dst_data_block->get_data() );

  typedef DataBlock::index_type index_type;

  std::vector<index_type> start(3);
  std::vector<index_type> stride(3);
  
  index_type nx = static_cast<index_type>( src_data_block->get_nx() );
  index_type ny = static_cast<index_type>( src_data_block->get_ny() );
  index_type nz = static_cast<index_type>( src_data_block->get_nz() );
  index_type nxy = nx*ny;
  index_type nxyz = nx*ny*nz;
  
  for ( index_type j = 0; j < 3; j++)
  {
    if ( permutation[ j ] == 1 )
    {
      start[ j ] = 0;
      stride[ j ] = 1; 
    }
    else if ( permutation[ j ] == -1 )
    {
      start[ j ] = nx - 1;
      stride[ j ] = -1;   
    }
    else if ( permutation[ j ] == 2 )
    {
      start[ j ] = 0;
      stride[ j ] = nx; 
    }
    else if ( permutation[ j ] == -2 )
    {
      start[ j ] = nxy - nx;
      stride[ j ] = -nx;  
    }
    else if ( permutation[ j ] == 3 )
    {
      start[ j ] = 0;
      stride[ j ] = nxy; 
    }
    else if ( permutation[ j ] == -3 )
    {
      start[ j ] = nxyz - nxy;
      stride[ j ] = -nxy;   
    }
  }
  
  index_type dz = start[ 2 ];
  index_type dz_stride = stride[ 2 ];
  for ( index_type sz = 0 ; sz < nz ; sz += nxy )
  {
    index_type dy = start[ 1 ];
    index_type dy_stride = stride[ 1 ];

    for ( index_type sy = 0 ; sy < ny ; sy += nx )
    {
      index_type dx = start[ 0 ];
      index_type dx_stride = stride[ 0 ];
      for ( index_type sx = 0 ; sx < nx ; sx++ )
      {
        dst [ dx + dy + dz ] = src [ sx + sy + sz ]; 
        dx += dx_stride;
      }
      dy += dy_stride;
    }
    dz += dz_stride;
  }
  
  return true;
}


bool DataBlock::PermuteDataBlock( const DataBlockHandle& src_data_block, 
  DataBlockHandle& dst_data_block, std::vector<int> permutation )
{
  lock_type lock( src_data_block->get_mutex( ) );

  if ( permutation.size() != 3 )
  {
    dst_data_block.reset();
    return false;
  }
  
  size_t dn[3];
  dn[ 0 ] = 0;
  dn[ 1 ] = 0;
  dn[ 2 ] = 0;
  
  for ( size_t j = 0; j < 3; j++)
  {
    if ( permutation[ j ] == 1 || permutation[ j ] == -1 ) dn[ j ] = src_data_block->get_nx();
    if ( permutation[ j ] == 2 || permutation[ j ] == -2 ) dn[ j ] = src_data_block->get_ny();
    if ( permutation[ j ] == 3 || permutation[ j ] == -3 ) dn[ j ] = src_data_block->get_nz();    
  }
  
  if ( dn[ 0 ] * dn[ 1 ] * dn[ 2 ] == 0 ) return false;
  
  dst_data_block = DataBlockHandle( new StdDataBlock( dn[ 0 ], dn[ 1 ], dn[ 2 ], 
    src_data_block->get_type() ) ); 

  switch( src_data_block->get_type() )
  {
    case DataType::CHAR_E:
      return PermuteDataBlockInternal<signed char>( src_data_block, dst_data_block, 
        permutation );
    case DataType::UCHAR_E:
      return PermuteDataBlockInternal<unsigned char>( src_data_block, dst_data_block, 
        permutation );
    case DataType::SHORT_E:
      return PermuteDataBlockInternal<short>( src_data_block, dst_data_block, 
        permutation );
    case DataType::USHORT_E:
      return PermuteDataBlockInternal<unsigned short>( src_data_block, dst_data_block, 
        permutation );
    case DataType::INT_E:
      return PermuteDataBlockInternal<int>( src_data_block, dst_data_block, 
        permutation );
    case DataType::UINT_E:
      return PermuteDataBlockInternal<unsigned int>( src_data_block, dst_data_block, 
        permutation );
    case DataType::FLOAT_E:
      return PermuteDataBlockInternal<float>( src_data_block, dst_data_block, 
        permutation );
    case DataType::DOUBLE_E:
      return PermuteDataBlockInternal<double>( src_data_block, dst_data_block, 
        permutation );
    default:
      dst_data_block.reset();
      return false;
  }
}

} // end namespace Utils
