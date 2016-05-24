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

#include <Testing/Utils/DataBlockSource.h>
#include <Core/DataBlock/StdDataBlock.h>

namespace Testing {
  
namespace Utils {
    
template<class T>
std::tuple<Core::DataBlockHandle, Core::GridTransform>
generate3x3x3StdDataBlockInternal( Core::DataBlockHandle& dataBlock,
                                   const Core::DataType& type,
                                   const Core::Point& origin,
                                   const Core::Vector& spacing,
                                   bool nodeCentered )
{
  std::vector<T> vec = generate3x3x3Data<T>();
  T* data = new T[27];
  std::copy(vec.begin(), vec.end(), data);

  std::vector<size_t> dims;
  dims += 3, 3, 3;
  Core::Transform transform(origin,
                            Core::Vector( spacing.x(), 0.0 , 0.0 ),
                            Core::Vector( 0.0, spacing.y(), 0.0 ),
                            Core::Vector( 0.0, 0.0, spacing.z() ));
  Core::GridTransform gridTransform( dims[ 0 ], dims[ 1 ], dims[ 2 ], transform );
  gridTransform.set_originally_node_centered( nodeCentered );
  dataBlock = Core::StdDataBlock::New( gridTransform.get_nx(), 
                                       gridTransform.get_ny(),
                                       gridTransform.get_nz(),
                                       type );
  dataBlock->set_data(reinterpret_cast<void*>(data));
  return std::make_tuple(dataBlock, gridTransform);
}

std::tuple<Core::DataBlockHandle, Core::GridTransform>
generate3x3x3StdDataBlock( const Core::DataType& type,
                           const Core::Point& origin,
                           const Core::Vector& spacing,
                           bool nodeCentered )
{
  Core::DataBlockHandle dataBlock;
  switch(type)
  {
    case Core::DataType::CHAR_E:
      return generate3x3x3StdDataBlockInternal<signed char>(dataBlock,
                                                            type,
                                                            origin,
                                                            spacing,
                                                            nodeCentered);
    case Core::DataType::UCHAR_E:
      return generate3x3x3StdDataBlockInternal<unsigned char>(dataBlock,
                                                              type,
                                                              origin,
                                                              spacing,
                                                              nodeCentered);
    case Core::DataType::SHORT_E:
      return generate3x3x3StdDataBlockInternal<short>(dataBlock,
                                                      type,
                                                      origin,
                                                      spacing,
                                                      nodeCentered);
    case Core::DataType::USHORT_E:
      return generate3x3x3StdDataBlockInternal<unsigned short>(dataBlock,
                                                               type,
                                                               origin,
                                                               spacing,
                                                               nodeCentered);
    case Core::DataType::INT_E:
      return generate3x3x3StdDataBlockInternal<int>(dataBlock,
                                                    type,
                                                    origin,
                                                    spacing,
                                                    nodeCentered);
    case Core::DataType::UINT_E:
      return generate3x3x3StdDataBlockInternal<unsigned int>(dataBlock,
                                                             type,
                                                             origin,
                                                             spacing,
                                                             nodeCentered);
    case Core::DataType::LONGLONG_E:
      return generate3x3x3StdDataBlockInternal<long long>(dataBlock,
                                                          type,
                                                          origin,
                                                          spacing,
                                                          nodeCentered);
    case Core::DataType::ULONGLONG_E:
      return generate3x3x3StdDataBlockInternal<unsigned long long>(dataBlock,
                                                                   type,
                                                                   origin,
                                                                   spacing,
                                                                   nodeCentered);
    case Core::DataType::FLOAT_E:
      return generate3x3x3StdDataBlockInternal<float>(dataBlock,
                                                      type,
                                                      origin,
                                                      spacing,
                                                      nodeCentered);
    case Core::DataType::DOUBLE_E:
      return generate3x3x3StdDataBlockInternal<double>(dataBlock,
                                                       type,
                                                       origin,
                                                       spacing,
                                                       nodeCentered);
    default:
      Core::GridTransform gridTransform;
      return std::make_tuple(dataBlock, gridTransform);
  }
}

}}
