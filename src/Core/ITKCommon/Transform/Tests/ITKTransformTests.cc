/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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


#include <Core/ITKCommon/itkCascadedTransform.h>
#include <Core/ITKCommon/itkRBFTransform.h>
#include <Core/ITKCommon/itkRadialDistortionTransform.h>

// TODO: get more test parameters from ITK unit tests
TEST(ITKCascadedTransform, ITKTransformTests)
{
  typedef itk::CascadedTransform<float, 3> CascadedTransform_float_3d;
  CascadedTransform_float_3d::Pointer t = CascadedTransform_float_3d::New();
  CascadedTransform_float_3d::InverseTransformBasePointer tInv = t->GetInverseTransform();
  std::string str = tInv->GetTransformTypeAsString();
//  std::cout << str << std::endl;

  EXPECT_FALSE(tInv.IsNull());
  EXPECT_EQ(str, "CascadedTransform_float_3_3");
}

TEST(ITKRBFTransform, ITKTransformTests)
{
  itk::RBFTransform::Pointer t = itk::RBFTransform::New();
  itk::RBFTransform::InverseTransformBasePointer tInv = t->GetInverseTransform();
  std::string str = tInv->GetTransformTypeAsString();
//  std::cout << str << std::endl;
  
  EXPECT_FALSE(tInv.IsNull());
  EXPECT_EQ(str, "RBFTransform_double_2_2");
}

// doesn't support float
TEST(ITKRadialDistortionTransform, ITKTransformTests)
{
  typedef itk::RadialDistortionTransform<double, 3> RadialDistortionTransform_double_3d;
  RadialDistortionTransform_double_3d::Pointer t = RadialDistortionTransform_double_3d::New();
  RadialDistortionTransform_double_3d::InverseTransformBasePointer tInv = t->GetInverseTransform();
  std::string str = tInv->GetTransformTypeAsString();
  std::cout << str << std::endl;

  EXPECT_FALSE(tInv.IsNull());
//  EXPECT_EQ(str, "RadialDistortionTransform_float_3_3");
}
