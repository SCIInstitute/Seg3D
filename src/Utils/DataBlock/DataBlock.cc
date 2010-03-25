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
      this->histogram_.compute( reinterpret_cast<signed char*>( this->data_ ), size() );
      break;
    case DataType::UCHAR_E:
      this->histogram_.compute( reinterpret_cast<unsigned char*>( this->data_ ), size() );
      break;
    case DataType::SHORT_E:
      this->histogram_.compute( reinterpret_cast<short*>( this->data_ ), size() );
      break;
    case DataType::USHORT_E:
      this->histogram_.compute( reinterpret_cast<unsigned short*>( this->data_ ), size() );
      break;
    case DataType::INT_E:
      this->histogram_.compute( reinterpret_cast<int*>( this->data_ ), size() );
      break;
    case DataType::UINT_E:
      this->histogram_.compute( reinterpret_cast<unsigned int*>( this->data_ ), size() );
      break;
    case DataType::FLOAT_E:
      this->histogram_.compute( reinterpret_cast<float*>( this->data_ ), size() );
      break;
    case DataType::DOUBLE_E:
      this->histogram_.compute( reinterpret_cast<double*>( this->data_ ), size() );
      break;
  }
}

} // end namespace Utils
