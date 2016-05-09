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

// Teem includes
#include <teem/nrrd.h>

// Boost includes
#include <boost/algorithm/string.hpp>

// Core includes
#include <Core/Utils/StringUtil.h>
#include <Core/DataBlock/DataType.h>

namespace Core
{

bool ImportFromString( const std::string& data_type_string, DataType& data_type )
{
  std::string lower_data_type = boost::to_lower_copy( data_type_string ); 
  boost::erase_all( lower_data_type , " " );

  if ( lower_data_type == "char" || lower_data_type == "signedchar")
  {
    data_type = DataType::CHAR_E;
    return true;
  }
  else if ( lower_data_type == "unsignedchar")
  {
    data_type = DataType::UCHAR_E;
    return true;
  }
  else if ( lower_data_type == "short" || lower_data_type == "signedshort")
  {
    data_type = DataType::SHORT_E;
    return true;
  }
  else if ( lower_data_type == "unsignedshort")
  {
    data_type = DataType::USHORT_E;
    return true;
  }
  else if ( lower_data_type == "int" || lower_data_type == "signedint")
  {
    data_type = DataType::INT_E;
    return true;
  }
  else if ( lower_data_type == "unsignedint")
  {
    data_type = DataType::UINT_E;
    return true;
  }
  else if ( lower_data_type == "longlong" || lower_data_type == "signedlonglong")
  {
    data_type = DataType::LONGLONG_E;
    return true;
  }
  else if ( lower_data_type == "unsignedlonglong")
  {
    data_type = DataType::ULONGLONG_E;
    return true;
  }
  else if ( lower_data_type == "float" )
  {
    data_type = DataType::FLOAT_E;
    return true;
  }
  else if ( lower_data_type == "double")
  {
    data_type = DataType::DOUBLE_E;
    return true;
  }
  else
  {
    data_type = DataType::UNKNOWN_E;
    return false;
  }
}

std::string ExportToString( DataType data_type )
{
  switch ( data_type )
  {
    case DataType::CHAR_E:
      return "char";
    case DataType::UCHAR_E:
      return "unsigned char";
    case DataType::SHORT_E:
      return "short";
    case DataType::USHORT_E:
      return "unsigned short";
    case DataType::INT_E:
      return "int";
    case DataType::UINT_E:
      return "unsigned int";
    case DataType::LONGLONG_E:
      return "long long";
    case DataType::ULONGLONG_E:
      return "unsigned long long";
    case DataType::FLOAT_E:
      return "float";
    case DataType::DOUBLE_E:
      return "double";
    default:
      return "unknown";
  }
}

int GetNrrdDataType( const DataType& data_type)
{
  switch ( data_type )
  {
    case DataType::CHAR_E:
      return nrrdTypeChar;
    case DataType::UCHAR_E:
      return nrrdTypeUChar;
    case DataType::SHORT_E:
      return nrrdTypeShort;
    case DataType::USHORT_E:
      return nrrdTypeUShort;
    case DataType::INT_E:
      return nrrdTypeInt;
    case DataType::UINT_E:
      return nrrdTypeUInt;
    case DataType::LONGLONG_E:
      return nrrdTypeLLong;
    case DataType::ULONGLONG_E:
      return nrrdTypeULLong;
    case DataType::FLOAT_E:
      return nrrdTypeFloat;
    case DataType::DOUBLE_E:
      return nrrdTypeDouble;
    default:
      return nrrdTypeUnknown;
  }
}


size_t GetSizeDataType( const DataType& data_type)
{
  switch ( data_type )
  {
    case DataType::CHAR_E:
    case DataType::UCHAR_E:
      return sizeof( char );
    case DataType::SHORT_E:
    case DataType::USHORT_E:
      return sizeof( short );
    case DataType::INT_E:
    case DataType::UINT_E:
      return sizeof( int );
    case DataType::LONGLONG_E:
    case DataType::ULONGLONG_E:
      return sizeof( long long );
    case DataType::FLOAT_E:
      return sizeof( float );
    case DataType::DOUBLE_E:
      return sizeof( double );
    default:
      return 0;
  }
}

bool IsInteger( const DataType& data_type )
{
  return ( data_type == DataType::CHAR_E || data_type == DataType::UCHAR_E ||
      data_type == DataType::SHORT_E || data_type == DataType::USHORT_E ||
      data_type == DataType::INT_E || data_type == DataType::UINT_E );
}

bool IsReal( const DataType& data_type )
{
  return ( data_type == DataType::FLOAT_E || data_type == DataType::DOUBLE_E );
}

DataType GetDataType( signed char* )
{
  return DataType::CHAR_E;
}

DataType GetDataType( unsigned char* )
{
  return DataType::UCHAR_E;
}

DataType GetDataType( short* )
{
  return DataType::SHORT_E;
}

DataType GetDataType( unsigned short* )
{
  return DataType::USHORT_E;
}

DataType GetDataType( int* )
{
  return DataType::INT_E;
}

DataType GetDataType( unsigned int* )
{
  return DataType::UINT_E;
}

DataType GetDataType( long long* )
{
  return DataType::LONGLONG_E;
}

DataType GetDataType( unsigned long long* )
{
  return DataType::ULONGLONG_E;
}

DataType GetDataType( float* )
{
  return DataType::FLOAT_E;
}

DataType GetDataType( double* )
{
  return DataType::DOUBLE_E;
}


} // end namespace Core
