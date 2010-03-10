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

#include <Utils/DataBlock/StdDataBlock.h>

namespace Utils
{

StdDataBlock::StdDataBlock( size_t nx, size_t ny, size_t nz, DataType dtype )
{
  // Set the properties of this datablock
  set_nx( nx );
  set_ny( ny );
  set_nz( nz );
  set_type( dtype );

  // Allocate the memory block through C++'s std library
  switch( type() )
  {
  case DataType::UNKNOWN_E:
    set_data( 0 );
    break;
  case DataType::CHAR_E:
    set_data( reinterpret_cast< void* > ( new char[ size() ] ) );
    break;
  case DataType::UCHAR_E:
    set_data(reinterpret_cast<void*>(new unsigned char[size()])); break;
  case DataType::SHORT_E:
    set_data( reinterpret_cast< void* > ( new short[ size() ] ) );
    break;
  case DataType::USHORT_E:
    set_data(reinterpret_cast<void*>(new unsigned short[size()])); break;
  case DataType::INT_E:
    set_data( reinterpret_cast< void* > ( new int[ size() ] ) );
    break;
  case DataType::UINT_E:
    set_data(reinterpret_cast<void*>(new unsigned int[size()])); break;
  case DataType::FLOAT_E:
    set_data( reinterpret_cast< void* > ( new float[ size() ] ) );
    break;
  case DataType::DOUBLE_E:
    set_data( reinterpret_cast< void* > ( new double[ size() ] ) );
    break;
  }
}

StdDataBlock::~StdDataBlock()
{
  if ( data() )
  {
    switch( type() )
    {
    case DataType::UNKNOWN_E:
      break;
    case DataType::CHAR_E:
      delete[] reinterpret_cast< char* > ( data() );
      break;
    case DataType::UCHAR_E:
      delete[] reinterpret_cast<unsigned char*>(data()); break;
    case DataType::SHORT_E:
      delete[] reinterpret_cast< short* > ( data() );
      break;
    case DataType::USHORT_E:
      delete[] reinterpret_cast<unsigned short*>(data()); break;
    case DataType::INT_E:
      delete[] reinterpret_cast< int* > ( data() );
      break;
    case DataType::UINT_E:
      delete[] reinterpret_cast<unsigned int*>(data()); break;
    case DataType::FLOAT_E:
      delete[] reinterpret_cast< float* > ( data() );
      break;
    case DataType::DOUBLE_E:
      delete[] reinterpret_cast< double* > ( data() );
      break;
    }
  }
}

} // end namespace Utils
