/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <Core/Application/Application.h>

// TODO: how to tell if parameter is invalid?
// --socket (correct) vs --port (incorrect) is common error

TEST(CommandLineTests, EmptyParams)
{
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter("") );
  // check some typical params
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter("help") );
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter("version") );
}

TEST(CommandLineTests, BasicParamsFailsOnSingleParam)
{
  // expects program name as argv[0]
  const int ARGC = 1;
  char** argv = new char* [ARGC];
  argv[0] = "--version";

  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter( "version") );
  
  delete [] argv;
}

TEST(CommandLineTests, ParamNotSet)
{
  const int ARGC = 1;
  char** argv = new char* [ARGC];
  argv[0] = "test";
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter( "notaflag") );
  
  delete [] argv;
}

TEST(CommandLineTests, StringParam)
{
  // expects program name as argv[0]
  const int ARGC = 2;
  char** argv = new char* [ARGC];
  argv[0] = "test";
  argv[1] = "--version";
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "version") );

  delete [] argv;
}

TEST(CommandLineTests, StringParamNotFlag)
{
  // expects program name as argv[0]
  const int ARGC = 2;
  char** argv = new char* [ARGC];
  argv[0] = "test";
  argv[1] = "version";
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "version") );
  
  delete [] argv;
}

TEST(CommandLineTests, HelpParam)
{
  const int ARGC = 1;
  char** argv = new char* [ARGC];
  argv[0] = "test";
  argv[1] = "--help";
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "help") );
  
  delete [] argv;
}
