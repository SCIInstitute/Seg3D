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

#include <boost/shared_array.hpp>
#include <Core/Graphics/GLSLProgram.h>
#include <Core/RenderResources/RenderResources.h>

namespace Core
{

GLSLProgram::GLSLProgram()
{
  this->program_id_ = glCreateProgram();
}

GLSLProgram::~GLSLProgram()
{
  RenderResources::Instance()->delete_program( this->program_id_ );
}

void GLSLProgram::attach_shader( GLSLShaderHandle shader )
{
  glAttachShader( this->program_id_, shader->shader_id_ );
}

void GLSLProgram::detach_shader( GLSLShaderHandle shader )
{
  glDetachShader( this->program_id_, shader->shader_id_ );
}

int GLSLProgram::get_uniform_location( const char* name )
{
  return glGetUniformLocation( this->program_id_, name );
}

bool GLSLProgram::link()
{
  glLinkProgram( this->program_id_ );
  int status;
  glGetProgramiv( this->program_id_, GL_LINK_STATUS, &status );
  return status == GL_TRUE;
}

bool GLSLProgram::validate()
{
  glValidateProgram( this->program_id_ );
  int status;
  glGetProgramiv( this->program_id_, GL_VALIDATE_STATUS, &status );
  return status == GL_TRUE;
}

std::string GLSLProgram::get_info_log()
{
  int log_length;
  glGetProgramiv( this->program_id_, GL_INFO_LOG_LENGTH, &log_length );
  if ( log_length == 0 )
  {
    return std::string("");
  }

  boost::shared_array<char> log_str( new char[ log_length ] );
  int actual_len;
  glGetProgramInfoLog( this->program_id_, log_length, &actual_len, log_str.get() );

  return std::string( log_str.get() );
}

void GLSLProgram::enable()
{
  glUseProgram( this->program_id_ );
}

void GLSLProgram::disable()
{
  glUseProgram( 0 );
}

void GLSLProgram::bind_attrib_location( unsigned int index, const char* name )
{
  glBindAttribLocation( this->program_id_, index, name );
}

} // end namespace Core
