#include <gtest/gtest.h>

#include <Core/ITKCommon/itkIdentityTransformAdaptor.h>
#include <Core/ITKCommon/itkTransformAdaptor.h>

#include <itkTransform.h>

TEST(ITKTransformAdapterTest, ITKTransformAdapterTests)
{
  itk::TransformAdapter<double, 2, 2>::Pointer t1 = itk::TransformAdapter<double, 2, 2>::New();
  itk::TransformAdapter<double, 2, 2>::InverseTransformType::Pointer t2 = t1->GetInverse();
//  std::string str1 = t1->GetTransformTypeAsString();
//  std::cout << "str1=[" << str1 << "]" << std::endl;
  EXPECT_TRUE(t2.IsNull());

//  itk::Transform<double, 2, 2>::Pointer t3 = itk::Transform<double, 2, 2>::New();
//  itk::Transform<double, 2, 2>::InputPointType pnt;
//  t3->TransformPoint(pnt);
//  itk::Transform<double, 2, 2>::ParametersType parameters(6);
//  t3->SetParameters(parameters);
//  std::string str2 = t2->GetTransformTypeAsString();
//  std::cout << "str2=[" << str2 << "]" << std::endl;
}

TEST(ITKIdentityTransformAdapterTest, ITKTransformAdapterTests)
{
  itk::IdentityTransformAdapter<double, 2>::Pointer t1 = itk::IdentityTransformAdapter<double, 2>::New();
  itk::IdentityTransformAdapter<double, 2>::InverseTransformType::Pointer t2 = t1->GetInverse();
//  std::string str2 = t3->GetTransformTypeAsString();

  EXPECT_FALSE(t2.IsNull());
//  itk::IdentityTransformAdapter<double, 2>::InverseTransformType::Pointer t3 =
//    static_cast<itk::IdentityTransformAdapter<double, 2>::InverseTransformType::Pointer>(t1);
//  EXPECT_EQ(*(t2.GetPointer()), *(t3.GetPointer()));
}