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

#ifndef CORE_GRAPHICS_GLSLSHADER_H
#define CORE_GRAPHICS_GLSLSHADER_H

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <GL/glew.h>

namespace Core
{

class GLSLShader;
typedef boost::shared_ptr< GLSLShader > GLSLShaderHandle;

class GLSLShader : public boost::noncopyable
{
protected:
  GLSLShader( GLenum shader_type );
  virtual ~GLSLShader();

public:
  bool set_source_file( const std::string& file_name );
  void set_source( const std::string& source );

  // Compile the shader. Returns true if successful, otherwise false.
  // Additional information can be acquired by calling "get_info_log".
  bool compile();

  std::string get_info_log();

private:
  friend class GLSLProgram;
  GLuint shader_id_;
};

class GLSLVertexShader : public GLSLShader
{
public:
  GLSLVertexShader() : 
    GLSLShader( GL_VERTEX_SHADER )
  {
  }

  ~GLSLVertexShader() {}
};

class GLSLFragmentShader : public GLSLShader
{
public:
  GLSLFragmentShader() :
    GLSLShader( GL_FRAGMENT_SHADER )
  {
  }

  ~GLSLFragmentShader() {}
};

} // end namespace Core

#endif
