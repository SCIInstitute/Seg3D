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

#include <boost/filesystem.hpp>

#include <algorithm>
#include <tuple>
#include <vector>
#include <fstream>

#include <Core/DataBlock/NrrdData.h>
#include <Testing/Utils/DataBlockSource.h>
#include <Testing/Utils/FilesystemPaths.h>

using namespace Core;
using namespace Testing::Utils;

// Save detached nrrd for easy header contents testing.
// Data from dummy dataset.
TEST(NrrdDataTests, DetachedNrrdFromStdDataBlockNodeCentered)
{
  Core::Point origin(1, 1, 1);
  Core::Vector spacing(0.5, 0.5, 0.5);
  std::tuple<Core::DataBlockHandle, Core::GridTransform> tuple =
    generate3x3x3StdDataBlock(Core::DataType::INT_E, origin, spacing, true);

  // TODO: switch to nullptr
  Core::DataBlockHandle dataBlock = std::get<0>(tuple);
  Core::GridTransform gridTransform = std::get<1>(tuple);
  ASSERT_FALSE(dataBlock.get() == 0);

  Core::NrrdDataHandle nrrd =
    Core::NrrdDataHandle( new Core::NrrdData( dataBlock, gridTransform ) );

  boost::filesystem::path nrrdFile = testOutputDir() / "headerTest.nhdr";

  std::string error;
  // no compression
  EXPECT_TRUE(NrrdData::SaveNrrd(nrrdFile.string(), nrrd, error, false, 0));
  EXPECT_TRUE(error.empty());
//
//  // open and read header...
//  std::ifstream inputfile;
//  inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
  
}
