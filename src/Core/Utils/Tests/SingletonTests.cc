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

#include <Core/Utils/Singleton.h>

class EmptyClass
{
public:
  EmptyClass() {}
};

TEST(SingletonTests, DefaultConstructor)
{
  Core::Singleton<EmptyClass> singleton;
}

TEST(SingletonTests, GetInstance)
{
  Core::Singleton<EmptyClass> singleton;
  EmptyClass* ptr = singleton.instance();
  ASSERT_TRUE(ptr != 0);
}


TEST(SingletonTests, GetTwoInstances)
{
  Core::Singleton<EmptyClass> singleton;
  EmptyClass* ptr0 = singleton.instance();
  ASSERT_TRUE(ptr0 != 0);

  EmptyClass* ptr1 = singleton.instance();
  ASSERT_TRUE(ptr1 != 0);

  ASSERT_TRUE(ptr0 == ptr1);
}
