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

//Boost Includes
#include <boost/lexical_cast.hpp>

#include <Core/Geometry/GridTransform.h>

namespace Core
{

// By default, assume data is node-centered unless otherwise indicated.  Intended to match 
// unu resample behavior.
const bool GridTransform::DEFAULT_NODE_CENTERED_C = true;
const Vector GridTransform::X_AXIS( 1.0, 0.0, 0.0 );
const Vector GridTransform::Y_AXIS( 0.0, 1.0, 0.0 );
const Vector GridTransform::Z_AXIS( 0.0, 0.0, 1.0 );
const Point GridTransform::DEFAULT_ORIGIN( 0.0, 0.0, 0.0 );

GridTransform::GridTransform() :
  nx_( 0 ), ny_( 0 ), nz_( 0 ), originally_node_centered_( DEFAULT_NODE_CENTERED_C )
{
}

GridTransform::GridTransform( const GridTransform& copy ) :
  Transform( copy.transform() ), nx_( copy.nx_ ), ny_( copy.ny_ ), nz_( copy.nz_ ), 
  originally_node_centered_( copy.originally_node_centered_ )
{
}

GridTransform& GridTransform::operator=( const GridTransform& copy )
{
  this->mat_ = copy.mat_;
  this->nx_ = copy.nx_;
  this->ny_ = copy.ny_;
  this->nz_ = copy.nz_;
  this->originally_node_centered_ = copy.originally_node_centered_;

  return ( *this );
}

GridTransform::GridTransform( size_t nx, size_t ny, size_t nz, const Point& p, const Vector& i,
    const Vector& j, const Vector& k ) :
  nx_( nx ), ny_( ny ), nz_( nz ), originally_node_centered_( DEFAULT_NODE_CENTERED_C )
{
  load_basis( p, i, j, k );
}

GridTransform::GridTransform( size_t nx, size_t ny, size_t nz, const Transform& transform ) :
  Transform( transform ), nx_( nx ), ny_( ny ), nz_( nz ), 
  originally_node_centered_( DEFAULT_NODE_CENTERED_C )
{
}

GridTransform::GridTransform( size_t nx, size_t ny, size_t nz, const Transform& transform, 
  bool node_centered ) :
  Transform( transform ), nx_( nx ), ny_( ny ), nz_( nz ), originally_node_centered_( node_centered )
{
}

GridTransform::GridTransform( size_t nx, size_t ny, size_t nz ) :
  nx_( nx ), ny_( ny ), nz_( nz ), originally_node_centered_( DEFAULT_NODE_CENTERED_C )
{
  load_identity();
}

Transform GridTransform::transform() const
{
  return ( Transform( *this ) );
}

bool GridTransform::operator==( const GridTransform& gt ) const
{
  return ( this->nx_ == gt.nx_ && this->ny_ == gt.ny_ && this->nz_ == gt.nz_ && 
    Transform::operator==( gt ) );
}

bool GridTransform::operator!=( const GridTransform& gt ) const
{
  return ( this->nx_ != gt.nx_ || this->ny_ != gt.ny_ || this->nz_ != gt.nz_ || 
    Transform::operator!=( gt ) );
}

double GridTransform::get_diagonal_length() const
{
  return project( Vector( static_cast<double>( this->nx_ ), static_cast<double>( this->ny_ ), 
     static_cast<double>( this->nz_ ) ) ).length();
}

void GridTransform::AlignToCanonicalCoordinates( const GridTransform& src_transform, 
  std::vector< int >& permutation, GridTransform& dst_transform )
{
  // Step 1. Align the transformation frame to axes
  Vector axes[ 3 ];
  std::vector< Vector > canonical_axes( 3 );
  canonical_axes[ 0 ] = X_AXIS;
  canonical_axes[ 1 ] = Y_AXIS;
  canonical_axes[ 2 ] = Z_AXIS;
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
    double length = axes[ i ].length();
    if ( length == 0.0 ) length = 1.0;
    axes[ i ] = canonical_axes[ index ] * ( Sign( proj_len ) * length );
    canonical_axes.erase( canonical_axes.begin() + index );
  }
  
  Point src_origin = src_transform.project( DEFAULT_ORIGIN );
  
  std::vector< size_t > src_size( 3 );
  src_size[ 0 ] = src_transform.get_nx();
  src_size[ 1 ] = src_transform.get_ny();
  src_size[ 2 ] = src_transform.get_nz();
  Vector spacing;
  std::vector< size_t > dst_size( 3 );
  // NOTE: The origin should remain the same, as the value is already in world coordinates
  Point dst_origin = src_origin;

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
        break;
      }     
    }
  }
  
  for ( int i = 0; i < 3; i++ )
  {
    if ( permutation[ i ] < 0 )
    {
      if ( src_transform.get_originally_node_centered() )
      {
        dst_origin[ i ] = dst_origin[ i ] - 
          static_cast<double> ( dst_size[ i ] ) * spacing[ i ];
      }
      else
      {
        dst_origin[ i ] = dst_origin[ i ] - 
          static_cast<double> ( dst_size[ i ] - 1 ) * spacing[ i ];
      }
    }
  }
  
  dst_transform.load_basis( dst_origin, Vector( spacing[ 0 ], 0.0, 0.0 ),
    Vector( 0.0, spacing[ 1 ], 0.0 ), Vector( 0.0, 0.0, spacing[ 2 ] ) );
  dst_transform.set_nx( dst_size[ 0 ] );
  dst_transform.set_ny( dst_size[ 1 ] );
  dst_transform.set_nz( dst_size[ 2 ] );

  dst_transform.set_originally_node_centered( src_transform.get_originally_node_centered() );
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

std::ostream& operator<<( std::ostream& os, const GridTransform& gt )
{
  os << "grid size: [" << gt.get_nx() << ", " << gt.get_ny() << ", " << gt.get_nz() << "]" << std::endl;
  os << "spacing: [" << gt.spacing_x() << ", " << gt.spacing_y() << ", " << gt.spacing_z() << "]" << std::endl;
  os << "originally node centered: " << gt.get_originally_node_centered() << std::endl;
  os << gt.transform();
  return os;
}

std::string ExportToString( const GridTransform& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.get_nx() ) + ',' + ExportToString(
      value.get_ny() ) + ',' + ExportToString( value.get_nz() ) + ',' + ExportToString(
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
    value.set( &values[ 3 ] );
    return ( true );
  }
  return ( false );
}

} // namespace Core
