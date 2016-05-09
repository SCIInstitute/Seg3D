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

#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/DataBlockManager.h>

namespace Core
{

StdDataBlock::StdDataBlock( size_t nx, size_t ny, size_t nz, DataType dtype )
{
  // Set the properties of this datablock
  set_nx( nx );
  set_ny( ny );
  set_nz( nz );
  set_type( dtype );

  // Allocate the memory block through C++'s std library
  switch( get_data_type() )
  {
  case DataType::UNKNOWN_E:
    set_nx( 0 );
    set_ny( 0 );
    set_nz( 0 );
    set_data( 0 );
    break;
  case DataType::CHAR_E:
    set_data( reinterpret_cast< void* > ( new char[ get_size() ] ) );
    break;
  case DataType::UCHAR_E:
    set_data( reinterpret_cast<void*>( new unsigned char[ get_size() ] ) ); 
    break;
  case DataType::SHORT_E:
    set_data( reinterpret_cast< void* > ( new short[ get_size() ] ) );
    break;
  case DataType::USHORT_E:
    set_data(reinterpret_cast<void*>( new unsigned short[ get_size() ] ) ); 
    break;
  case DataType::INT_E:
    set_data( reinterpret_cast< void* > ( new int[ get_size() ] ) );
    break;
  case DataType::UINT_E:
    set_data( reinterpret_cast<void*>( new unsigned int[ get_size() ] ) ); 
    break;
  case DataType::FLOAT_E:
    set_data( reinterpret_cast< void* > ( new float[ get_size() ] ) );
    break;
  case DataType::DOUBLE_E:
    set_data( reinterpret_cast< void* > ( new double[ get_size() ] ) );
    break;
  }
}

StdDataBlock::~StdDataBlock()
{
  if ( get_data() )
  {
    switch( get_data_type() )
    {
    case DataType::UNKNOWN_E:
      break;
    case DataType::CHAR_E:
      delete[] reinterpret_cast< char* > ( get_data() );
      break;
    case DataType::UCHAR_E:
      delete[] reinterpret_cast<unsigned char*>( get_data() ); 
      break;
    case DataType::SHORT_E:
      delete[] reinterpret_cast< short* > ( get_data() );
      break;
    case DataType::USHORT_E:
      delete[] reinterpret_cast<unsigned short*>( get_data() ); 
      break;
    case DataType::INT_E:
      delete[] reinterpret_cast< int* > ( get_data() );
      break;
    case DataType::UINT_E:
      delete[] reinterpret_cast<unsigned int*>( get_data() ); 
      break;
    case DataType::FLOAT_E:
      delete[] reinterpret_cast< float* > ( get_data() );
      break;
    case DataType::DOUBLE_E:
      delete[] reinterpret_cast< double* > ( get_data() );
      break;
    }
  }
}

DataBlockHandle StdDataBlock::New( size_t nx, size_t ny, size_t nz, DataType type )
{
  try
  {
    DataBlockHandle data_block( new StdDataBlock( nx, ny, nz, type ) );
    return data_block;
  }
  catch ( ... )
  {
    // Return an empty handle
    DataBlockHandle data_block;
    return data_block;
  }
}

DataBlockHandle StdDataBlock::New( GridTransform transform, DataType type )
{
  try
  {
    DataBlockHandle data_block( new StdDataBlock( transform.get_nx(), 
      transform.get_ny(), transform.get_nz(), type ) );
    return data_block;
  }
  catch ( ... )
  {
    // Return an empty handle
    DataBlockHandle data_block;
    return data_block;
  }
}

} // end namespace Core
