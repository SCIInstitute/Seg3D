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

#include <Core/Geometry/Vector.h>

using namespace Core;

TEST(VectorTests, EmptyVector)
{
  Vector *vector = new Vector;
  ASSERT_FALSE(vector == nullptr);

  ASSERT_EQ(vector->x(), 0);
  ASSERT_EQ(vector->y(), 0);
  ASSERT_EQ(vector->z(), 0);
}

TEST(VectorTests, EmptyVectorF)
{
  VectorF *vector = new VectorF;
  ASSERT_FALSE(vector == nullptr);

  ASSERT_EQ(vector->x(), 0);
  ASSERT_EQ(vector->y(), 0);
  ASSERT_EQ(vector->z(), 0);
}
