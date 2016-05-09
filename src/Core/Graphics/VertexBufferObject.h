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

#ifndef CORE_GRAPHICS_VERTEXBUFFEROBJECT_H
#define CORE_GRAPHICS_VERTEXBUFFEROBJECT_H

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <GL/glew.h>

#include <Core/Utils/EnumClass.h>
#include <Core/Graphics/BufferObject.h>

namespace Core
{

CORE_ENUM_CLASS
(
  VertexAttribArrayType,
  VERTEX_E = 0,
  COLOR_E,
  TEXTURE_COORD_E,
  SECONDARY_COLOR_E,
  NORMAL_E,
  FOG_COORD_E,
  INDEX_E,
  EDGE_FLAG_E
)

class VertexBufferObject;
class VertexAttribArrayBuffer;
class ElementArrayBuffer;
typedef boost::shared_ptr< VertexBufferObject > VertexBufferObjectHandle;
typedef boost::shared_ptr< VertexAttribArrayBuffer > VertexAttribArrayBufferHandle;
typedef boost::shared_ptr< ElementArrayBuffer > ElementArrayBufferHandle;

class VertexBufferObject : public BufferObject
{
protected:
  VertexBufferObject();
  VertexBufferObject( const BufferObjectHandle& bo );
  virtual ~VertexBufferObject();
};

// CLASS VertexAttribArrayBuffer
// Buffer Target: GL_ARRAY_BUFFER
class VertexAttribArrayBuffer : public VertexBufferObject
{
private:

  class VertexAttribArrayInfo
  {
  public:
    VertexAttribArrayInfo() {}
    ~VertexAttribArrayInfo() {}

    GLenum array_type_;
    bool generic_;
    boost::function< void () > gl_array_pointer_func_;
  };

  typedef boost::shared_ptr< VertexAttribArrayInfo > VertexAttribArrayInfoHandle;

public:
  VertexAttribArrayBuffer();
  VertexAttribArrayBuffer( const BufferObjectHandle& bo );
  virtual ~VertexAttribArrayBuffer() {}

  // SET_ARRAY:
  // Set up a vertex attribute array in the vertex buffer
  void set_array( VertexAttribArrayType array_type, GLint vertex_size, GLenum data_type, 
    GLsizei stride, int offset );
  void set_array( VertexAttribArrayType array_type, GLenum data_type, GLsizei stride, int offset );
  void set_array( VertexAttribArrayType array_type, GLsizei stride, int offset );
  void set_generic_array( GLuint index, GLint attrib_size, GLenum data_type, 
    GLboolean normalized, GLsizei stride, int offset );

  void enable_arrays();
  void disable_arrays();

  void draw_arrays( GLenum mode, GLint first, GLsizei count );
  void multi_draw_arrays( GLenum mode, GLint* first, GLsizei* count, GLsizei primcount );

private:
  std::vector<VertexAttribArrayInfoHandle> vertex_arrays_;
  const static GLenum GL_ARRAY_TYPES_C[];

public:
  static void RestoreDefault();
};

// CLASS ElementArrayBuffer
// Buffer Target: GL_ELEMENT_ARRAY_BUFFER
class ElementArrayBuffer : public VertexBufferObject
{
public:
  ElementArrayBuffer();
  ElementArrayBuffer( const BufferObjectHandle& bo );
  virtual ~ElementArrayBuffer() {}

  void draw_elements( GLenum mode, GLsizei count, GLenum data_type, int offset = 0 );
  void draw_range_elements( GLenum mode, GLuint start, GLuint end, GLsizei count, 
    GLenum data_type, int offset = 0 );
  void multi_draw_elements( GLenum mode, GLsizei* count, GLenum data_type,
    const GLvoid** offsets, GLsizei primcount );

public:
  static void RestoreDefault();
};

} // end namespace Core

#endif
