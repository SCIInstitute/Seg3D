/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_RENDERER_VERTEXBUFFEROBJECT_H
#define APPLICATION_RENDERER_VERTEXBUFFEROBJECT_H

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <GL/glew.h>

namespace Seg3D
{
class VertexBufferObject;
typedef boost::shared_ptr<VertexBufferObject> VertexBufferObjectHandle;

class VertexBufferObject : public boost::noncopyable
{
public:
  VertexBufferObject(GLenum target, GLenum array_type);
  ~VertexBufferObject(void);

  void set_buffer_data(GLenum data_type, GLint vertex_size, GLsizeiptr size, const GLvoid* data, GLenum usage);
  void set_buffer_sub_data(GLintptr offset, GLsizeiptr size, const GLvoid* data);

  void bind();
  void enable();
  void disable();

  void draw_arrays(GLenum mode, GLint first, GLsizei count);
  void multi_draw_arrays(GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);

  void draw_elements(GLenum mode, GLsizei count);
  void draw_range_elements(GLenum mode, GLuint start, GLuint end, GLsizei count);
  void multi_draw_elements(GLenum mode, GLsizei* count, GLsizei primcount);

private:
  boost::function<void (GLint, GLenum, GLsizei, GLvoid*)> gl_array_pointer_func4_;
  boost::function<void (GLenum, GLsizei, const GLvoid*)> gl_array_pointer_func3_;
  boost::function<void (GLsizei, const GLvoid*)> gl_array_pointer_func2_;

  boost::function<void ()> invoke_gl_array_pointer_func_;

  void safe_bind();
  void safe_unbind();

  void invoke_array_pointer_func4();
  void invoke_array_pointer_func3();
  void invoke_array_pointer_func2();

private:
  GLenum target_;
  GLuint id_;
  GLenum array_type_;
  GLenum query_target_;
  GLint vertex_size_;
  GLenum data_type_;
  GLint saved_id_;

};

} // end namespace Seg3D

#endif