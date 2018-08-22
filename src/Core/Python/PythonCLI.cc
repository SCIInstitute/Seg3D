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

#ifdef _MSC_VER
#pragma warning( disable: 4244 )
#endif

//STL includes
#include <iostream>


// Core includes
#include <Core/Utils/Log.h>

#include <Core/Python/PythonCLI.h>

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( PythonCLI );

static void PythonCLIPrintOutput( std::string output )
{
  std::cout << output;
  std::cout.flush();
}

static void PythonCLIPrintError( std::string error )
{
  std::cerr << error;
  std::cerr.flush();
}

PythonCLI::PythonCLI()
{
  this->connection_handler.add_connection( Core::PythonInterpreter::Instance()->error_signal_.connect(
    boost::bind( &PythonCLIPrintError, _1 ) ) );
  this->connection_handler.add_connection( Core::PythonInterpreter::Instance()->output_signal_.connect(
    boost::bind( &PythonCLIPrintOutput, _1 ) ) );
}

PythonCLI::~PythonCLI()
{ }

void PythonCLI::execute_file( std::string& python_script )
{
  PythonInterpreter::Instance()->run_file(python_script);
}

}
