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

#ifndef CORE_GEOMETRY_GRIDTRANSFORM_H
#define CORE_GEOMETRY_GRIDTRANSFORM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <ostream>

// Core includes
#include <Core/Geometry/Matrix.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Plane.h>
#include <Core/Geometry/Transform.h>

#include <Core/Math/MathFunctions.h>

namespace Core
{

class GridTransform : public Transform
{
  // -- constuctors --
public:
  GridTransform();

  GridTransform( const GridTransform& );

  GridTransform( size_t nx, size_t ny, size_t nz, const Point&, const Vector&, const Vector&,
      const Vector& );

  GridTransform( size_t nx, size_t ny, size_t nz, const Transform& transform );

  GridTransform( size_t nx, size_t ny, size_t nz, const Transform& transform, bool node_centered );

  GridTransform( size_t nx, size_t ny, size_t nz );

  GridTransform& operator=( const GridTransform& copy );

  // -- comparison --
public:
  bool operator==( const GridTransform& ) const;
  bool operator!=( const GridTransform& ) const;

  // -- accessors --
public:
  // GET_NX, GET_NY, GET_NZ, GET_SIZE
  // Get the dimensions of the grid this class describes
  size_t get_nx() const
  {
    return nx_;
  }
  size_t get_ny() const
  {
    return ny_;
  }
  size_t get_nz() const
  {
    return nz_;
  }

  // SPACING_X, SPACING_Y, SPACING_Z
  // The spacing in each of directions
  double spacing_x() const
  {
    return project( Vector( 1.0, 0.0, 0.0 ) ).length();
  }

  double spacing_y() const
  {
    return project( Vector( 0.0, 1.0, 0.0 ) ).length();
  }

  double spacing_z() const
  {
    return project( Vector( 0.0, 0.0, 1.0 ) ).length();
  }

  // SET_NX, SET_NY, SET_NY
  // Set the size of the grid
  void set_nx( size_t nx )
  {
    nx_ = nx;
  }
  void set_ny( size_t ny )
  {
    ny_ = ny;
  }
  void set_nz( size_t nz )
  {
    nz_ = nz;
  }
  
  // TRANSFORM:
  // Get the underlying transform
  Transform transform() const;

  // This is needed when this value needs to be copied from one grid transform to another
  void set_originally_node_centered( bool originally_node_centered )
  {
    this->originally_node_centered_ = originally_node_centered;
  }

  bool get_originally_node_centered() const
  {
    return this->originally_node_centered_;
  }

  // -- internal representation --
private:

  // The dimensions of the grid
  size_t nx_;
  size_t ny_;
  size_t nz_;

  // Centering
  bool originally_node_centered_;
  static const bool DEFAULT_NODE_CENTERED_C;

public:
  static void AlignToCanonicalCoordinates( const GridTransform& src_transform,
    std::vector< int >& permutation, GridTransform& dst_transform );
};

Point operator*( const GridTransform& gt, const Point& d );
Vector operator*( const GridTransform& gt, const Vector& d );
PointF operator*( const GridTransform& gt, const PointF& d );
VectorF operator*( const GridTransform& gt, const VectorF& d );

std::string ExportToString( const GridTransform& value );
bool ImportFromString( const std::string& str, GridTransform& value );

} // End namespace Core

#endif
