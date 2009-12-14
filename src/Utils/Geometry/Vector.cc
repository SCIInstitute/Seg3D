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

// STL includes
#include <sstream>

// Utils includes
#include <Utils/Geometry/Vector.h>

namespace Utils {

bool
Vector::find_orthogonal(Vector& v1, Vector& v2) const
{
  Vector v0(Cross(*this, Vector(1.0,0.0,0.0)));
  if(v0.length2() == 0)
  {
    v0=Cross(*this, Vector(0.0,1.0,0.0));
  }
  v1=Cross(*this, v0);
  if (v1.normalize() == 0.0) return (false);
  v2=Cross(*this, v1);
  if (v2.normalize() == 0.0) return (false);
  return (true);
}

bool
VectorF::find_orthogonal(VectorF& v1, VectorF& v2) const
{
  VectorF v0(Cross(*this, VectorF(1.0f,0.0f,0.0f)));
  if(v0.length2() == 0)
  {
    v0=Cross(*this, VectorF(0.0f,1.0f,0.0f));
  }
  v1=Cross(*this, v0);
  if (v1.normalize() == 0.0f) return (false);
  v2=Cross(*this, v1);
  if (v2.normalize() == 0.0f) return (false);
  return (true);
}

std::ostream& operator<<( std::ostream& os, const Vector& v )
{
  os << '[' << v.x() << ' ' << v.y() << ' ' << v.z() << ']';
  return os;
}

std::ostream& operator<<( std::ostream& os, const VectorF& v )
{
  os << '[' << v.x() << ' ' << v.y() << ' ' << v.z() << ']';
  return os;
}

} // End namespace Utils
