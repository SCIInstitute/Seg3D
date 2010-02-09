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

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <Utils/Geometry/Matrix.h>
#include <Utils/Math/MathFunctions.h>

namespace Utils {

namespace ublas = boost::numeric::ublas;

const Matrix Matrix::IDENTITY_C = boost::numeric::ublas::identity_matrix<double>(4);
const Matrix Matrix::ZERO_C = boost::numeric::ublas::zero_matrix<double>(4,4);
const double Matrix::EPSILON_C = 1e-7;

Vector 
Matrix::operator *(const Vector &rhs) const
{
  ublas::vector<double> v(4);
  v[0] = rhs[0];
  v[1] = rhs[1];
  v[2] = rhs[2];
  v[3] = 1.0;
  
  ublas::vector<double> result(4);
  result = ublas::prec_prod(*this, v);
  
  return Vector(result[0]/result[3], result[1]/result[3], result[2]/result[3]);
}

VectorF 
Matrix::operator *(const VectorF &rhs) const
{  
  ublas::vector<double> v(4);
  v[0] = rhs[0];
  v[1] = rhs[1];
  v[2] = rhs[2];
  v[3] = 1.0;

  ublas::vector<double> result(4);
  result = ublas::prod(*this, v);

  return VectorF(static_cast<float>(result[0]/result[3]), 
                          static_cast<float>(result[1]/result[3]), 
                          static_cast<float>(result[2]/result[3]));
}

Point 
Matrix::operator*(const Point& rhs) const
{
  ublas::vector<double> v(4);
  v[0] = rhs[0];
  v[1] = rhs[1];
  v[2] = rhs[2];
  v[3] = 1.0;

  ublas::vector<double> result(4);
  result = ublas::prec_prod(*this, v);
  
  return Point(result[0]/result[3], result[1]/result[3], result[2]/result[3]);
}

PointF 
Matrix::operator*(const PointF& rhs) const
{
  ublas::vector<double> v(4);
  v[0] = rhs[0];
  v[1] = rhs[1];
  v[2] = rhs[2];
  v[3] = 1.0;

  ublas::vector<double> result(4);
  result = ublas::prod(*this, v);

  return PointF(static_cast<float>(result[0]/result[3]), 
                        static_cast<float>(result[1]/result[3]), 
                        static_cast<float>(result[2]/result[3]));
}

bool Invert(const Matrix& m, Matrix& inverse)
{
  Matrix A(m);
  ublas::permutation_matrix<std::size_t> pm(A.size1());
  inverse = Matrix::IDENTITY_C;
  
  Matrix::size_type singular = ublas::lu_factorize(A, pm);
  if (singular != 0)
  {
    return false;
  }
  
  // If the determinant of A is less than a small value, 
  // consider it singular.
  double det = A(0, 0) * A(1, 1) * A(2, 2) * A(3, 3);
  if (Abs(det) < Matrix::EPSILON_C)
  {
    return false;
  }
  
  ublas::lu_substitute(A, pm, inverse);
  
  return true;
}

} // End namespace Utils
