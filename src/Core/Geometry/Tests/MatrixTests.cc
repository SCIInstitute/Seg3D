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

#include <gtest/gtest.h>

#include <Core/Geometry/Matrix.h>

#define EPSILON 10e-9
#define EPSILONF 10e-9f

using namespace Core;

TEST(MatrixTests, EmptyMatrix)
{
  Matrix *matrix = new Matrix;
  ASSERT_FALSE(matrix == nullptr);

  double *data = matrix->data();
  for (size_t i =  0; i < 16; ++i)
  {
    ASSERT_EQ(data[i], 0);
  }
}

TEST(MatrixTests, EmptyMatrixF)
{
  MatrixF *matrix = new MatrixF;
  ASSERT_FALSE(matrix == nullptr);

  float *data = matrix->data();
  for (size_t i =  0; i < 16; ++i)
  {
    ASSERT_EQ(data[i], 0);
  }
}

TEST(MatrixTests, ZeroMatrix)
{
  Matrix zero = Matrix::Zero();
  double *data = zero.data();
  for (size_t i =  0; i < 16; ++i)
  {
    ASSERT_EQ(data[i], 0);
  }
}

TEST(MatrixTests, IdentityZeroF)
{
  MatrixF zero = MatrixF::Zero();
  float *data = zero.data();
  for (size_t i =  0; i < 16; ++i)
  {
    ASSERT_EQ(data[i], 0);
  }
}

TEST(MatrixTests, IdentityMatrix)
{
  Matrix identity = Matrix::Identity();
  double *data = identity.data();
  for (size_t i =  0; i < 16; ++i)
  {
    if (i == 0 || i == 5 || i == 10 || i == 15)
    {
      ASSERT_NEAR(data[i], 1, EPSILON);
    }
    else
    {
      ASSERT_NEAR(data[i], 0, EPSILON);
    }
  }
}

TEST(MatrixTests, IdentityMatrixF)
{
  MatrixF identity = MatrixF::Identity();
  float *data = identity.data();
  for (size_t i =  0; i < 16; ++i)
  {
    if (i == 0 || i == 5 || i == 10 || i == 15)
    {
      ASSERT_NEAR(data[i], 1, EPSILONF);
    }
    else
    {
      ASSERT_NEAR(data[i], 0, EPSILONF);
    }
  }
}
