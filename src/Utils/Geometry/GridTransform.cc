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

#include <Utils/Geometry/GridTransform.h>

namespace Utils
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
  
  std::string GridTransform::get_as_string()
  {
    std::string string_representation;
    string_representation = boost::lexical_cast<std::string>( nx_ )+ "x" + 
      boost::lexical_cast<std::string>( ny_ ) + "x" +
    boost::lexical_cast<std::string>( nz_ );
    return string_representation;
  }
  
  

bool GridTransform::operator==( const GridTransform& gt ) const
{
  return ( nx_ == gt.nx_ && ny_ == gt.ny_ && nz_ == gt.nz_ && mat_ == gt.mat_ );
}

bool GridTransform::operator!=( const GridTransform& gt ) const
{
  return ( nx_ != gt.nx_ || ny_ != gt.ny_ || nz_ != gt.nz_ || mat_ != gt.mat_ );
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

} // namespace Utils
