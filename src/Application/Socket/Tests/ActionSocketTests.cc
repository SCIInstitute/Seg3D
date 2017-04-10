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
#include <iostream>
#include <clocale>
#include <cstdlib>

#include <boost/filesystem.hpp>
#include <boost/preprocessor.hpp>

#include <Python.h>
#include <Core/Python/PythonInterpreter.h>

#include <Application/Socket/ActionSocket.h>

using namespace Core;
using namespace Seg3D;
using namespace ::testing;
using namespace boost::filesystem;

class ActionSocketTests : public Test
{
protected:
  virtual void SetUp()
  {
    python_wstr = L"Seg3D2";
    python_import_module = "import seg3d2\n";
    python_import_from_module = "from seg3d2 import *\n";
  }
  
  const wchar_t* python_wstr;
  std::string python_import_module;
  std::string python_import_from_module;
};

TEST_F(ActionSocketTests, StartSocket)
{
  Core::PythonInterpreter::module_list_type python_modules;
  Core::PythonInterpreter::Instance()->initialize( python_wstr, python_modules );
  Core::PythonInterpreter::Instance()->run_string( python_import_module );
  Core::PythonInterpreter::Instance()->run_string( python_import_from_module );

  const int PORT = 9400;
  const std::string PORT_FILE("port");
  ActionSocket::Instance()->start( PORT );

  bool portFileExists = exists( PORT_FILE );
  ASSERT_TRUE(portFileExists);

  std::ifstream inputfile;
  inputfile.open( PORT_FILE.c_str() );
  int port = 0;
  inputfile >> port;
  ASSERT_EQ(port, PORT);
}
