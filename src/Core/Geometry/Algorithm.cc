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

#include <iostream>

#include <Core/Geometry/Algorithm.h>

namespace Core 
{

double DistanceToLine2( const Point& p, const Point& a, const Point& b, 
  const double epsilon )
{
  Vector m = b - a;
  Vector n = p - a;
  if ( m.length2() < epsilon ) 
  {
    return n.length2();
  }
  else 
  {
    const double t0 = Dot( m, n ) / Dot( m, m );
    if ( t0 <= 0 ) return n.length2();
    else if ( t0 >= 1.0 ) return ( p - b ).length2();
    else return ( n - m * t0 ).length2();
  }
}

void DistanceToLine2Aux( Point& result, const Point& p, const Point& a, 
            const Point& b, const double epsilon )
{
  Vector m = b - a;
  Vector n = p - a;
  if ( m.length2() < epsilon ) 
  {
    result = a;
  }
  else
  {
    const double t0 = Dot( m, n ) / Dot( m, m );
    if ( t0 <= 0 ) 
    {
      result = a;
    }
    else if ( t0 >= 1.0 )
    {
      result = b;
    }
    else
    {
      Vector offset = m * t0;
      result = a + offset;
    }
  }
}

void DistanceToLine2Aux( Point& result, int& node, const Point& p, 
            const Point& a, const Point& b, const double epsilon )
{
  node = -1;
  Vector m = b - a;
  Vector n = p - a;
  if ( m.length2() < epsilon ) 
  {
    node = 0;
    result = a;
  }
  else
  {
    const double t0 = Dot( m, n ) / Dot( m, m );
    if ( t0 <= 0 ) 
    {
      node = 0;
      result = a;
    }
    else if ( t0 >= 1.0 )
    {
      node = 1;
      result = b;
    }
    else
    {
      Vector offset = m * t0;
      result = a + offset;
    }
  }
}

template< class T >
T SolveMatrix3x3( const T* p, const T* q, T* r ) 
{
  const T a=p[0], b=p[1], c=p[2];
  const T d=p[3], e=p[4], f=p[5];
  const T g=p[6], h=p[7], i=p[8];

  const T detp=a*e*i-c*e*g+b*f*g+c*d*h-a*f*h-b*d*i;
  const T detinvp=(detp ? 1.0/detp : 0.0);

  r[0] = detinvp * ( (e*i-f*h)*q[0] + (c*h-b*i)*q[1] + (b*f-c*e)*q[2] );
  r[1] = detinvp * ( (f*g-d*i)*q[0] + (a*i-c*g)*q[1] + (c*d-a*f)*q[2] );
  r[2] = detinvp * ( (d*h-e*g)*q[0] + (b*g-a*h)*q[1] + (a*e-b*d)*q[2] ); 

  return detp;
}

bool RayTriangleIntersection( Point& result, const Point& orig, const Vector& dir,
  const Point& p0, const Point& p1, const Point& p2 )
{
  double mat[ 9 ];
  double b[ 3 ];
  double x[ 3 ];

  mat[ 0 ] = p1.x() - p0.x();
  mat[ 3 ] = p1.y() - p0.y();
  mat[ 6 ] = p1.z() - p0.z();

  mat[ 1 ] = p2.x() - p0.x();
  mat[ 4 ] = p2.y() - p0.y();
  mat[ 7 ] = p2.z() - p0.z();

  mat[ 2 ] = - dir.x();
  mat[ 5 ] = - dir.y();
  mat[ 8 ] = - dir.z();

  b[ 0 ] = orig.x() - p0.x();
  b[ 1 ] = orig.y() - p0.y();
  b[ 2 ] = orig.z() - p0.z();

  if ( SolveMatrix3x3<double>( mat, b, x ) )
  {
    if ( x[ 0 ] < -0.001 ) return false;
    if ( x[ 1 ] < -0.001 ) return false;
    if ( x[ 0 ] + x[ 1 ] > 1.001 ) return false;
    if ( x[ 2 ] < 0.0 ) return false;

    result = orig + x[ 2 ]*dir;
    return true;
  }

  return false;
}

void ClosestPointOnTriangle( Point& result, const Point& orig, const Point& p0, 
              const Point& p1, const Point& p2, const double epsilon )
{
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;

  const Vector dir = Cross(edge1, edge2);

  const Vector pvec = Cross(dir, edge2);

  const double inv_det = 1.0 / Dot(edge1, pvec);

  const Vector tvec = orig - p0;
  double u = Dot(tvec, pvec) * inv_det;

  const Vector qvec = Cross(tvec, edge1);
  double v = Dot(dir, qvec) * inv_det;

  if (u < 0.0)
  {
    DistanceToLine2Aux(result, orig, p0, p2,epsilon);
  }
  else if (v < 0.0)
  {
    DistanceToLine2Aux(result, orig, p0, p1,epsilon);
  }
  else if (u + v > 1.0)
  {
    DistanceToLine2Aux(result, orig, p1, p2,epsilon);
  }
  else
  {
    result = p0 + u * edge1 + v * edge2;
  }
}


void ClosestPointOnTriangle( Point& result, int& edge, int& node, const Point &orig,
              const Point &p0, const Point &p1, const Point &p2, const double epsilon)
{
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;

  const Vector dir = Cross(edge1, edge2);

  const Vector pvec = Cross(dir, edge2);

  const double inv_det = 1.0 / Dot(edge1, pvec);

  const Vector tvec = orig - p0;
  double u = Dot(tvec, pvec) * inv_det;

  const Vector qvec = Cross(tvec, edge1);
  double v = Dot(dir, qvec) * inv_det;

  if (u < 0.0)
  {
    DistanceToLine2Aux(result, node, orig, p0, p2,epsilon);
    if (node == 1) node = 2;
    edge = 2;
  }
  else if (v < 0.0)
  {
    DistanceToLine2Aux(result, node, orig, p0, p1,epsilon);
    edge = 0;
  }
  else if (u + v > 1.0)
  {
    DistanceToLine2Aux(result, node, orig, p1, p2,epsilon);
    node = node + 1;
    edge = 1;
  }
  else
  {
    result = p0 + u * edge1 + v * edge2;
    edge = -1;
    node = -1;
  }
}


void EstimateClosestPointOnQuad( Point &result, const Point &orig, 
                const Point &p0, const Point &p1, 
                const Point &p2,const Point &p3, 
                const double epsilon )
{
  // This implementation is only an approximation
  Point r1,r2;
  ClosestPointOnTriangle(r1,orig,p0,p1,p3,epsilon);
  ClosestPointOnTriangle(r2,orig,p3,p1,p2,epsilon);

  Point r3,r4;
  ClosestPointOnTriangle(r3,orig,p0,p2,p3,epsilon);
  ClosestPointOnTriangle(r4,orig,p0,p1,p2,epsilon);

  if ((r2-orig).length2() < (r1-orig).length2()) r1 = r2;
  if ((r4-orig).length2() < (r3-orig).length2()) r3 = r4;

  r1 += r3;
  result = 0.5*r1;
}


double RayPlaneIntersection( const Point &p,  const Vector &dir, 
              const Point &p0, const Vector &pn, const double epsilon )
{
  // Compute divisor.
  const double Vd = Dot(dir, pn);

  // Return no intersection if parallel to plane or no cross product.
  if (Vd < epsilon)
    return 1.0e30*epsilon;

  const double D = - Dot(pn, p0);

  const double V0 = - (Dot(pn, p) + D);

  return V0 / Vd;
}

bool RayTriangleIntersection( double &t, double &u, double &v, bool backface_cull,
               const Point &orig,  const Vector &dir, const Point &p0, 
               const Point &p1, const Point &p2, const double epsilon )
{
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;
  const double vepsilon = epsilon*epsilon*epsilon;

  const Vector pvec = Cross(dir, edge2);

  const double det = Dot(edge1, pvec);

  if (det < vepsilon && (backface_cull || det > -vepsilon))
    return false;

  const double inv_det = 1.0 / det;

  const Vector tvec = orig - p0;

  u = Dot(tvec, pvec) * inv_det;
  if (u < 0.0 || u > 1.0)
    return false;

  const Vector qvec = Cross(tvec, edge1);

  v = Dot(dir, qvec) * inv_det;
  if (v < 0.0 || u+v > 1.0)
    return false;

  t = Dot(edge2, qvec) * inv_det;

  return true;
}


bool ClosestLineToLine( double &s, double &t, const Point &a0, const Point &a1,
             const Point &b0, const Point &b1, const double epsilon )
{
  const Vector u = a1 - a0;
  const Vector v = b1 - b0;
  const Vector w = a0 - b0;

  const double a = Dot(u, u);
  const double b = Dot(u, v);
  const double c = Dot(v, v);
  const double d = Dot(u, w);
  const double e = Dot(v, w);
  const double D = a*c - b*b;

  if (D < epsilon*epsilon)
  {
    s = 0.0;
    t = (b>c?d/b:e/c);
    return false;
  }
  else
  {
    s = (b*e - c*d) / D;
    t = (a*e - b*d) / D;
    return true;
  }
}

double TetrahedraVolume( const Point &p0, const Point &p1, 
            const Point &p2, const Point &p3 )
{
  return fabs(Dot(Cross(p1-p0,p2-p0),p3-p0)) / 6.0;
}

void TriangleTriangleIntersection( const Point &A0, const Point &A1, const Point &A2,
                  const Point &B0, const Point &B1, const Point &B2,
                  std::vector<Point> &results )
{
  double t, u, v;
  if (RayTriangleIntersection(t, u, v, false, A0, A1-A0, B0, B1, B2) &&
    t >= 0.0 && t <= 1.0)
  {
    results.push_back(A0 + (A1-A0) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, A1, A2-A1, B0, B1, B2) &&
    t >= 0.0 && t <= 1.0)
  {
    results.push_back(A1 + (A2-A1) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, A2, A0-A2, B0, B1, B2) &&
    t >= 0.0 && t <= 1.0)
  {
    results.push_back(A2 + (A0-A2) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, B0, B1-B0, A0, A1, A2) &&
    t >= 0.0 && t <= 1.0)
  {
    results.push_back(B0 + (B1-B0) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, B1, B2-B1, A0, A1, A2) &&
    t >= 0.0 && t <= 1.0)
  {
    results.push_back(B1 + (B2-B1) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, B2, B0-B2, A0, A1, A2) &&
    t >= 0.0 && t <= 1.0)
  {
    results.push_back(B2 + (B0-B2) * t);
  }
}

} // end namespace Core
