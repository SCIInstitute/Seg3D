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

#include <fstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <Application/LayerIO/Actions/ActionExportPoints.h>
#include <Application/LayerIO/Actions/ActionExportMatrix.h>

#include <Core/Action/ActionContext.h>
#include <Core/Geometry/Point.h>

#include <Testing/Utils/FilesystemPaths.h>

using namespace Core;
using namespace Seg3D;
using namespace Testing::Utils;
using namespace ::testing;

class ActionExportPointsTests : public Test
{
protected:
  virtual void SetUp()
  {
    dummy_path = "/dummy/file/path/points.txt";
    boost::filesystem::path path = testOutputDir();
    path /= "points.txt";
    file_path = path.string();
    points.clear();
    context.reset( new ActionContext() );
    result.reset( new ActionResult() );
  }

  void SinglePointVector()
  {
    points.clear();
    points.push_back( Point(0, 0, 0) );
  }

  std::string dummy_path;
  std::string file_path;
  std::vector< Core::Point > points;
  ActionContextHandle context;
  ActionResultHandle result;
};


TEST_F(ActionExportPointsTests, EmptyAction)
{
  // factory instantiation
  ActionExportPoints action;
}

TEST_F(ActionExportPointsTests, ValidateEmptyParams)
{
  ActionExportPoints action;
  ASSERT_FALSE( action.validate(context) );
}

TEST_F(ActionExportPointsTests, ValidateEmptyFilePath)
{
  ActionExportPoints action;
  SinglePointVector();
  action.set_vector( points );
  ASSERT_FALSE( action.validate(context) );
}

TEST_F(ActionExportPointsTests, ValidateEmptyPoints)
{
  ActionExportPoints action;
  action.set_file_path( dummy_path );
  ASSERT_FALSE( action.validate(context) );
}

TEST_F(ActionExportPointsTests, RunInvalidPath)
{
  SinglePointVector();
  ActionExportPoints action( dummy_path, points );
  ASSERT_TRUE( action.validate(context) );
  ASSERT_FALSE( action.run( context, result ) );
}

TEST_F(ActionExportPointsTests, RunValidPath)
{
  SinglePointVector();
  ActionExportPoints action( file_path, points );
  ASSERT_TRUE( action.validate(context) );
  ASSERT_TRUE( action.run( context, result ) );

  std::ifstream inputfile;
  inputfile.open(file_path.c_str() );
  double x = -1.0, y = -1.0, z = -1.0;
  inputfile >> x >> y >> z;
  ASSERT_DOUBLE_EQ(x, 0);
  ASSERT_DOUBLE_EQ(y, 0);
  ASSERT_DOUBLE_EQ(z, 0);
}


class ActionExportMatrixTests : public Test
{
protected:
  virtual void SetUp()
  {
    dummy_path = "/dummy/file/path/matrix.txt";
    boost::filesystem::path path = testOutputDir();
    path /= "matrix.txt";
    file_path = path.string();
    matrix.clear();
    context.reset(new ActionContext());
    result.reset(new ActionResult());
  }

  void TwoByTwoIdentityMatrix()
  {
    matrix = { 1, 0, 0, 1 };
  }

  std::string dummy_path;
  std::string file_path;
  std::vector< double > matrix;
  ActionContextHandle context;
  ActionResultHandle result;
};


TEST_F(ActionExportMatrixTests, EmptyAction)
{
  // factory instantiation
  ActionExportMatrix action;
}

TEST_F(ActionExportMatrixTests, ValidateEmptyParams)
{
  ActionExportMatrix action;
  ASSERT_FALSE(action.validate(context));
}

TEST_F(ActionExportMatrixTests, ValidateEmptyFilePath)
{
  ActionExportMatrix action;
  TwoByTwoIdentityMatrix();
  action.set_vector(matrix);
  ASSERT_FALSE(action.validate(context));
}

TEST_F(ActionExportMatrixTests, ValidateEmptyPoints)
{
  ActionExportMatrix action;
  action.set_file_path(dummy_path);
  ASSERT_FALSE(action.validate(context));
}

TEST_F(ActionExportMatrixTests, RunInvalidPath)
{
  TwoByTwoIdentityMatrix();
  ActionExportMatrix action(dummy_path, matrix, 2, 2);
  ASSERT_TRUE(action.validate(context));
  ASSERT_FALSE(action.run(context, result));
}

TEST_F(ActionExportMatrixTests, RunValidPath)
{
  TwoByTwoIdentityMatrix();
  ActionExportMatrix action(file_path, matrix, 2, 2);
  ASSERT_TRUE(action.validate(context));
  ASSERT_TRUE(action.run(context, result));

  std::ifstream inputfile;
  inputfile.open(file_path.c_str());

  double w = -1, x = -1.0, y = -1.0, z = -1.0;
  inputfile >> w >> x >> y >> z;
  ASSERT_DOUBLE_EQ(w, 1);
  ASSERT_DOUBLE_EQ(x, 0);
  ASSERT_DOUBLE_EQ(y, 0);
  ASSERT_DOUBLE_EQ(z, 1);
}
