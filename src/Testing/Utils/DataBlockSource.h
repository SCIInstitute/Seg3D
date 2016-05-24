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

#ifndef TESTING_UTILS_DATABLOCKSOURCE_H
#define TESTING_UTILS_DATABLOCKSOURCE_H

#include <boost/assign.hpp>

#include <tuple>
#include <vector>

#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/DataType.h>
#include <Core/Geometry/GridTransform.h>

namespace Testing {

namespace Utils {

using namespace boost::assign;

template<class T>
std::vector<T> generate3x3x3Data()
{
  std::vector<T> data;
  data += 0, 1, 2, 3, 4, 5, 6, 7, 8,
          9, 10, 11, 12, 13, 14, 15, 16, 17,
          18, 19, 20, 21, 22, 23, 24, 25, 26;

  return data;
}

std::tuple<Core::DataBlockHandle, Core::GridTransform>
generate3x3x3StdDataBlock( const Core::DataType& type,
                           const Core::Point& origin,
                           const Core::Vector& spacing,
                           bool nodeCentered );

}}

#endif
