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

#include <Core/Utils/StringUtil.h>
#include <Core/VolumeRenderer/TransferFunctionControlPoint.h>

namespace Core
{

std::string ExportToString( const TransferFunctionControlPointVector& control_points )
{
  size_t num_of_floats = control_points.size() * 2;
  std::vector< float > tmp( num_of_floats );
  memcpy( &tmp[ 0 ], &control_points[ 0 ], sizeof( float ) * num_of_floats );
  return ExportToString( tmp );
}

std::string ExportToString( const TransferFunctionControlPoint& control_point )
{
  std::vector< float > tmp( 2 );
  tmp[ 0 ] = control_point.get_value();
  tmp[ 1 ] = control_point.get_opacity();
  return ExportToString( tmp );
}

bool ImportFromString( const std::string& str, TransferFunctionControlPointVector& control_points )
{
  std::vector< float > tmp;
  if ( !ImportFromString( str, tmp ) )
  {
    return false;
  }

  size_t num_of_floats = tmp.size();
  if ( num_of_floats % 2 != 0 )
  {
    return false;
  }

  size_t num_of_pts = num_of_floats / 2;
  control_points.resize( num_of_pts );
  for ( size_t i = 0; i < num_of_pts; ++i )
  {
    control_points[ i ].set_value( tmp[  i << 1 ] );
    control_points[ i ].set_opacity( tmp[ ( i << 1 ) + 1 ] );
  }

  return true;
}

bool ImportFromString( const std::string& str, TransferFunctionControlPoint& control_point )
{
  std::vector< float > tmp;
  if ( !ImportFromString( str, tmp ) || tmp.size() != 2 )
  {
    return false;
  }

  control_point.set_value( tmp[ 0 ] );
  control_point.set_opacity( tmp[ 1 ] );
  return true;
}

} // end namespace Core
