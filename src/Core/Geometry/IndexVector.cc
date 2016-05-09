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

// STL includes
#include <sstream>
#include <vector>

// Core includes
#include <Core/Utils/StringUtil.h>
#include <Core/Geometry/IndexVector.h>

namespace Core
{

std::ostream& operator<<( std::ostream& os, const IndexVector& idx )
{
  os << '[' << idx.x() << ' ' << idx.y() << ' ' << idx.z() << ']';
  return os;
}

std::string ExportToString( const IndexVector& idx )
{
  return ( std::string(1, '[') + ExportToString( idx.x()) + ','
    + ExportToString( idx.y() ) + ','
    + ExportToString( idx.z( ) ) + ']' );
}

bool ImportFromString( const std::string& str, IndexVector& idx )
{
  std::vector< IndexVector::index_type > values;
  ImportFromString( str, values );
  if ( values.size() == 3 )
  {
    idx = IndexVector( values[ 0 ], values[ 1 ], values[ 2 ] );
    return true;
  }
  return false;
}

} // End namespace Core
