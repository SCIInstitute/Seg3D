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

#ifndef CORE_DATABLOCK_DATATYPE_H
#define CORE_DATABLOCK_DATATYPE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

// Core includes
#include <Core/Utils/EnumClass.h>

namespace Core
{

// CLASS DataType:
/// This class describes the data types that the program uses

CORE_ENUM_CLASS
(
  DataType,
  CHAR_E = 0, 
  UCHAR_E, 
  SHORT_E, 
  USHORT_E, 
  INT_E, 
  UINT_E,
  LONGLONG_E,
  ULONGLONG_E, 
  FLOAT_E, 
  DOUBLE_E, 
  UNKNOWN_E
)

// IMPORTFROMSTRING:
/// Import a DataType from a string
/// NOTE: If the import fails UNKNOWN_E is returned and the function returns false
bool ImportFromString( const std::string& data_type_string, DataType& data_type );

// EXPORTTOSTRING:
/// Export the data type to a string
std::string ExportToString( DataType data_type );

// ISINTEGER:
/// Test whether data type is an integer 
bool IsInteger( const DataType& data_type );

// ISREAL:
/// Test whether data is floating point
bool IsReal( const DataType& data_type );

// GETNRRDDATATYPE:
/// Get the type of the data
int GetNrrdDataType( const DataType& data_type );

// GETSIZEDATATYPE:
/// Get the number of bytes in this data type
size_t GetSizeDataType( const DataType& data_type );

// GETDATATYPE:
/// Get the data type from a pointer to the type
DataType GetDataType( signed char* );
DataType GetDataType( unsigned char* );
DataType GetDataType( short* );
DataType GetDataType( unsigned short* );
DataType GetDataType( int* );
DataType GetDataType( unsigned int* );
DataType GetDataType( long long* );
DataType GetDataType( unsigned long long* );
DataType GetDataType( float* );
DataType GetDataType( double* );


} // end namespace Core

#endif
