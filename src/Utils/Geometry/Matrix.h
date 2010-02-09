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

#ifndef UTILS_GEOMETRY_MATRIX_H
#define UTILS_GEOMETRY_MATRIX_H

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>

#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>

namespace Utils {

class Matrix : 
  public boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major> {
  
  typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major> base_type;
    
  public:
    Matrix() : base_type(4, 4) {}
    
    Matrix(const base_type& m) : base_type(m) 
    {
      assert(m.size1() == 4 && m.size2() == 4);
    }
    
    ~Matrix() {}
    
    inline Matrix& operator=(const base_type& m)
    {
      assert(m.size1() == 4 && m.size2() == 4);
      assign(m);
      return (*this);
    }
    
    Vector operator*(const Vector& rhs) const;
    VectorF operator*(const VectorF& rhs) const;
    Point operator*(const Point& rhs) const;
    PointF operator*(const PointF& rhs) const;
    
    
  private:
    
  public:
    // Identity matrix
    const static Matrix IDENTITY_C;
    
    // Zero matrix
    const static Matrix ZERO_C;
    
    // Threshold value of determinant for classifying a matrix as singular
    const static double EPSILON_C;
};


// Compute the inverse of the input matrix using LU decomposition
bool Invert(const Matrix& m, Matrix& inverse);

} // End namespace Utils

#endif