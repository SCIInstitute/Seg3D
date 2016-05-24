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

#ifndef CORE_GEOMETRY_ALGORITHM_H
#define CORE_GEOMETRY_ALGORITHM_H

#include <vector>

#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>

namespace Core 
{

// DISTANCETOLINE2:
/// Compute the distance squared from the point to the given line,
/// where the line is specified by two end points.  This function
/// actually computes the distance to the line segment
/// between the given points and not to the line itself.
double DistanceToLine2( const Point& p, const Point& a, const Point& b, 
             const double epsilon = 1e-12 );

void DistanceToLine2Aux( Point& result, const Point& p, const Point& a, 
            const Point &b, const double epsilon = 1e-12 );

void DistanceToLine2Aux( Point& result, int& node, const Point& p, 
            const Point& a, const Point& b, const double epsilon = 1e-12 );

// CLOSESTPOINTONTRIANGLE:
/// Compute the point on the triangle closest to the given point.
/// The distance to the triangle will be (P - result).length())
void ClosestPointOnTriangle( Point& result, const Point& p, const Point& a,
              const Point& b, const Point& c, const double epsilon = 1e-12 );

void ClosestPointOnTriangle( Point& result, int& edge, int& node, const Point& p,
              const Point& a, const Point& b, const Point& c, const double epsilon = 1e-12 );


// ESTIMATECLOSESTPOINTONQUAD:
/// This is only an estimate for an element that is not flat, it is exact
/// for a flat element.
void EstimateClosestPointOnQuad( Point& result, const Point& p, const Point &a,
                 const Point& b, const Point& c, const Point& d, const double epsilon = 1e-12 );

// CLOSESTLINETOLINE:
/// Compute s and t such that the distance between a0 + s * (a1 - aO)
/// and b0 + t * (b1 - b0) is minimal.  Return false if the lines are
/// parallel, true otherwise.
bool ClosestLineToLine( double& s, double& t, const Point& a0, const Point& a1,
             const Point& b0, const Point& b1, const double epsilon = 1e-12 );

bool RayTriangleIntersection( Point& result, const Point& orig, const Vector& dir,
               const Point& p0, const Point& p1, const Point& p2 );

bool RayTriangleIntersection( double& t, double& u, double& v, bool backface_cull,
               const Point& orig, const Vector& dir, const Point& p0, 
               const Point& p1, const Point& p2, const double epsilon = 1e-6 );

double RayPlaneIntersection( const Point& p, const Vector& dir, const Point& p0, 
              const Vector& pn, const double epsilon = 1e-6 );


void TriangleTriangleIntersection( const Point& a0, const Point& a1, const Point& a2,
                  const Point& b0, const Point& b1, const Point& b2,
                  std::vector<Point>& results );

double TetrahedraVolume( const Point& p0, const Point& p1, const Point& p2, 
            const Point& p3 );

} // end namespace Core

#endif
