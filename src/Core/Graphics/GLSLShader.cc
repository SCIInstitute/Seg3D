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

#include <fstream>
#include <string>
#include <iostream>

#include <boost/shared_array.hpp>

#include <Core/Graphics/GLSLShader.h>
#include <Core/RenderResources/RenderResources.h>

#include <Core/Application/Application.h>

namespace Core
{

GLSLShader::GLSLShader( GLenum shader_type )
{
  this->shader_id_ = glCreateShader( shader_type );
}

GLSLShader::~GLSLShader()
{
  RenderResources::Instance()->delete_shader( this->shader_id_ );
}

bool GLSLShader::set_source_file( const std::string& file_name )
{
  std::ifstream source_file( file_name.c_str() );
  if ( !source_file )
  {
    return false;
  }

  std::string source_str;
  
  // NOTE: OSX 10.5 and less do not have proper support for clipping in the shader
  if ( Core::Application::Instance()->is_osx_10_5_or_less() )
  {
    source_str += std::string( "#define DISABLE_CLIPPING\n" );
  }
  
  while ( !source_file.eof() )
  {
    std::string line;
    std::getline( source_file, line );
    source_str += ( line + '\n' ); 
  }
  source_file.close();

  const char* str = source_str.c_str();
  glShaderSource( this->shader_id_, 1, &str, NULL );

  return true;
}

void GLSLShader::set_source( const std::string& source )
{
  std::string source_str;
  
  // NOTE: OSX 10.5 and less do not have proper support for clipping in the shader
  if ( Core::Application::Instance()->is_osx_10_5_or_less() )
  {
    source_str += std::string( "#define DISABLE_CLIPPING\n" );
}

  source_str += source;

  const char* shader_str = source_str.c_str();
  glShaderSource( this->shader_id_, 1, &shader_str, NULL );
}

bool GLSLShader::compile()
{
  glCompileShader( this->shader_id_ );
  int status;
  glGetShaderiv( this->shader_id_, GL_COMPILE_STATUS, &status );
  return status == GL_TRUE;
}

std::string GLSLShader::get_info_log()
{
  int log_length;
  glGetShaderiv( this->shader_id_, GL_INFO_LOG_LENGTH, &log_length );
  if ( log_length == 0 )
  {
    return std::string("");
  }

  boost::shared_array<char> log_str( new char[ log_length ] );
  int actual_len;
  glGetShaderInfoLog( this->shader_id_, log_length, &actual_len, log_str.get() );

  return std::string( log_str.get() );
}

} // end namespace Core
