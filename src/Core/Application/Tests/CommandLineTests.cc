
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

#include <Core/Application/Application.h>

#include <Core/Geometry/Vector.h>

// TODO: how to tell if parameter is invalid?
// --socket (correct) vs --port (incorrect) is common error

TEST(CommandLineTests, EmptyArgs)
{
  std::string nullArg0 = Core::Application::Instance()->get_argument(0);
  ASSERT_TRUE( nullArg0.empty() );
  
  std::string nullArg1 = Core::Application::Instance()->get_argument(1);
  ASSERT_TRUE( nullArg1.empty() );
}

TEST(CommandLineTests, EmptyParams)
{
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter("") );
  // check some typical params
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter("help") );
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter("version") );
}

TEST(CommandLineTests, BasicParamsFailsOnSingleParam)
{
  const int NUM_ARGS = 0; // also argument default
  // expects program name as argv[0]
  const int ARGC = 1;
  char argv0[] = { '-', '-', 'v', 'e', 'r', 's', 'i', 'o', 'n', '\0' };
  char* argv[] = { argv0 };

  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv, NUM_ARGS );
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter( "version") );
}

TEST(CommandLineTests, ParamNotSet)
{
  const int ARGC = 1;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char* argv[] = { argv0 };
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_FALSE( Core::Application::Instance()->is_command_line_parameter( "notaflag") );
}

TEST(CommandLineTests, StringParam)
{
  // expects program name as argv[0]
  const int ARGC = 2;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char argv1[] = { '-', '-', 'v', 'e', 'r', 's', 'i', 'o', 'n', '\0' };
  char* argv[] = { argv0, argv1 };
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "version") );
}

// accepts parameters without dashes
// TODO: is this a good thing?
TEST(CommandLineTests, StringParamNotFlag)
{
  // expects program name as argv[0]
  const int ARGC = 2;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char argv1[] = { 'v', 'e', 'r', 's', 'i', 'o', 'n', '\0' };
  char* argv[] = { argv0, argv1 };
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "version") );
}

TEST(CommandLineTests, SingleLetterFlag)
{
  const int ARGC = 2;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char argv1[] = { '-', 'h', '\0' };
  char* argv[] = { argv0, argv1 };

  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "h") );
}

TEST(CommandLineTests, VectorParam)
{
  const int ARGC = 2;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char argv1[] = { '-', '-', 'v', 'e', 'c', 't', 'o', 'r', '=', '[', '0', '.', '1', ',', '0', '.', '1', ',', '0', '.', '1', ']', '\0' };
  char* argv[] = { argv0, argv1 };
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "vector") );

  Core::Vector vector(1.0, 1.0, 1.0);
  std::string vector_string;
  ASSERT_TRUE( Core::Application::Instance()->check_command_line_parameter( "vector" , vector_string ) );
  ASSERT_TRUE( Core::ImportFromString( vector_string, vector ) );

  EXPECT_EQ( vector.x(), 0.1 );
  EXPECT_EQ( vector.y(), 0.1 );
  EXPECT_EQ( vector.z(), 0.1 );
}

TEST(CommandLineTests, PointParam)
{
  const int ARGC = 2;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char argv1[] = { '-', '-', 'p', 'o', 'i', 'n', 't', '=', '0', '.', '1', ',', '0', '.', '1', ',', '0', '.', '1', '\0' };
  char* argv[] = { argv0, argv1 };
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "point") );
  
  Core::Point point(0, 0, 0);
  std::string point_string;
  ASSERT_TRUE( Core::Application::Instance()->check_command_line_parameter( "point" , point_string ) );
  ASSERT_TRUE( Core::ImportFromString( point_string, point ) );
  
  EXPECT_EQ( point.x(), 0.1 );
  EXPECT_EQ( point.y(), 0.1 );
  EXPECT_EQ( point.z(), 0.1 );
}

TEST(CommandLineTests, VectorParamWithFileArgs)
{
  const int ARGC = 4;
  const int FILE_ARGS = 2;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char argv1[] = { 'f', 'i', 'l', 'e', '1', '\0' };
  char argv2[] = { 'f', 'i', 'l', 'e', '2', '\0' };
  char argv3[] = { '-', '-', 'v', 'e', 'c', 't', 'o', 'r', '=', '0', '.', '1', ',', '0', '.', '1', ',', '0', '.', '1', '\0' };
  char* argv[] = { argv0, argv1, argv2, argv3 };
  
  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv, FILE_ARGS );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "vector") );
  
  Core::Vector vector(1.0, 1.0, 1.0);
  std::string vector_string;
  ASSERT_TRUE( Core::Application::Instance()->check_command_line_parameter( "vector" , vector_string ) );
  ASSERT_TRUE( Core::ImportFromString( vector_string, vector ) );
  
  EXPECT_EQ( vector.x(), 0.1 );
  EXPECT_EQ( vector.y(), 0.1 );
  EXPECT_EQ( vector.z(), 0.1 );
}

TEST(CommandLineTests, NegativePointParamWithFileArgs)
{
  const int ARGC = 4;
  const int FILE_ARGS = 2;
  char argv0[] = { 't', 'e', 's', 't', '\0' };
  char argv1[] = { 'f', 'i', 'l', 'e', '1', '\0' };
  char argv2[] = { 'f', 'i', 'l', 'e', '2', '\0' };
  char argv3[] = { '-', '-', 'p', 'o', 'i', 'n', 't', '=', '-', '0', '.', '1', ',', '-', '0', '.', '1', ',', '-', '0', '.', '1', '\0' };
  char* argv[] = { argv0, argv1, argv2, argv3 };

  Core::Application::Instance()->parse_command_line_parameters( ARGC, argv, FILE_ARGS );
  ASSERT_TRUE( Core::Application::Instance()->is_command_line_parameter( "point") );

  Core::Vector point(1.0, 1.0, 1.0);
  std::string point_string;
  ASSERT_TRUE( Core::Application::Instance()->check_command_line_parameter( "point" , point_string ) );
  ASSERT_TRUE( Core::ImportFromString( point_string, point ) );

  EXPECT_EQ( point.x(), -0.1 );
  EXPECT_EQ( point.y(), -0.1 );
  EXPECT_EQ( point.z(), -0.1 );
}

