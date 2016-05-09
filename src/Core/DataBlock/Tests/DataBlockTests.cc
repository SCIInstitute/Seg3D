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
#include <algorithm>

#include <Testing/Utils/DataBlockSource.h>
#include <Testing/Utils/DummyDataBlock.h>


using namespace Core;
using namespace Testing::Utils;

class DataBlockTest : public ::testing::Test {
protected:
  virtual void SetUp()
  {
    dataBlock_ = new DummyDataBlock;
  }
  
  virtual void TearDown()
  {
    delete dataBlock_;
  }
  
  DummyDataBlock *dataBlock_;
};

TEST_F(DataBlockTest, EmptyDataBlock)
{
  ASSERT_EQ(dataBlock_->get_nx(), 0);
  ASSERT_EQ(dataBlock_->get_ny(), 0);
  ASSERT_EQ(dataBlock_->get_nz(), 0);
  ASSERT_EQ(dataBlock_->get_data_type(), DataType::UNKNOWN_E);
  ASSERT_EQ(dataBlock_->get_generation(), -1);
  ASSERT_EQ(dataBlock_->get_elem_size(), 0);
  // no notification of error, but shouldn't crash if data type is UNKNOWN_E
  dataBlock_->set_data_at(1, 0);
  double data = dataBlock_->get_data_at(1);
  ASSERT_EQ(data, 0);
}

TEST_F(DataBlockTest, SetData)
{
  ASSERT_EQ(dataBlock_->get_data_type(), DataType::UNKNOWN_E);
  ASSERT_EQ(dataBlock_->get_nx(), 0);
  ASSERT_EQ(dataBlock_->get_ny(), 0);
  ASSERT_EQ(dataBlock_->get_nz(), 0);

  std::vector<int> vec = generate3x3x3Data<int>();
  int* data = new int[27];
  std::copy(vec.begin(), vec.end(), data);

  dataBlock_->set_data(reinterpret_cast<void*>(data));

  // get_data_at will return 0 if data type is UNKNOWN_E and size is 0
  EXPECT_EQ(dataBlock_->get_data_at( 0, 0, 0 ), 0);
  EXPECT_EQ(dataBlock_->get_data_at( 1, 1, 1 ), 0);
  EXPECT_EQ(dataBlock_->get_data_at( 25 ), 0);
  EXPECT_EQ(dataBlock_->get_data_type(), DataType::UNKNOWN_E);
  EXPECT_EQ(dataBlock_->get_size(), 0);
  
  EXPECT_EQ(dataBlock_->get_nx(), 0);
  EXPECT_EQ(dataBlock_->get_ny(), 0);
  EXPECT_EQ(dataBlock_->get_nz(), 0);

  // same code as set_type()
  dataBlock_->update_data_type(DataType::INT_E);
  dataBlock_->set_size(3, 3, 3);
 
  EXPECT_EQ(dataBlock_->get_data_at( 0, 0, 0 ), 0);
  EXPECT_EQ(dataBlock_->get_data_at( 1, 1, 1 ), 13.0);
  EXPECT_EQ(dataBlock_->get_data_at( 25 ), 25.0);
  EXPECT_EQ(dataBlock_->get_data_type(), DataType::INT_E);
  EXPECT_EQ(dataBlock_->get_size(), 3*3*3);

  EXPECT_EQ(dataBlock_->get_nx(), 3);
  EXPECT_EQ(dataBlock_->get_ny(), 3);
  EXPECT_EQ(dataBlock_->get_nz(), 3);
}

