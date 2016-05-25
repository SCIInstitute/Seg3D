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

#include <Core/Utils/Log.h>

class msgTest {
public:
  std::vector<Core::LogMessageType> types;
  msgTest() {
    types.push_back( Core::LogMessageType::ERROR_E );
    types.push_back( Core::LogMessageType::WARNING_E );
    types.push_back( Core::LogMessageType::MESSAGE_E );
    types.push_back( Core::LogMessageType::DEBUG_E );
    types.push_back( Core::LogMessageType::SUCCESS_E );
    types.push_back( Core::LogMessageType::CRITICAL_ERROR_E );
  }
};

TEST(LogTests, UniqueBits)
{
  // LogMessageTypes should be a unique bit mask
  msgTest msg;
  for (size_t ii = 0; ii < msg.types.size(); ii++)
  {
    for (size_t jj = 0; jj < msg.types.size(); jj++)
    {
      if (ii != jj) {
        ASSERT_FALSE( msg.types[ii] & msg.types[jj] );
      }
    }
  }
}

TEST(LogTests, All)
{
  // ALL_E should contain all message types
  msgTest msg;
  unsigned int all = msg.types[0];
  for (size_t ii = 1; ii < msg.types.size(); ii++)
  {
    all = all | msg.types[ii];
  }
  ASSERT_TRUE( Core::LogMessageType::ALL_E == all );
}


TEST(LogTests, NoDebug)
{
  // NODEBUG_E group should not contain DEBUG_E
  ASSERT_FALSE( Core::LogMessageType::NODEBUG_E & Core::LogMessageType::DEBUG_E );
}
