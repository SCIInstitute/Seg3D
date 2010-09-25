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

//Boost Includes
#include <boost/lexical_cast.hpp>

#include <Core/Geometry/GridTransform.h>

namespace Core
{

GridTransform::GridTransform() :
  nx_( 0 ), ny_( 0 ), nz_( 0 )
{
}

GridTransform::GridTransform( const GridTransform& copy ) :
  Transform( copy.transform() ), nx_( copy.nx_ ), ny_( copy.ny_ ), nz_( copy.nz_ )
{
}

GridTransform& GridTransform::operator=( const GridTransform& copy )
{
  mat_ = copy.mat_;
  nx_ = copy.nx_;
  ny_ = copy.ny_;
  nz_ = copy.nz_;

  return ( *this );
}

GridTransform::GridTransform( size_t nx, size_t ny, size_t nz, const Point& p, const Vector& i,
    const Vector& j, const Vector& k ) :
  nx_( nx ), ny_( ny ), nz_( nz )
{
  load_basis( p, i, j, k );
}

GridTransform::GridTransform( size_t nx, size_t ny, size_t nz, const Transform& transform ) :
  Transform( transform ), nx_( nx ), ny_( ny ), nz_( nz )
{
}

GridTransform::GridTransform( size_t nx, size_t ny, size_t nz ) :
  nx_( nx ), ny_( ny ), nz_( nz )
{
  load_identity();
}

Transform GridTransform::transform() const
{
  return ( Transform( *this ) );
}

bool GridTransform::operator==( const GridTransform& gt ) const
{
  return ( nx_ == gt.nx_ && ny_ == gt.ny_ && nz_ == gt.nz_ && mat_ == gt.mat_ );
}

bool GridTransform::operator!=( const GridTransform& gt ) const
{
  return ( nx_ != gt.nx_ || ny_ != gt.ny_ || nz_ != gt.nz_ || mat_ != gt.mat_ );
}

void GridTransform::AlignToCanonicalCoordinates( const GridTransform& src_transform, 
                        std::vector< int >& permutation, GridTransform& dst_transform )
{
  // Step 1. Align the transformation frame to axes
  Vector axes[ 3 ];
  std::vector< Vector > canonical_axes( 3 );
  canonical_axes[ 0 ] = Vector( 1.0, 0.0, 0.0 );
  canonical_axes[ 1 ] = Vector( 0.0, 1.0, 0.0 );
  canonical_axes[ 2 ] = Vector( 0.0, 0.0, 1.0 );
  axes[ 0 ] = src_transform.project( canonical_axes[ 0 ] );
  axes[ 1 ] = src_transform.project( canonical_axes[ 1 ] );
  axes[ 2 ] = src_transform.project( canonical_axes[ 2 ] );

  // Find the closest axis to each vector
  for ( int i = 0; i < 3; ++i )
  {
    double proj_len = 0;
    size_t index = 0;
    for ( size_t j = 0; j < canonical_axes.size(); ++j )
    {
      double dot_prod = Dot( axes[ i ], canonical_axes[ j ] );
      if ( Abs( dot_prod ) > Abs( proj_len ) )
      {
        index = j;
        proj_len = dot_prod;
      }
    }
    axes[ i ] = canonical_axes[ index ] * ( Sign( proj_len ) * axes[ i ].length() );
    canonical_axes.erase( canonical_axes.begin() + index );
  }
  
  Point src_origin = src_transform.project( Point( 0, 0, 0 ) );
  std::vector< size_t > src_size( 3 );
  src_size[ 0 ] = src_transform.get_nx();
  src_size[ 1 ] = src_transform.get_ny();
  src_size[ 2 ] = src_transform.get_nz();
  Vector spacing;
  std::vector< size_t > dst_size( 3 );
  Point dst_origin;
  // Step 2. Get the permutation transformation from the source to canonical coordinates
  permutation.resize( 3 );
  for ( int i = 0; i < 3; ++i )
  {
    for ( int j = 0; j < 3; ++j )
    {
      if ( axes[ j ][ i ] != 0 )
      {
        permutation[ i ] = Sign( axes[ j ][ i ] ) * ( j + 1 );
        spacing[ i ] = Abs( axes[ j ][ i ] );
        dst_size[ i ] = src_size[ j ];
        if ( axes[ i ][ j ] > 0 )
        {
          dst_origin[ i ] = src_origin[ j ];
        }
        else
        {
          dst_origin[ i ] = src_origin[ j ] + axes[ j ][ i ] * 
            ( static_cast< int >( src_size[ j ] ) - 1 );
        }
        break;
      }     
    }
  }
  
  dst_transform.load_basis( dst_origin, Vector( spacing[ 0 ], 0.0, 0.0 ),
    Vector( 0.0, spacing[ 1 ], 0.0 ), Vector( 0.0, 0.0, spacing[ 2 ] ) );
  dst_transform.set_nx( dst_size[ 0 ] );
  dst_transform.set_ny( dst_size[ 1 ] );
  dst_transform.set_nz( dst_size[ 2 ] );
}

Point operator*( const GridTransform& gt, const Point& d )
{
  return gt.project( d );
}

Vector operator*( const GridTransform& gt, const Vector& d )
{
  return gt.project( d );
}

PointF operator*( const GridTransform& gt, const PointF& d )
{
  return gt.project( d );
}

VectorF operator*( const GridTransform& gt, const VectorF& d )
{
  return gt.project( d );
}

std::string ExportToString( const GridTransform& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.get_nx() ) + ' ' + ExportToString(
      value.get_ny() ) + ' ' + ExportToString( value.get_nz() ) + ' ' + ExportToString(
      value.transform() ) + ']' );
}

bool ImportFromString( const std::string& str, GridTransform& value )
{
  std::vector< double > values;
  ImportFromString( str, values );
  if ( values.size() == 19 )
  {
    value.set_nx( static_cast< size_t > ( values[ 0 ] ) );
    value.set_ny( static_cast< size_t > ( values[ 1 ] ) );
    value.set_nz( static_cast< size_t > ( values[ 2 ] ) );
    value.transform().set( &values[ 3 ] );
    return ( true );
  }
  return ( false );
}

} // namespace Core
